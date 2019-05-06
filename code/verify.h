#ifndef VERIFY_H
#define VERIFY_H

#include <stdint.h>
#include <string>
#include <iostream>
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

using namespace std;


string blockhash_to_string( BlockHash b);
BlockHash string_to_blockhash( string h );
uint32_t get_chain_id_from_hash( string h);
string compute_merkle_tree_root( vector<string> leaves );
vector <string> compute_merkle_proof( vector<string> leaves, int index );
int merkle_proof_length();

bool verify_merkle_proof( vector <string> proof, BlockHash bh, string root, uint32_t index );




#endif