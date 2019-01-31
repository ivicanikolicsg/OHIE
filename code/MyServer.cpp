/*
Copyright (c) 2018, Ivica Nikolic <cube444@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include "Blockchain.hpp"
#include "MyServer.hpp"
#include "miner.h"
#include "verify.h"
#include "requests.h"
#include "process_buffer.h"



using boost::asio::ip::tcp;
using namespace std;




extern tcp_server *ser;
extern Blockchain *bc;
extern mt19937 rng;

mutex mtx; 

string folder_sessions = string(FOLDER_SESSIONS);






/*
 * tcp_connection
 */


typedef boost::shared_ptr<tcp_connection> pointer;


pointer tcp_connection::create(boost::asio::io_service& io_service)
{
    return pointer(new tcp_connection(io_service));
}

tcp::socket& tcp_connection::socket()
{
    return socket_;
}




void tcp_connection::start()
{
  

  try{
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_buffer),
        [this, self ](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {

            std::unique_lock<std::mutex> l(bc->lock);
            bc->can_write.wait( l, [](){return !bc->locker_write;});
            bc->locker_write = true;


            if ( WRITE_SESSIONS_TO_HDD ){
              string filename =  folder_sessions+"/"+to_string(id);
              ofstream file;
              file.open(filename, std::ios_base::app); 
              for(int k=0; k<length;k++)
                file << data_buffer[k];
              file << endl;
              file.close();
            }


            for(int z=0; z<length; z++)
                full_buffer.push_back( data_buffer[z]);

            // Increase amount of received bytes
          	ser->add_bytes_received( length, 0);

			// Process buffer            
            process_buffer( full_buffer, ser, bc );

            bc->locker_write = false;
            l.unlock();
            bc->can_write.notify_one();


          	start();



          }
          else{

            //printf("ERROR OCCURED !!!\n");
            //fflush(stdout);
          }

          //start();

        });

    }
    catch(...){
      cout <<"async_read_some failed"<<endl;
      fflush(stdout);
      exit(3);
    }



}


tcp_connection::tcp_connection(boost::asio::io_service& io_service)
: socket_(io_service),full_buffer(""), id(0)
{
}





/*
 *
 * tcp_server
 *
 */


tcp_server::tcp_server(boost::asio::io_service& io_service, string ip, uint32_t port)
: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), my_ip(ip), my_port(port), t(new boost::asio::deadline_timer(io_service)), 
  last_ask_for_incomplete(0), last_print_blockchain(0), last_update_commited(0), bytes_received(0), bytes_txs_received(0), folder_blockchain(string(FOLDER_BLOCKCHAIN))
{
    start_accept();

    if ( PING_REPEAT > 0){
      no_pings = 0;
      unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
      next_ping = time_of_now + PING_MIN_WAIT +  (rng() % (PING_MAX_WAIT-PING_MIN_WAIT) );
    }


}

int tcp_server::no_peers()
{
  return peers.size();
}

int tcp_server::no_connected_peers()
{
  int count= 0;
  for( int i=0; i<peers.size(); i++)
    count +=  peers[i].connected ;  // peers[i].session != NULL;
  return count;

}



void tcp_server::add_peer( Peer p, bool is_connected)
{
    if (!(p.ip == my_ip && p.port == my_port )){

        string key = p.ip+":"+to_string(p.port);
        if ( speers.find(key) != speers.end() ) return;

        if( !is_connected ){
          p.connected = false;
          p.session = NULL;
          p._strand = NULL;

        }
        peers.push_back(p);

        // Make the hash table of peers (used later to avoid connecting such peers as blind peers)
        speers.insert( make_pair( key , 1 ) );
    }
}

void tcp_server::add_indirect_peer_if_doesnt_exist( string p)
{

  if( speers.find(p) == speers.end() ){
    int pos = p.find(":");
        if( pos > 0 ){
            Peer pr;
            pr.ip = p.substr(0,pos);
            pr.port= atoi(p.substr(pos+1,  p.length()).c_str());
            add_peer( pr, false );
        }
  }
}

void tcp_server::add_peers_ip( string ip)
{
    peer_ips.insert( ip );
}


void tcp_server::close_peer_connection ( uint32_t no)
{

    if( no >= peers.size() ) return;

    peers[no].session = NULL;
    peers[no].connected = false;
    peers[no]._strand = NULL;
 //   peers[no]._m.clear();

    if ( PRINT_PEER_CONNECTION_MESSAGES ){
      printf("\033[31;1mClosing connection to peer %s:%d\033[0m",(peers[no].ip).c_str(),peers[no].port);
      printf(" ::: #Connected peers : %d", no_connected_peers() );
      printf("\n");
      fflush(stdout);
  }
}

