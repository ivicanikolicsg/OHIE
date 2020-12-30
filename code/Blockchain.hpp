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

typedef struct networkblocks{                   // a block.
                                                // the real block structure which is serialized on network.
                                                // An instance of process_block

 	uint32_t chain_id;                          // Corresponding chain id
 	BlockHash parent;                           // parent block hash
 	BlockHash hash;                             // block hash
	BlockHash trailing;                         // the block which had the greatest next_rank before mining this block
	uint32_t trailing_id;                       // the chain in which trailing block is located on
	string merkle_root_chains;                  // merkle root of deepest blocks
	string merkle_root_txs;                     // trxs merkle root
	vector <string> proof_new_chain;            // merkle proof of trailing block
	uint32_t no_txs;                            // trxs number in block
	uint32_t depth;                             // block depth in chain
	uint32_t rank;                              // rank
	uint32_t next_rank;                         // next_rank
	unsigned long time_mined;                   // mining time
	unsigned long time_received;                // receiving time or mining time
	unsigned long time_commited[NO_T_DISCARDS]; // the time in which this block is confirmed on
	unsigned long time_partial[NO_T_DISCARDS];  // the time in which this block is partially confirmed on


}network_block;

typedef struct hashes{              // it is only an abstraction for a block.
                                    // node makes this structure in order to work with network_blocks

	BlockHash hash;                 // block hash
	network_block *nb;              // network block links to this block
	bool is_full_block;             // These blocks are not full. In order to do this, only their hash is stored and all other stuffs are ignored for later filling.

	struct hashes *left, *right;    // These pointers make a binary search tree.
	                                // Only needs for searching through the chain, sorts the blocks. Increasing from left to right
	struct hashes *parent;          // Parent block in chain.
	struct hashes *child;           // Child block in chain
	struct hashes *sibling;         // Handling fork.
}block;


typedef struct incomplete_blocks{   // when a block arrives, we check whether it's parent is in chain or not. If it is, then we simply add the child to its parent and upgrade our chain
                                    // and if it isn't, we have to put this new block in incomplete blocks, hoping that someday we can find the parent.
                                    // each incomplete_block is included in a chain of incomplete_blocks, and the roots of these chains form a link list. For a new incomplete_block, if its parent is in one of the existing incomplete chains, then we add it to the paren's chain as its children.
	block *b;                       // corresponding block structure
	struct incomplete_blocks *next; // this variable is set when this block is one of the root blocks in an incomplete chain of blocks, and we find a new incomplete_block that its parent is neither located in not previous incomplete chains.
	unsigned long last_asked;       // last time it was asked from other peers
	uint32_t no_asks;               // number of times it is asked from other peers
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
	map<BlockHash,pair <int,unsigned long> > received_non_full_blocks;                          // All the newly arrived blocks are non_full, and when its corresponding transactions are find and attached to it, then it will change state to full block.
	map<BlockHash,unsigned long > waiting_for_full_blocks;                                      // used when you have send a ask_full_block request. then the requested block will be added here in order not to ask it multiple times.
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