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

#include "configuration.h"
#include "misc.h"

// Number of chains
uint32_t CHAINS = 1000;
//
uint32_t MAX_PEERS = 4;
//
uint32_t COEFF = 4;
//
// Expected mine time
uint32_t EXPECTED_MINE_TIME_IN_MILLISECONDS = int( COEFF * 0.25 * 1000);
//
// Ask for parents of incomplete blocks
uint32_t ASK_FOR_INCOMPLETE_EACH_MILLISECONDS = 50;
uint32_t ASK_FOR_INCOMPLETE_INDIVIDUAL_MILLISECONDS = 2000 ;
uint32_t NO_ASKS_BEFORE_REMOVING = 600;
// Ask for full blocks 
uint32_t ASK_FOR_FULL_BLOCKS_EACH_MILLISECONDS = 50;
uint32_t ASK_FOR_FULL_BLOCKS_INDIVIDUAL_EACH_MILLISECONDS = 2000;
uint32_t MAX_WAIT_FOR_FULL_BLOCK_MILLSECONDS = 20000;
uint32_t MAX_ASK_NON_FULL_IN_ONE_GO = 100;
// Sing + verify transactions
uint32_t SIGN_TRANSACTIONS = 0;
uint32_t VERIFY_TRANSACTIONS = 0;
uint32_t ADDRESS_SIZE_IN_DWORDS = 5;
// NETWORK
uint32_t CONNECT_TO_PEERS_MILLISECONDS = 10000;
uint32_t RUN_NETWORK_EACH_MILLISECONDS = 20;
//
uint32_t MAX_ASK_BLOCKS = 4;
// HDD 
uint32_t WRITE_BLOCKS_TO_HDD = 0;
uint32_t WRITE_SESSIONS_TO_HDD = 0;
uint32_t WRITE_HASH_TO_HDD=0;
// PRINTING
uint32_t PRINT_BLOCKCHAIN_EACH_MILLISECONDS = 2000;
//
uint32_t PRINT_SENDING_MESSAGES = 0;
uint32_t PRINT_RECEIVING_MESSAGES = 0;
uint32_t PRINT_MINING_MESSAGES = 0;
uint32_t PRINT_INTERRUPT_MESSAGES = 1;
uint32_t PRINT_PEER_CONNECTION_MESSAGES = 1;
uint32_t PRINT_TRANSMISSION_ERRORS = 0;
uint32_t PRINT_VERIFYING_TXS = 0;
// Stop the miner even after receiving a block
uint32_t CAN_INTERRUPT = 0;
// Cease all mining after mining MAX_MINE_BLOCKS blocks
uint32_t MAX_MINE_BLOCKS = 300000;
uint32_t BLOCK_SIZE_IN_BYTES = ( COEFF * 8*1024);
// 
uint32_t REJECT_CONNECTIONS_FROM_UNKNOWNS  = 1;
//
uint32_t STORE_HASH_FREQ = 1;
uint32_t STORE_HASH_MINUS = 6;
//
uint32_t PING_MIN_WAIT=5;
uint32_t PING_MAX_WAIT=10;
uint32_t PING_REPEAT=1000;
//
uint32_t STORE_BLOCKS=1;
uint32_t BLOCKS_STORE_FREQUENCY = 1000;
//
uint32_t NO_DISCARD_LOCAL=6;
uint32_t UPDATE_COMMITED_TIME_EACH_MILLISECONDS=500;
//
bool fake_transactions = true;
//
uint32_t T_DISCARD[6] = { 6 };


