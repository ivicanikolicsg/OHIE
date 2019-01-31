#ifndef PARAMS_H
#define PARAMS_H

#include <iostream>
#include <string>


// Max number of chains
// The actual number of chains is specified in CHAINS
#define MAX_CHAINS (8*1024)


// Folder&file names
#define FOLDER_BLOCKCHAIN "_Blockchains"
#define FOLDER_SESSIONS "_Sessions"
#define FOLDER_HASHES "_Hashes"
#define FOLDER_PINGS "_Pings"
#define FOLDER_BLOCKS "_Blocks"
#define FILE_CONFIGURATION	"_configuration"
#define FILE_ECC_KEY	"_ecc_key"
#define FILE_PEER_IPS	"_peer_ip_allowed"

// Dummy strings
#define DUMMY_SIGNATURE "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
#define EMPTY_LEAF "00000000000000000000000000000000"



/*
 * All of the below values can be set before runtime (no need to compile!!!) in the file "configuration", by
 * [VARIABLE_NAME],[VARIABLE_VALUE]
 */
extern uint32_t CHAINS;

extern uint32_t MAX_PEERS;

extern uint32_t COEFF;

// Expected mine time
extern uint32_t EXPECTED_MINE_TIME_IN_MILLISECONDS;
//#define EXPECTED_MINE_TIME_IN_MILLISECONDS ( COEFF * 0.25 * 1000)

// Ask for parents of incomplete blocks
extern uint32_t ASK_FOR_INCOMPLETE_EACH_MILLISECONDS;
extern uint32_t ASK_FOR_INCOMPLETE_INDIVIDUAL_MILLISECONDS; //2000
extern uint32_t NO_ASKS_BEFORE_REMOVING;
// Ask for full blocks 
extern uint32_t ASK_FOR_FULL_BLOCKS_EACH_MILLISECONDS;
extern uint32_t ASK_FOR_FULL_BLOCKS_INDIVIDUAL_EACH_MILLISECONDS;
extern uint32_t MAX_WAIT_FOR_FULL_BLOCK_MILLSECONDS;
extern uint32_t MAX_ASK_NON_FULL_IN_ONE_GO;
// Sing + verify transactions
extern uint32_t SIGN_TRANSACTIONS;
extern uint32_t VERIFY_TRANSACTIONS;
extern uint32_t ADDRESS_SIZE_IN_DWORDS;
// NETWORK
extern uint32_t CONNECT_TO_PEERS_MILLISECONDS;
extern uint32_t RUN_NETWORK_EACH_MILLISECONDS;
//
extern uint32_t MAX_ASK_BLOCKS;
// HDD 
extern uint32_t WRITE_BLOCKS_TO_HDD;
extern uint32_t WRITE_SESSIONS_TO_HDD;
extern uint32_t WRITE_HASH_TO_HDD;
// PRINTING
extern uint32_t PRINT_BLOCKCHAIN_EACH_MILLISECONDS;
//
extern uint32_t PRINT_SENDING_MESSAGES;
extern uint32_t PRINT_RECEIVING_MESSAGES;
extern uint32_t PRINT_MINING_MESSAGES;
extern uint32_t PRINT_INTERRUPT_MESSAGES;
extern uint32_t PRINT_PEER_CONNECTION_MESSAGES;
extern uint32_t PRINT_TRANSMISSION_ERRORS;
extern uint32_t PRINT_VERIFYING_TXS;
// Stop the miner even after receiving a block
extern uint32_t CAN_INTERRUPT;
// Cease all mining after mining MAX_MINE_BLOCKS blocks
extern uint32_t MAX_MINE_BLOCKS;
extern uint32_t BLOCK_SIZE_IN_BYTES;
//
extern uint32_t REJECT_CONNECTIONS_FROM_UNKNOWNS;
//
extern uint32_t STORE_HASH_FREQ;
extern uint32_t STORE_HASH_MINUS;
//
extern uint32_t PING_MIN_WAIT;
extern uint32_t PING_MAX_WAIT;
extern uint32_t PING_REPEAT;
//
extern uint32_t STORE_BLOCKS ;
extern uint32_t BLOCKS_STORE_FREQUENCY ;
//
extern uint32_t NO_DISCARD_LOCAL;
extern uint32_t UPDATE_COMMITED_TIME_EACH_MILLISECONDS;
//
extern bool fake_transactions;


#define NO_T_DISCARDS 1
extern uint32_t T_DISCARD[NO_T_DISCARDS];


#endif
