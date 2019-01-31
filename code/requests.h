#ifndef REQUESTS_H
#define REQUESTS_H

#include <iostream>
#include "Blockchain.hpp"
#include "MyServer.hpp"
#include "verify.h"
#include "params.h"

using namespace std;


string create__ask_block( uint32_t chain_id, BlockHash hash, uint32_t my_depth, uint32_t hash_depth );
bool parse__ask_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash, uint32_t &max_number_of_blocks   );


string create__process_block( network_block *nb );
bool parse__process_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, network_block &nb   );

string create__got_full_block(uint32_t chain_id, BlockHash hash );
bool parse__got_full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash  );


string create__have_full_block( uint32_t chain_id, BlockHash hash);
bool parse__have_full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash  );


string create__ask_full_block( uint32_t chain_id, BlockHash hash);
bool parse__ask_full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash  );


string create__full_block( uint32_t chain_id, BlockHash hash, tcp_server *ser, Blockchain *bc );
bool parse__full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash, string &txs, network_block &nb, unsigned long &sent_time  );


string create__ping( string tt, uint32_t dnext, unsigned long tsec, int mode);
bool parse__ping( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, string &tt, uint32_t &dnext, unsigned long &tsec , int &mode  );



bool key_present( string key, map<string,int> &passed );




#endif