set<string> KNOWN_VARS = {
    "CHAINS",
	"MAX_PEERS", 
	"COEFF", 
	"EXPECTED_MINE_TIME_IN_MILLISECONDS", 
	"ASK_FOR_INCOMPLETE_EACH_MILLISECONDS", "ASK_FOR_INCOMPLETE_INDIVIDUAL_MILLISECONDS","NO_ASKS_BEFORE_REMOVING",
	"ASK_FOR_FULL_BLOCKS_EACH_MILLISECONDS","ASK_FOR_FULL_BLOCKS_INDIVIDUAL_EACH_MILLISECONDS","MAX_WAIT_FOR_FULL_BLOCK_MILLSECONDS","MAX_ASK_NON_FULL_IN_ONE_GO",
	"SIGN_TRANSACTIONS","VERIFY_TRANSACTIONS","ADDRESS_SIZE_IN_DWORDS",
	"CONNECT_TO_PEERS_MILLISECONDS","RUN_NETWORK_EACH_MILLISECONDS",
	"MAX_ASK_BLOCKS",
	"WRITE_BLOCKS_TO_HDD","WRITE_SESSIONS_TO_HDD","WRITE_HASH_TO_HDD",
	"PRINT_BLOCKCHAIN_EACH_MILLISECONDS",
	"PRINT_SENDING_MESSAGES","PRINT_RECEIVING_MESSAGES","PRINT_MINING_MESSAGES","PRINT_INTERRUPT_MESSAGES","PRINT_PEER_CONNECTION_MESSAGES","PRINT_TRANSMISSION_ERRORS","PRINT_VERIFYING_TXS",
	"CAN_INTERRUPT",
	"MAX_MINE_BLOCKS",
	"BLOCK_SIZE_IN_BYTES",
    "REJECT_CONNECTIONS_FROM_UNKNOWNS", 
    "STORE_HASH_FREQ","STORE_HASH_MINUS", 
    "PING_MIN_WAIT","PING_MAX_WAIT","PING_REPEAT",
    "STORE_BLOCKS","BLOCKS_STORE_FREQUENCY", 
    "NO_DISCARD_LOCAL","UPDATE_COMMITED_TIME_EACH_MILLISECONDS"


};