void tcp_server::print_peers()
{
    cout << "Peers:" << endl;
    for(int i=0; i<peers.size(); i++){
        cout << '\t'<< (peers[i]).ip<<" : " << peers[i].port << "   connected: " << peers[i].connected<< endl;
    }
}


void tcp_server::handle_write( boost::system::error_code ec, size_t length, int peer_index)
{

  if( peer_index >= peers.size() || peers[peer_index]._m.size() <= 0 ) return;

  string mm =  peers[peer_index]._m[0];
  if( mm.find("#full_block") == 0 and mm.length() > 10 ){
      string mz = mm.substr( mm.length() - 14, 13  );
      bool pr=true;
      unsigned long sol = safe_stoull( mz, pr);
      unsigned long nol = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
      if (pr){
//        cout <<"GOTIT: "<<(nol-sol)<<endl;
      }
  }


  peers[peer_index]._m.pop_front();
  if ( ec ) {
    close_peer_connection(peer_index);

    return;
  }

  if ( !peers[peer_index]._m.empty() ){
    write( peer_index );
  }
}


void tcp_server::write( int peer_index )
{
    if( peer_index >= peers.size() ) return;

    string mm =  peers[peer_index]._m[0];
    if( mm.find("#full_block") == 0 and mm.length() > 10 ){
        string mz = mm.substr( mm.length() - 14, 13  );
        bool pr=true;
        unsigned long sol = safe_stoull( mz, pr);
        unsigned long nol = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
        if (pr){
//          cout <<"TOTIT: "<<(nol-sol)<<endl;
        }
    }



    if( peer_index < peers.size() &&  peers[peer_index].session != NULL && peers[peer_index].connected && peers[peer_index]._strand != NULL ) {
      boost::asio::async_write( peers[peer_index].session->socket(), boost::asio::buffer(peers[peer_index]._m[0]),
                  peers[peer_index]._strand->wrap(
                                 boost::bind( &tcp_server::handle_write, this, boost::asio::placeholders::error, 
                                  boost::asio::placeholders::bytes_transferred, peer_index )
                                 )
                  );
    }
    else{
      boost::system::error_code ec;
      handle_write( ec, 0, peer_index );
    }
}


void tcp_server::strand_write( string message, int peer_index )
{

    if( peer_index >= peers.size() ) return;

    peers[peer_index]._m.push_back( message + "!" );
    if( peers[peer_index]._m.size() > 1)
      return;

    write(peer_index);

}


void tcp_server::strand_proceed( int peer_index )
{

    if( peer_index >= peers.size() ) return;

    if( peers[peer_index]._m.size() > 0)
      write(peer_index);

}


void tcp_server::write_to_all_peers(string message )
{

  // Write to all  peers
  for( int i=0; i<peers.size(); i++){

      if( peers[i]._strand != NULL )
          peers[i]._strand->post( boost::bind(&tcp_server::strand_write, this, message, i ) );
  }

}



void tcp_server::write_to_one_peer(string peer_ip, uint32_t peer_port, string message )
{


    string mm =  message;
    if( mm.find("#full_block") == 0 and mm.length() > 10 ){
        string mz = mm.substr( mm.length() - 13, 13  );
        bool pr=true;
        unsigned long sol = safe_stoull( mz, pr);
        unsigned long nol = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
        if (pr){
//          cout <<"MOTIT: "<<(nol-sol)<<endl;
        }
    }



    for( int i=0; i<peers.size(); i++)
      if(peers[i].ip == peer_ip && peers[i].port == peer_port ){

          if( peers[i]._strand != NULL )
              peers[i]._strand->post( boost::bind(&tcp_server::strand_write, this, message, i ) );

          break;
        }

}


