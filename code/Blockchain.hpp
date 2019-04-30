#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include  <condition_variable>
#include "params.h"

using namespace std;

extern string my_ip;
extern uint32_t my_port; 



typedef uint64_t BlockHash;



/*
 *
 * 'block' is one node in the tree chain 
 *
 */

typedef struct networkblocks{

 	uint32_t chain_id;
 	BlockHash parent;
 	BlockHash hash;
	BlockHash trailing;
	string merkle_root_chains;
	string merkle_root_txs;
	vector <string> proof_new_chain;
	vector <string> proof_trailing_chain;
	uint32_t no_txs;
	uint32_t depth;
	uint32_t rank;
	uint32_t next_rank;
	unsigned long time_mined;
	unsigned long time_received;
	unsigned long time_commited[NO_T_DISCARDS];
	unsigned long time_partial[NO_T_DISCARDS];


}network_block;

typedef struct hashes{

	BlockHash hash;
	network_block *nb;
	bool is_full_block;

	struct hashes *left, *right;
	struct hashes *parent;
	struct hashes *child;
	struct hashes *sibling;
}block;


typedef struct incomplete_blocks{
	block *b;
	struct incomplete_blocks *next;
	unsigned long last_asked;
	uint32_t no_asks;
}block_incomplete;







class Blockchain
{
public:

	Blockchain(BlockHash hashes[]);
	bool add_received_block( uint32_t chain_id, BlockHash parent, BlockHash hash, network_block nb, bool &added );
	void specific_print_blockchain();
	block *find_block_by_hash_and_chain_id( BlockHash hash, uint32_t chain_id );
	block *find_incomplete_block_by_hash_and_chain_id(BlockHash hash, uint32_t chain_id);
	block_incomplete *get_incomplete_chain( uint32_t chain_id);
	block *get_deepest_child_by_chain_id( uint32_t chain_id) ;
	bool have_full_block( uint32_t chain_id, BlockHash hash);
    bool still_waiting_for_full_block(  BlockHash hash, unsigned long time_of_now);

	bool add_block_by_parent_hash_and_chain_id( BlockHash parenthash, BlockHash new_block, uint32_t chain_id, network_block nb);

	vector<BlockHash> get_incomplete_chain_hashes( uint32_t chain_id , unsigned long time_of_now );
	void remove_waiting_blocks( unsigned long time_of_now );
	vector< pair <BlockHash, uint32_t> > get_non_full_blocks( unsigned long time_of_now );
	
	void set_block_full( uint32_t chain_id, BlockHash hash, string misc );
	void add_mined_block();

	void update_blocks_commited_time();

	
	bool locker_write = false;
	std::mutex lock;
	std::condition_variable can_write;
	


private:

	block *chains[MAX_CHAINS];
	block_incomplete *inchains[MAX_CHAINS];
	block *deepest[MAX_CHAINS];
	map<BlockHash,pair <int,unsigned long> > received_non_full_blocks;
	map<BlockHash,unsigned long > waiting_for_full_blocks;
	unsigned long mined_blocks, processed_full_blocks, total_received_blocks;
	unsigned long long receiving_latency;
	unsigned long receving_total;
	unsigned long long commited_latency[NO_T_DISCARDS];
	unsigned long commited_total[NO_T_DISCARDS];
	unsigned long long partially_latency[NO_T_DISCARDS];
	unsigned long partially_total[NO_T_DISCARDS];

	block_incomplete *is_incomplete_hash( block_incomplete *l, BlockHash hash);
	block_incomplete *remove_one_chain( block_incomplete *l, block_incomplete *to_be_removed);

	void add_subtree_to_received_non_full( block *b, uint32_t chain_id );


	void print_blocks( block *root );
	block *find_block_by_hash( block *b, BlockHash hash );
	bool add_block_by_parent_hash( block **root, BlockHash parent, BlockHash hash);

	block *find_max_depth( block *r );

	block *find_deepest_child( block *r);
	int find_number_of_nodes( block *r);


	void print_one( block *r);
	void print_heaviest_chain ( block *r);
	void print_full_tree( block *root );



	block *insert_block_only_by_hash( block *r, BlockHash hash, block **newnode);
	void print_hash_tree( block *root);
	block *insert_one_node( block *r, block *subtree);
	block *insert_subtree_by_hash( block *r, block *subtree);



	block_incomplete *add_block_to_incomplete(block_incomplete *l, BlockHash parent_hash, BlockHash child_hash);
	bool is_in_incomplete(block_incomplete *l, BlockHash parent_hash, BlockHash child_hash);
	block *find_incomplete_block(block_incomplete *l, BlockHash child_hash);
	void print_all_incomplete_chains( block_incomplete *l);
	int find_number_of_incomplete_blocks(block_incomplete *l);

};




























#endif