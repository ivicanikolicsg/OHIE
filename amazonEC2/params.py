import os
from regions import *
from instances import *


FOLDER_NODE_CPP = "node_cpp_code"
FILE_CONFIG_CPP = FOLDER_NODE_CPP+'/_configuration'
FILE_NETWORK_CPP = FOLDER_NODE_CPP+'/_network'
FILE_NETWORK_PEERS_CPP = FOLDER_NODE_CPP+'/_peer_ips'

username = "ubuntu"

instance_type='m4.2xlarge'

coef=64.0

# Blockchain network settings
chains=1024  
max_instances=1000
nodes=50
start_port = 8000
print_state_each_seconds = 5
blocksize=int(20*1024)
blocks_per_second=50


STORE_BLOCKS=1
BLOCKS_STORE_FREQUENCY=2
PRINT_SENDING_MESSAGES = 0
PRINT_RECEIVING_MESSAGES = 0
PRINT_MINING_MESSAGES = 1
PRINT_INTERRUPT_MESSAGES = 0
PRINT_PEER_CONNECTION_MESSAGES = 0
PRINT_TRANSMISSION_ERRORS = 0
PRINT_VERIFYING_TXS = 0
WRITE_SESSIONS_TO_HDD=0
ASK_FOR_INCOMPLETE_EACH_MILLISECONDS=50
ASK_FOR_INCOMPLETE_INDIVIDUAL_MILLISECONDS=150
ASK_FOR_FULL_BLOCKS_EACH_MILLISECONDS = 50
ASK_FOR_FULL_BLOCKS_INDIVIDUAL_EACH_MILLISECONDS=500 #300
MAX_WAIT_FOR_FULL_BLOCK_MILLSECONDS = 200000#10000
MAX_ASK_NON_FULL_IN_ONE_GO = 1000
WRITE_HASH_TO_HDD=1
STORE_HASH_FREQ = 1
STORE_HASH_MINUS = 6
PING_MIN_WAIT=3
PING_MAX_WAIT=6
PING_REPEAT=3
NO_DISCARD_LOCAL=6