void tcp_server::run_network()
{


  std::unique_lock<std::mutex> l(bc->lock);
  bc->can_write.wait( l, [](){return !bc->locker_write;});
  bc->locker_write = true;


  unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);


  // connecting to peers
  if ( time_of_now - last_peer_connect > CONNECT_TO_PEERS_MILLISECONDS){

    last_peer_connect = time_of_now;

    for( int i=0; i<peers.size(); i++){
        if( !peers[i].connected /* || peers[i].session == NULL */){

  //          peers[i].session = make_shared<tcp_connection>(acceptor_.get_io_service());
          try{
            peers[i].session = tcp_connection::create(acceptor_.get_io_service());
          }
          catch(...){
            cout << "Creating session threw... nothing major..."<<endl;
            continue;
          }
          peers[i].session->id = rng();


          if ( peers[i]._strand == NULL ){
            try{
              peers[i]._strand = new boost::asio::io_service::strand( acceptor_.get_io_service() );
            }
            catch(...){
              cout <<"Creating strand threw... nothing major..."<<endl;
              continue;
            }
          }


          tcp::endpoint *ep;
          try {
            ep = new tcp::endpoint( boost::asio::ip::address_v4::from_string(peers[i].ip), peers[i].port );
          }
          catch(...){
            cout <<"Creating endpoing to "<<peers[i].ip<<":"<<peers[i].port<<" threw... nothing major..."<<endl;
            continue;
          }

          peers[i].session->socket().async_connect(*ep, [this,i](boost::system::error_code const& ec) 
          {
            if (!ec)
            {
              if ( i < peers.size() ){
                peers[i].connected = true;
                if ( PRINT_PEER_CONNECTION_MESSAGES ){
                  printf("\033[32;1m[+] Connected to peer %s:%d\033[0m",(peers[i].ip).c_str(),peers[i].port);
                  printf("\n");
                  fflush(stdout);
                }
                peers[i]._strand->post( boost::bind(&tcp_server::strand_proceed, this,  i ) );
              }
            }
            else
            {
              close_peer_connection( i );
            }
          });   

        }      
    }
  }


  // Get incomplete chains
  if ( time_of_now - last_ask_for_incomplete > ASK_FOR_INCOMPLETE_EACH_MILLISECONDS){
    
      for( int i=0; i<CHAINS; i++){
        vector<BlockHash> hashes = bc->get_incomplete_chain_hashes( i , time_of_now );
        for ( int j=0; j<hashes.size(); j++){

              if( PRINT_SENDING_MESSAGES ){
                printf ("\033[34;1mAsking %d: %lx from all peers\033[0m\n", i, hashes[j] );
                fflush(stdout);
              }

              string s = create__ask_block( i, hashes[j], 0, MAX_ASK_BLOCKS );
              write_to_all_peers( s );
        }     
      }
      last_ask_for_incomplete = time_of_now;
  }


    // Get full block s
  if ( time_of_now - last_ask_for_full > ASK_FOR_FULL_BLOCKS_EACH_MILLISECONDS){
    
      vector< pair <BlockHash, uint32_t> > blocks = bc->get_non_full_blocks( time_of_now );

      for( auto it=blocks.begin(); it != blocks.end(); it++){

        if( PRINT_SENDING_MESSAGES ){
          printf ("\033[34;1mGotFullBlock %d: %lx from all peers\033[0m\n", it->second, it->first );
          fflush(stdout);
        }


        string s = create__got_full_block(it->second, it->first);
        write_to_all_peers( s );
      }

      //remove waiting blocks if too much wait time has passed
      bc->remove_waiting_blocks( time_of_now );

      last_ask_for_full = time_of_now;
  }


  // Update commited 
  if ( time_of_now - last_update_commited > UPDATE_COMMITED_TIME_EACH_MILLISECONDS){
    
      bc->update_blocks_commited_time();
      last_update_commited = time_of_now;
  }



  // Pings
  if ( no_pings < PING_REPEAT && next_ping < time_of_now ){

      no_pings ++;

      string tt = my_ip + ":"+ to_string(my_port)+":"+to_string(no_pings);
      int mode = no_pings % 2;
      add_ping( tt, 0, mode ); 

      string s = create__ping( tt , 0, time_of_now, mode );
      ser->write_to_all_peers( s );


      next_ping = time_of_now + 1000*(PING_MIN_WAIT +  (rng() % (PING_MAX_WAIT-PING_MIN_WAIT) ) );

  }



  // Print blockchain
  if ( time_of_now - last_print_blockchain > PRINT_BLOCKCHAIN_EACH_MILLISECONDS){

      unsigned long time_of_finish = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
      float secs = (time_of_finish - time_of_start)/ 1000.0;
    
      printf("-------------------------------------Server: %s:%d-------------------------------------------\n", my_ip.c_str(), my_port);
      printf("\n=============== [MAIN:   ]    #Chains:  %4d   Mine time: %7.0f msecs   Block size : %.0f KB  Localblocks: ", CHAINS, (float)EXPECTED_MINE_TIME_IN_MILLISECONDS, (float)BLOCK_SIZE_IN_BYTES/1024.0);
      for( int j=0; j<NO_T_DISCARDS;j ++)
        printf("%d ", T_DISCARD[j]);
      printf("\n");
      
      printf("\n=============== [NETWORK:]    #Peers    : %3d :  ", no_connected_peers() );
      for( int j=0; j<peers.size(); j++) {
        if( !peers[j].connected ) printf ("\033[37m");
        else printf ("\033[1m");
        printf("%s:%d ", peers[j].ip.c_str(), peers[j].port);
        printf ("\033[0m" );
      }
      printf("\n");

      printf("\n=============== [THROUGHPUT:] MB: %.1f   MB/s: %.2f    GB/h:  %.1f  ::::  txs MB/s:  %.2f  txs GB/h:  %.1f \n", 
        bytes_received/(1024.0*1024), bytes_received/(1024.0*1024)/secs, bytes_received/(1024.0*1024)/secs * 3600/1000, 
        bytes_txs_received/(1024.0*1024)/secs, bytes_txs_received/(1024.0*1024)/secs * 3600/1000);
      
      printf("\n=============== [TXS       :] Verified :  %8ld     Rate: %.0f txs/s  \n", no_verified_transactions, no_verified_transactions/ secs);

      bc->specific_print_blockchain();
      last_print_blockchain = time_of_now;

      printf("--------------------------------------------------------------------------------------------------------\n");



      // Store blockchain hash 
      // -6 
      if ( WRITE_HASH_TO_HDD ){

        block *b = bc->get_deepest_child_by_chain_id(0) ;
        int dp = 0;
        while( (dp++) < STORE_HASH_MINUS && NULL != b && NULL != b->parent)
            b = b->parent;

        if ( NULL != b && NULL != b->nb ){

          int maxd= b->nb->depth;
          vector <string> vals;
          while ( NULL !=b && b->nb->depth > last_stored_hash_depth  ){
          
            if ( ( b->nb->depth % STORE_HASH_FREQ) == 0 ){
              stringstream stream;
              stream << b->nb->depth << ":" << hex <<  b->hash << dec;
              vals.push_back( stream.str() );
            }

            b = b -> parent;
          }

          if ( maxd >= 0){
            last_stored_hash_depth = maxd;
            string filename =  string(FOLDER_HASHES)+"/"+my_ip+to_string(my_port);
            ofstream file;
            file.open(filename, std::ios_base::app); 
            for( int j=vals.size()-1; j>=0; j--)
              file << vals[j] << endl;
            file.close();
          }
        }
      }

  }


  bc->locker_write = false;
  l.unlock();
  bc->can_write.notify_one();




  // Repeat
  auto runcb = [this](boost::system::error_code const& error) { run_network(); };
  t->expires_from_now(boost::posix_time::milliseconds(RUN_NETWORK_EACH_MILLISECONDS));
  t->async_wait(runcb);



}



