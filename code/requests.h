#ifndef REQUESTS_H
#define REQUESTS_H

#include <iostream>
#include "Blockchain.hpp"
#include "MyServer.hpp"
#include "verify.h"
#include "params.h"

using namespace std;


/*
 *  ask_block: returns several process_blocks
 *  process_block: returns process_blocks to other peers. if the parent is not find, then returns ask_block
 */

// returns as much as block we can from the specified chain. Used for finding the parent of newly arrived child.
string create__ask_block( uint32_t chain_id, BlockHash hash, uint32_t my_depth, uint32_t hash_depth );
bool parse__ask_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash, uint32_t &max_number_of_blocks   );

// respond of ask_block. If I don't have the parent block, the procedure follows sending another ask_block
string create__process_block( network_block *nb );
bool parse__process_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, network_block &nb   );

// used when peer is looking for a full block with hash `hash'.
string create__got_full_block(uint32_t chain_id, BlockHash hash );
bool parse__got_full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash  );

// response of got full block in case of having the block with the same hash `hash'.
// note that this API does not send the full block. It only notifies the sender that I have the block.
string create__have_full_block( uint32_t chain_id, BlockHash hash);
bool parse__have_full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash  );

// after getting a have_full_block, we can ask_full_block from that peer in order to get the full_block.
// here we take care of not asking a same block multiple times.
string create__ask_full_block( uint32_t chain_id, BlockHash hash);
bool parse__ask_full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash  );

// returns full block.
string create__full_block( uint32_t chain_id, BlockHash hash, tcp_server *ser, Blockchain *bc );
bool parse__full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash, string &txs, network_block &nb, unsigned long &sent_time  );


string create__ping( string tt, uint32_t dnext, unsigned long tsec, int mode);
bool parse__ping( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, string &tt, uint32_t &dnext, unsigned long &tsec , int &mode  );



bool key_present( string key, map<string,int> &passed );




#endif