def create_cpp_network_config_files(instances, config_file):
    
    # Get all possible network IP addresses and 
    # Get number of running instances
    no_run_instances = 0
    with open( FILE_NETWORK_PEERS_CPP , "w" ) as f:
        for i in instances:
            if is_running_instance( i ):
                no_run_instances += 1
                f.write( get_instance_ip(i) +"\n")
                f.write( get_instance_ip_private(i) +"\n")
        f.close()

    # Create the network file
    with open( FILE_NETWORK_CPP , "w" ) as f:
        for i in instances:
            if is_running_instance( i ):
                ip = get_instance_ip( i )
                for j in range( int( nodes  ) ):
                    f.write(ip+":"+str(start_port + j)+"\n")
        f.close()

    # create file _configuration
    with open( config_file , "w" ) as f:

        if 'chains' in globals() :  f.write('CHAINS='+str(chains)+'\n')

        if 'blocksize' in globals() :  f.write('BLOCK_SIZE_IN_BYTES='+str(blocksize)+'\n')
        if 'nodes' in globals() :  f.write('EXPECTED_MINE_TIME_IN_MILLISECONDS='+str( int(float(nodes*no_run_instances)/blocks_per_second * 1000) if blocks_per_second> 0 else 10000000000 )+'\n')
        if 'print_state_each_seconds' in globals() :  f.write('PRINT_BLOCKCHAIN_EACH_MILLISECONDS='+str( int(print_state_each_seconds * 1000) )+'\n')
        if 'PRINT_SENDING_MESSAGES' in globals() :  f.write('PRINT_SENDING_MESSAGES='+str( PRINT_SENDING_MESSAGES )+'\n')
        if 'PRINT_RECEIVING_MESSAGES' in globals() :  f.write('PRINT_RECEIVING_MESSAGES='+str( PRINT_RECEIVING_MESSAGES )+'\n')
        if 'PRINT_MINING_MESSAGES' in globals() :  f.write('PRINT_MINING_MESSAGES='+str( PRINT_MINING_MESSAGES )+'\n')
        if 'PRINT_INTERRUPT_MESSAGES' in globals() :  f.write('PRINT_INTERRUPT_MESSAGES='+str( PRINT_INTERRUPT_MESSAGES )+'\n')
        if 'PRINT_PEER_CONNECTION_MESSAGES' in globals() :  f.write('PRINT_PEER_CONNECTION_MESSAGES='+str( PRINT_PEER_CONNECTION_MESSAGES )+'\n')
        if 'PRINT_TRANSMISSION_ERRORS' in globals() :  f.write('PRINT_TRANSMISSION_ERRORS='+str( PRINT_TRANSMISSION_ERRORS )+'\n')
        if 'PRINT_VERIFYING_TXS' in globals() :  f.write('PRINT_VERIFYING_TXS='+str( PRINT_VERIFYING_TXS )+'\n')
        if 'WRITE_SESSIONS_TO_HDD' in globals() :  f.write('WRITE_SESSIONS_TO_HDD='+str( WRITE_SESSIONS_TO_HDD )+'\n')
        if 'ASK_FOR_INCOMPLETE_EACH_MILLISECONDS' in globals() :  f.write('ASK_FOR_INCOMPLETE_EACH_MILLISECONDS='+str( ASK_FOR_INCOMPLETE_EACH_MILLISECONDS )+'\n')
        if 'ASK_FOR_INCOMPLETE_INDIVIDUAL_MILLISECONDS' in globals() :  f.write('ASK_FOR_INCOMPLETE_INDIVIDUAL_MILLISECONDS='+str( ASK_FOR_INCOMPLETE_INDIVIDUAL_MILLISECONDS )+'\n')
        if 'ASK_FOR_FULL_BLOCKS_EACH_MILLISECONDS' in globals() :  f.write('ASK_FOR_FULL_BLOCKS_EACH_MILLISECONDS='+str( ASK_FOR_FULL_BLOCKS_EACH_MILLISECONDS )+'\n')
        if 'ASK_FOR_FULL_BLOCKS_INDIVIDUAL_EACH_MILLISECONDS' in globals() :  f.write('ASK_FOR_FULL_BLOCKS_INDIVIDUAL_EACH_MILLISECONDS='+str( ASK_FOR_FULL_BLOCKS_INDIVIDUAL_EACH_MILLISECONDS )+'\n')
        if 'MAX_WAIT_FOR_FULL_BLOCK_MILLSECONDS' in globals() :  f.write('MAX_WAIT_FOR_FULL_BLOCK_MILLSECONDS='+str( MAX_WAIT_FOR_FULL_BLOCK_MILLSECONDS )+'\n')
        if 'MAX_ASK_NON_FULL_IN_ONE_GO' in globals() :  f.write('MAX_ASK_NON_FULL_IN_ONE_GO='+str( MAX_ASK_NON_FULL_IN_ONE_GO )+'\n')
        if 'WRITE_HASH_TO_HDD' in globals() :  f.write('WRITE_HASH_TO_HDD='+str( WRITE_HASH_TO_HDD )+'\n')
        if 'STORE_HASH_FREQ' in globals() :  f.write('STORE_HASH_FREQ='+str( STORE_HASH_FREQ )+'\n')
        if 'STORE_HASH_MINUS' in globals() :  f.write('STORE_HASH_MINUS='+str( STORE_HASH_MINUS )+'\n')
        if 'PING_MIN_WAIT' in globals() :  f.write('PING_MIN_WAIT='+str( PING_MIN_WAIT )+'\n')
        if 'PING_MAX_WAIT' in globals() :  f.write('PING_MAX_WAIT='+str( PING_MAX_WAIT )+'\n')
        if 'PING_REPEAT' in globals() :  f.write('PING_REPEAT='+str( PING_REPEAT )+'\n')
        if 'STORE_BLOCKS' in globals() :  f.write('STORE_BLOCKS='+str( STORE_BLOCKS )+'\n')
        if 'BLOCKS_STORE_FREQUENCY' in globals() :  f.write('BLOCKS_STORE_FREQUENCY='+str( BLOCKS_STORE_FREQUENCY )+'\n')
        if 'NO_DISCARD_LOCAL' in globals() :  f.write('NO_DISCARD_LOCAL='+str( NO_DISCARD_LOCAL )+'\n')
        if 'RUN_NETWORK_EACH_MILLISECONDS' in globals() :  f.write('RUN_NETWORK_EACH_MILLISECONDS='+str( RUN_NETWORK_EACH_MILLISECONDS )+'\n')



        f.close()
