void set_configuration(string filepath)
{


	ifstream infile(  filepath.c_str() );
    string l;
    while( getline(infile, l) ){
    	vector<std::string> sp = split( l, "=");
    	if( sp.size() == 2){
    		trim(sp[0]);
    		trim(sp[1]);
    		string vname = sp[0];
    		bool converted = true;
    		unsigned long vvalue = safe_stoull( sp[1], converted );
    		if ( !converted ) continue;

    		if( KNOWN_VARS.find(vname) == KNOWN_VARS.end() ) continue;

    		printf("\t ::: %s = %ld\n", vname.c_str(), vvalue  );

            if (    vname == "CHAINS" ) CHAINS = vvalue;
    		else if(vname == "MAX_PEERS" ) MAX_PEERS = vvalue;
    		else if(vname == "EXPECTED_MINE_TIME_IN_MILLISECONDS" ) EXPECTED_MINE_TIME_IN_MILLISECONDS = vvalue;
    		else if(vname == "ASK_FOR_INCOMPLETE_EACH_MILLISECONDS" ) ASK_FOR_INCOMPLETE_EACH_MILLISECONDS = vvalue;
    		else if(vname == "ASK_FOR_INCOMPLETE_INDIVIDUAL_MILLISECONDS" ) ASK_FOR_INCOMPLETE_INDIVIDUAL_MILLISECONDS = vvalue;
    		else if(vname == "NO_ASKS_BEFORE_REMOVING" ) NO_ASKS_BEFORE_REMOVING = vvalue;
    		else if(vname == "ASK_FOR_FULL_BLOCKS_EACH_MILLISECONDS" ) ASK_FOR_FULL_BLOCKS_EACH_MILLISECONDS = vvalue;
    		else if(vname == "ASK_FOR_FULL_BLOCKS_INDIVIDUAL_EACH_MILLISECONDS" ) ASK_FOR_FULL_BLOCKS_INDIVIDUAL_EACH_MILLISECONDS = vvalue;
    		else if(vname == "MAX_WAIT_FOR_FULL_BLOCK_MILLSECONDS" ) MAX_WAIT_FOR_FULL_BLOCK_MILLSECONDS = vvalue;
    		else if(vname == "MAX_ASK_NON_FULL_IN_ONE_GO" ) MAX_ASK_NON_FULL_IN_ONE_GO = vvalue;
    		else if(vname == "SIGN_TRANSACTIONS" ) SIGN_TRANSACTIONS = vvalue;
    		else if(vname == "VERIFY_TRANSACTIONS" ) VERIFY_TRANSACTIONS = vvalue;
    		else if(vname == "ADDRESS_SIZE_IN_DWORDS" ) ADDRESS_SIZE_IN_DWORDS = vvalue;
    		else if(vname == "CONNECT_TO_PEERS_MILLISECONDS" ) CONNECT_TO_PEERS_MILLISECONDS = vvalue;
    		else if(vname == "RUN_NETWORK_EACH_MILLISECONDS" ) RUN_NETWORK_EACH_MILLISECONDS = vvalue;
    		else if(vname == "MAX_ASK_BLOCKS" ) MAX_ASK_BLOCKS = vvalue;
    		else if(vname == "WRITE_BLOCKS_TO_HDD" ) WRITE_BLOCKS_TO_HDD = vvalue;
    		else if(vname == "WRITE_SESSIONS_TO_HDD" ) WRITE_SESSIONS_TO_HDD = vvalue;
            else if(vname == "WRITE_HASH_TO_HDD" ) WRITE_HASH_TO_HDD = vvalue;
    		else if(vname == "PRINT_BLOCKCHAIN_EACH_MILLISECONDS" ) PRINT_BLOCKCHAIN_EACH_MILLISECONDS = vvalue;
    		else if(vname == "PRINT_SENDING_MESSAGES" ) PRINT_SENDING_MESSAGES = vvalue;
    		else if(vname == "PRINT_RECEIVING_MESSAGES" ) PRINT_RECEIVING_MESSAGES = vvalue;
    		else if(vname == "PRINT_MINING_MESSAGES" ) PRINT_MINING_MESSAGES = vvalue;
    		else if(vname == "PRINT_INTERRUPT_MESSAGES" ) PRINT_INTERRUPT_MESSAGES = vvalue;
    		else if(vname == "PRINT_PEER_CONNECTION_MESSAGES" ) PRINT_PEER_CONNECTION_MESSAGES = vvalue;
    		else if(vname == "PRINT_TRANSMISSION_ERRORS" ) PRINT_TRANSMISSION_ERRORS = vvalue;
    		else if(vname == "PRINT_VERIFYING_TXS" ) PRINT_VERIFYING_TXS = vvalue;
    		else if(vname == "CAN_INTERRUPT" ) CAN_INTERRUPT = vvalue;
    		else if(vname == "MAX_MINE_BLOCKS" ) MAX_MINE_BLOCKS = vvalue;
    		else if(vname == "BLOCK_SIZE_IN_BYTES" ) BLOCK_SIZE_IN_BYTES = vvalue;
            else if(vname == "REJECT_CONNECTIONS_FROM_UNKNOWNS" ) REJECT_CONNECTIONS_FROM_UNKNOWNS = vvalue;
            else if(vname == "STORE_HASH_FREQ" ) STORE_HASH_FREQ = vvalue;
            else if(vname == "STORE_HASH_MINUS" ) STORE_HASH_MINUS = vvalue;
            else if(vname == "PING_MIN_WAIT" ) PING_MIN_WAIT = vvalue;
            else if(vname == "PING_MAX_WAIT" ) PING_MAX_WAIT = vvalue;
            else if(vname == "PING_REPEAT" ) PING_REPEAT = vvalue;
            else if(vname == "STORE_BLOCKS" ) STORE_BLOCKS = vvalue;
            else if(vname == "BLOCKS_STORE_FREQUENCY" ) BLOCKS_STORE_FREQUENCY = vvalue;
            else if(vname == "NO_DISCARD_LOCAL" ) NO_DISCARD_LOCAL = vvalue;
            else if(vname == "UPDATE_COMMITED_TIME_EACH_MILLISECONDS" ) UPDATE_COMMITED_TIME_EACH_MILLISECONDS = vvalue;
            
            
            
    		else if(vname == "COEFF" ) {COEFF = vvalue; EXPECTED_MINE_TIME_IN_MILLISECONDS= uint32_t( COEFF * 0.25 * 1000);BLOCK_SIZE_IN_BYTES = uint32_t( COEFF * 8*1024); }
    		else {
    			printf("\t[-]Cannot find appropriate for %s\n", vname.c_str());
    		}
    	}
    }
    infile.close();


}

