#ifndef MYSERVER_HPP
#define MYSERVER_HPP



#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include <deque>
#include <set>

#include "transactions.h"


using boost::asio::ip::tcp;
using namespace std;

extern unsigned long time_of_start;



#define max_lengths (32*1024*1024) // max length of receiving buffer



class tcp_connection :  public boost::enable_shared_from_this<tcp_connection>
{
public:

  typedef boost::shared_ptr<tcp_connection> pointer;
  static pointer create(boost::asio::io_service& io_service);
  tcp::socket& socket();
  void start();

  tcp_connection(boost::asio::io_service& io_service);

  uint32_t id;

private:

  tcp::socket socket_;
  std::string message_;
  char data_[max_lengths];
  boost::array<char, max_lengths> data_buffer;
  string full_buffer;
};

typedef struct Peers{
    string ip;
    uint32_t port;
    bool connected;
    boost::shared_ptr<tcp_connection> session;
    deque <string> _m;
    boost::asio::io_service::strand *_strand;
}Peer;


typedef tcp::socket* bpv;


class tcp_server
{
public:

    tcp_server(boost::asio::io_service& io_service, string ip, uint32_t port);
    
    void add_peer( Peer p, bool is_connected);
    void add_indirect_peer_if_doesnt_exist( string p );
    void add_peers_ip( string ip);
    int no_peers();
    int no_connected_peers();
    void print_peers();
    void close_peer_connection ( uint32_t no);

    void send_block_to_peers(network_block *nb );
    void send_block_to_one_peer(string sender_ip, uint32_t sender_port, uint32_t chain_id, BlockHash parent, BlockHash hash, block *b );
    void write_to_all_peers(string message );
    void write_to_one_peer(string peer_ip, uint32_t peer_port, string message );

    void run_network();

    string get_server_folder();
    void additional_verified_transaction( uint32_t add_new);
    void add_bytes_received( uint32_t br, uint32_t mbr );

    bool add_ping( string tt, int dnext, bool overwrite );



private:
  void start_accept();
  void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error);
  void handle_write( boost::system::error_code ec, size_t length, int peer_index);

  void write( int peer_index );
  void strand_write( string message, int peer_index );
  void strand_proceed( int peer_index );



  string my_ip;
  uint32_t my_port;
  tcp::acceptor acceptor_;
  boost::asio::io_service my_io_service;

  // Peers
  std::vector<Peers> peers;
  std::map<string,int> speers;
  std::set<string> peer_ips;

  // Pings
  //std::map<string,pair<int, unsigned long>> pings;
  std::map<string,int> pings;
  unsigned long next_ping;
  unsigned int no_pings;
  

  std::unique_ptr<boost::asio::deadline_timer> t;
  unsigned long last_ask_for_incomplete;
  unsigned long last_ask_for_full;
  unsigned long last_print_blockchain;
  unsigned long last_peer_connect;
  unsigned long last_update_commited;
  unsigned long no_verified_transactions = 0;
  unsigned long bytes_received;
  unsigned long bytes_txs_received;
  string folder_blockchain;
  long last_stored_hash_depth = -1;
};



#endif