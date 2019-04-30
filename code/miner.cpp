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

#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <openssl/sha.h>
#include "Blockchain.hpp"
#include "MyServer.hpp"

#include "verify.h"
#include "transactions.h"
#include "crypto_stuff.h"

using namespace std;

extern mt19937 rng;
extern tcp_server *ser;
extern boost::thread *mythread;


uint32_t total_mined = 0;




uint32_t mine_new_block( Blockchain *bc)
{


  	std::unique_lock<std::mutex> l(bc->lock);
	bc->can_write.wait( l, [bc](){return !bc->locker_write;});
	bc->locker_write = true;


	// Concatenate the candidates of all chains 
	vector<string> leaves;		// used in Merkle tree hash computation

	// Last block of the trailing chain 
	block *trailing_block = bc->get_deepest_child_by_chain_id( 0 );
	int trailing_id = 0;
	for( int i=0; i<CHAINS; i++){

		block *b = bc->get_deepest_child_by_chain_id( i );
		if( NULL == b ){
			cout <<"Something is wrong in mine_new_block: get_deepest return NULL"<<endl;
			exit(3);
		}
		if( NULL == b->nb ){
			cout <<"Something is wrong in mine_new_block: get_deepest return block with NULL nb pointer"<<endl;
			exit(3);
		}
		if ( b->nb->next_rank > trailing_block->nb->next_rank ){
			trailing_block = b;
			trailing_id = i;
		}

		leaves.push_back( blockhash_to_string( b->hash ) );
	}

	// Make a complete binary tree
	uint32_t tot_size_add = (int)pow(2,ceil( log(leaves.size()) / log(2) )) - leaves.size();
	for( int i=0; i<tot_size_add ; i++)
		leaves.push_back( EMPTY_LEAF );

	// Add randomness (LATER REMOVE IT)


	// hash to produce the hash of the new block
	//string h = sha256(all_hash);
	string merkle_root_chains = compute_merkle_tree_root( leaves );
	string merkle_root_txs = to_string(rng());
	string h = sha256( merkle_root_chains + merkle_root_txs );


	// Determine the chain where it should go
	uint32_t chain_id = get_chain_id_from_hash(h);

	// Determine the new block
	BlockHash new_block = string_to_blockhash( h );


	// Create file holding the whole block
	// Supposedly composed of transactions
	uint32_t no_txs = create_transaction_block( new_block , ser->get_server_folder()+"/"+blockhash_to_string( new_block ) ); 
	if( 0 == no_txs  ) {
		printf("Cannot create the file with transactions\n");
		fflush(stdout);
		return 0;
	}


	// Find Merkle path for the winning chain
	vector <string> proof_new_chain = compute_merkle_proof( leaves, chain_id );

	// Find Merkle path for the trailing chain
	vector <string> proof_trailing_chain = compute_merkle_proof( leaves, trailing_id );

	// Last block of the chain where new block will be mined
	block *parent = bc->get_deepest_child_by_chain_id( chain_id );


	network_block nb;
	nb.chain_id = chain_id;
	nb.parent = parent->hash;
	nb.hash = new_block;
	nb.trailing = trailing_block->hash;
	nb.merkle_root_chains = merkle_root_chains;
	nb.merkle_root_txs = merkle_root_txs;
	nb.proof_new_chain = proof_new_chain;
	nb.proof_trailing_chain = proof_trailing_chain;
	nb.no_txs = no_txs;
	nb.rank  = parent->nb->next_rank ;
	nb.next_rank  = trailing_block->nb->next_rank;
	if (nb.next_rank <= nb.rank ) nb.next_rank = nb.rank + 1;


	nb.depth = parent->nb->depth + 1;
	unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
	nb.time_mined    = time_of_now;
	nb.time_received = time_of_now;
	for( int j=0; j<NO_T_DISCARDS; j++){
		nb.time_commited[j] = 0;
		nb.time_partial[j] = 0;
	}


	// Add the block to the chain
	bc->add_block_by_parent_hash_and_chain_id( parent->hash, new_block, chain_id, nb );
	if( PRINT_MINING_MESSAGES) {
		printf("\033[33;1m[+] Mined block on chain[%d] : [%lx %lx]\n\033[0m", chain_id, parent->hash, new_block);
		fflush(stdout);	
	}


	// Set block flag as full block
    block * bz = bc->find_block_by_hash_and_chain_id( new_block, chain_id ); 
	if (NULL != bz && NULL != bz->nb ){
		bz->is_full_block = true;
	}

	// Increase the miner counter
	bc->add_mined_block();


	// Send the block to peers
	ser->send_block_to_peers( &nb );



	bc->locker_write = false;
	l.unlock();
	bc->can_write.notify_one();


	return chain_id;
}


uint32_t get_mine_time_in_milliseconds( ) 
{

	std::exponential_distribution<double> exp_dist (1.0/EXPECTED_MINE_TIME_IN_MILLISECONDS);
	uint32_t msec = exp_dist(rng);
	//printf("msec: %0.1f\n", (float)msec/1000);

	if( PRINT_MINING_MESSAGES) {
		printf("\033[33;1m[ ] Will mine new block in  %.3f  seconds \n\033[0m", (float)msec/1000 );
		fflush(stdout);
	}
	
	return msec;
}


void miner( Blockchain *bc)
{

	if (! CAN_INTERRUPT)
	    boost::this_thread::sleep(boost::posix_time::milliseconds(get_mine_time_in_milliseconds() ));
	else{

		try{
		    boost::this_thread::sleep(boost::posix_time::milliseconds(get_mine_time_in_milliseconds() ));
		}
		catch (boost::thread_interrupted &){

			if( PRINT_INTERRUPT_MESSAGES){
				printf("\033[35;1mInterrupt mining, recieved new block from a peer \n\033[0m");
				fflush(stdout);
			}

			miner( bc );
		}
	}

	if( total_mined >= MAX_MINE_BLOCKS) return;
	total_mined++;

	// Incorporate new block into the blockchain and pass it to peers
	if ( NULL != ser )
    	mine_new_block(bc);

  	// Mine next block
    miner( bc );


}