void tcp_server::start_accept()
{
    tcp_connection::pointer new_connection = tcp_connection::create(acceptor_.get_io_service());
    new_connection->id = rng();


    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
}

void tcp_server::handle_accept(tcp_connection::pointer new_connection,const boost::system::error_code& error)
{
    if (!error)
    {
      string connecting_ip = new_connection->socket().remote_endpoint().address().to_string();

      if( REJECT_CONNECTIONS_FROM_UNKNOWNS && peer_ips.find(connecting_ip) == peer_ips.end() ){
        printf("\033[31;1m[-] IP (%s) of peer not in the list of allowed\n\033[0m", connecting_ip.c_str() );
      }
      else{

        new_connection->start();
        printf("\033[32;1m[+] Connection established from %s:%d\n\033[0m", 
            new_connection->socket().remote_endpoint().address().to_string().c_str(), new_connection->socket().remote_endpoint().port());
      }
    }

    start_accept();
}




void tcp_server::send_block_to_peers(network_block *nb ) 
{

  if( PRINT_SENDING_MESSAGES ){
    printf ("\033[34;1mSending %d: (%lx %lx) to all peers\033[0m\n", nb->chain_id, nb->parent, nb->hash );
    fflush(stdout);
  }

  string s = create__process_block( nb );
  write_to_all_peers( s );

}


void tcp_server::send_block_to_one_peer(string sender_ip, uint32_t sender_port, uint32_t chain_id, BlockHash parent, BlockHash hash, block *b )
{

  if( PRINT_SENDING_MESSAGES ){
    printf ("\033[34;1mSending %d: (%lx %lx) to peer %s:%d\033[0m\n", chain_id, parent,hash, sender_ip.c_str(), sender_port );
    fflush(stdout);
  }

  string s = create__process_block( b->nb );
  write_to_one_peer(sender_ip, sender_port, s );
}




string tcp_server::get_server_folder()
{
  return folder_blockchain+"/_"+my_ip+"_"+to_string(my_port);
}


void tcp_server::additional_verified_transaction( uint32_t add_new)
{
  no_verified_transactions += add_new;
}

void tcp_server::add_bytes_received( uint32_t br, uint32_t mbr )
{
  bytes_received += br;
  bytes_txs_received += mbr;
}


bool tcp_server::add_ping( string tt, int dnext, bool overwrite )
{

  if (overwrite ){
    auto it = pings.find( tt );

    if( it == pings.end() || it->second > dnext ){
      pings[tt] = dnext;
      return true;
    }
    return false;
  }


  if ( pings.find( tt ) != pings.end() ) return false;

  pings.insert( make_pair(tt,dnext) );
  return true;
}
