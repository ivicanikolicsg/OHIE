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
#include "Blockchain.hpp"
#include "MyServer.hpp"


using namespace std;


extern unsigned long time_of_start;


/*
 *
 * Blockchain 
 * 
 */



block *bootstrap_chain( BlockHash initial_hash)
{
	block *root = (block *)malloc(sizeof(block));
	root->is_full_block = false;
	root->hash = initial_hash;
	root->left = root->right = root->child = root->parent = root->sibling = NULL;
	root->nb = NULL;

	return root;
}

block *Blockchain::find_block_by_hash( block *b, BlockHash hash )
{

	if (NULL == b) return NULL;

	
	if (b->hash > hash ) return find_block_by_hash( b->left, hash );
	else if (b->hash < hash ) return find_block_by_hash( b->right, hash);

	return b;

} 

block *Blockchain::insert_block_only_by_hash( block *r, BlockHash hash, block **newnode)
{
	if (NULL == r){
		block *t = (block *)malloc(sizeof(block));
		t->hash = hash;
		t->is_full_block = false;
		t->left = t->right = t->child = t->sibling = t->parent = NULL;
		*newnode = t;
		return t;
	}


	if (r->hash >= hash ) 		r->left 	= insert_block_only_by_hash ( r->left,  hash, newnode );
	else if (r->hash < hash) 	r->right 	= insert_block_only_by_hash ( r->right, hash, newnode );

	return r;
}



block *Blockchain::insert_one_node( block *r, block *subtree)
{
	if( NULL == r) return subtree;
	if( NULL == subtree ) return r;

	if ( r->hash > subtree->hash){
		if( r->left == NULL){
			r->left = subtree;
		}
		else
			r->left = insert_one_node(r->left, subtree);
	}
	else if ( r->hash < subtree->hash){
		if( r->right == NULL){
			r->right = subtree;
		}
		else
			r->right = insert_one_node(r->right, subtree);
	}
	else{
		cout <<"Same hash...bad"<<endl;
	}

	return r;


}

block *Blockchain::insert_subtree_by_hash( block *r, block *subtree)
{
	if (NULL == subtree) return r;

	block *left = subtree->left;
	block *right = subtree->right;

	subtree->left = subtree->right = NULL;
	r = insert_one_node( r, subtree );

	r = insert_subtree_by_hash(r, left);
	r = insert_subtree_by_hash(r, right);

	return r;
}


void Blockchain::print_blocks( block *root )
{
	if( NULL == root) return;

	print_blocks(root->left);
	printf("%8lx : %4d : %8lx : %d %d %d %d %d :  %d \n", root->hash, root->nb->depth, (root->parent == NULL)?0:root->parent->hash, 
											root->left != NULL, root->right != NULL, root->child != NULL, root->parent != NULL , root->sibling!=NULL, root->nb->depth );
	print_blocks(root->right);
}

void Blockchain::print_full_tree( block *root )
{
	if( NULL == root ) return;

	cout << hex<<root->hash << dec << "("<<root->nb->depth<<")"<< "  :  ";
	block *t = root->child;
	while (NULL != t){
		cout << hex<<t->hash << " ";
		t= t->sibling;
	}
	cout << endl;

	t = root->child;
	while (NULL != t){
		if (t->child != NULL)
			print_full_tree( t );
		t= t->sibling;
	}

}


bool Blockchain::add_block_by_parent_hash( block **root, BlockHash parent, BlockHash hash)
{
	// Find the parent block node by parent's blockhash
	block *p = find_block_by_hash( *root, parent );
	if (NULL == p){
		cout << "Cannot find parent for " << parent << endl;
		return false;
	}


	// Insert the new node (of the child)
	block *newnode = NULL;
	*root = insert_block_only_by_hash( *root, hash, &newnode );
	if( NULL == newnode ){
		cout << "Something is wrong, new node is NULL in 'add_child' " << endl;
		return false;
	}

	// Set the parent of the new node
	newnode->parent = p;

	// Set the new node as one of parents children
	if ( NULL == p->child ){
		p->child = newnode;
	}
	else{
		block *z = p->child;
		while ( z->sibling != NULL) z = z->sibling;
		z->sibling = newnode;
	}

	return true;
}




int Blockchain::find_number_of_nodes( block *r)
{
	if( NULL == r) return 0;

	int n = 0;
	block *c = r->child;
	while( NULL != c ){
		n += find_number_of_nodes( c );
		c = c->sibling;
	}

	return 1 + n;
}



void Blockchain::print_one( block *r)
{
	if( NULL == r) return;

	print_one(r->parent);
	printf("%lx (%d) ", r->hash, (NULL != r->nb)?r->nb->depth:-1);
	printf("%lx ", r->hash);
}



/*
 *
 * Incomplete 
 *
 */

block_incomplete *Blockchain::remove_one_chain( block_incomplete *l, block_incomplete *to_be_removed)
{
	if( NULL == l)
		return NULL;

	if( l == to_be_removed){
		block_incomplete *t = l->next;
		free(l);
		return t;	
	}
	else{
		block_incomplete *t = l;
		while( NULL != t && t->next != to_be_removed)
			t = t->next;

		if( t->next == to_be_removed){
			t->next = t->next->next;
			free(to_be_removed);
		}

		return l;
	}
}

block_incomplete *Blockchain::is_incomplete_hash( block_incomplete *l, BlockHash hash)
{
	if ( NULL == l) return NULL;

	block_incomplete *t = l;
	while( NULL != l && l->b->hash != hash )
		l = l->next;

	if (NULL != l && NULL != l->b && l->b->hash == hash)
		return l;

	return NULL;
}

bool Blockchain::is_in_incomplete(block_incomplete *l, BlockHash parent_hash, BlockHash child_hash)
{

	if ( NULL == l) return false;

	block_incomplete *t = l;
	while( NULL != t){
		block *b = find_block_by_hash( t->b, child_hash );
		if (NULL != b && b->parent!= NULL && b->parent->hash == parent_hash)
			return true;
		t = t->next;
	}

	return false;
}

block *Blockchain::find_incomplete_block(block_incomplete *l, BlockHash child_hash)
{

	if ( NULL == l) return NULL;

	block_incomplete *t = l;
	while( NULL != t){
		block *b = find_block_by_hash( t->b, child_hash );
		if (NULL != b )
			return b;
		t = t->next;
	}

	return NULL;
}


block_incomplete *Blockchain::add_block_to_incomplete(block_incomplete *l, BlockHash parent_hash, BlockHash child_hash)
{

	if ( NULL == l){

		block *bl = NULL;
		bl = bootstrap_chain ( parent_hash  );
		add_block_by_parent_hash( &bl, parent_hash, child_hash ) ;

		block_incomplete *bi;
		bi = (block_incomplete *)malloc(sizeof(block_incomplete));
		bi->b = bl;
		bi->next = NULL;
		bi->last_asked = 0;
		bi->no_asks = 0;
		return bi;
	}

	block_incomplete *tmp = l, *penultimate;
	block_incomplete *ch = NULL, *ph = NULL;
	while( NULL != tmp  ){

		if ( NULL == ch ) ch = (find_block_by_hash( tmp->b, child_hash ) != NULL) ? tmp : NULL;
		if ( NULL == ph ) ph = (find_block_by_hash( tmp->b, parent_hash) != NULL) ? tmp : NULL;

		penultimate = tmp;
		tmp = tmp->next;
	}
	
	// Neither parent nor child hash has been found
	if ( NULL == ch && NULL == ph ){
		block *bl = NULL;
		bl = bootstrap_chain ( parent_hash  );
		add_block_by_parent_hash( &bl, parent_hash, child_hash ) ;

		block_incomplete *bi;
		bi = (block_incomplete *)malloc(sizeof(block_incomplete));
		bi->b = bl;
		bi->next = NULL;
		bi->last_asked = 0;
		bi->no_asks = 0;
		penultimate->next = bi;
	}
	else if ( NULL == ch ){

		add_block_by_parent_hash( &(ph->b), parent_hash, child_hash ) ;
	}
	else if ( NULL == ph){
	
		block *bl = bootstrap_chain( parent_hash );
		block *tmp = ch->b;
	    bl = insert_subtree_by_hash( bl, ch->b );
	    bl->child = tmp;
	    tmp->parent = bl;
	    ch->b = bl;
	    ch->last_asked = 0;
	    ch->no_asks = 0;
	}
	else{
	
		block *Ztmp = ch->b;
		ph->b = insert_subtree_by_hash( ph->b, ch->b );
		block *parent_block = find_block_by_hash( ph->b, parent_hash);
		Ztmp->parent = parent_block;
		block *tmp = parent_block->child;
		if (NULL == tmp)
			parent_block->child = Ztmp;
		else{
			while(tmp->sibling != NULL)
				tmp = tmp->sibling;
			tmp->sibling = Ztmp;
		}


		l = remove_one_chain( l, ch );

	}


	return l;
}

void Blockchain::print_all_incomplete_chains( block_incomplete *l)
{
	if ( NULL == l) return;


	printf("\n");
	print_full_tree( l->b );
	print_all_incomplete_chains( l->next );

}


int Blockchain::find_number_of_incomplete_blocks(block_incomplete *l)
{
	if( NULL == l) return 0;

	int no = 0;
	while( NULL != l){
		no += find_number_of_nodes( l->b);
		l = l->next;
	}

	return no;
}





/*
 *
 * Blockchain
 *
 */


Blockchain::Blockchain(BlockHash hashes[MAX_CHAINS])
{
	cout <<"[ ] Bootstraping the blockchain ... ";

	for(int i=0; i<CHAINS; i++){
		this->chains[i] = bootstrap_chain( hashes[ i ] );
		this->inchains[i] = NULL;

		this->chains[i]->is_full_block = true;
		this->chains[i]->nb = new(network_block);
		this->chains[i]->nb->depth  = 0;
		this->chains[i]->nb->rank = 0;
		this->chains[i]->nb->next_rank = 0;
		this->chains[i]->nb->time_mined = 0;
		this->chains[i]->nb->time_received = 1;
		for( int j=0; j<NO_T_DISCARDS; j++){
			this->chains[i]->nb->time_commited[j] = 1;
			this->chains[i]->nb->time_partial[j]= 1;
		}
						

		this->deepest[i] = this->chains[i];
	}

	received_non_full_blocks.clear();
	waiting_for_full_blocks.clear();
	processed_full_blocks = 0;
	total_received_blocks = 0;
	mined_blocks = 0;


	receiving_latency = receving_total = 0;
	for( int j=0; j< NO_T_DISCARDS; j++){
		commited_latency[j] = 0;
		commited_total[j] = 0;
		partially_latency[j] = 0;
		partially_total[j] = 0;
	}


	cout <<" Done !" << endl;
	fflush(stdout);
}


void Blockchain::print_hash_tree( block *root)
{
	if( NULL == root) return;

	print_hash_tree(root->left);
	printf("Z: %lx\n", root->hash);
	print_hash_tree(root->right);


}

void Blockchain::add_subtree_to_received_non_full( block *b, uint32_t chain_id )
{
	if( NULL == b) return;

	BlockHash hash = b->hash;
	if ( received_non_full_blocks.find(hash) == received_non_full_blocks.end() && !have_full_block(chain_id, hash) ){
		received_non_full_blocks.insert( make_pair( hash, make_pair( chain_id, 0) ) );
		total_received_blocks ++;
	}

	block *c = b->child;
	while( NULL != c ){
		add_subtree_to_received_non_full( c, chain_id );
		c = c->sibling;
	}




}


block *Blockchain::find_max_depth( block *r )
{
	if( NULL == r) return NULL;

	block *mx = r;
	block *tmp = r->child;
	while( NULL != tmp ){
		block *fm = find_max_depth( tmp );
		if( NULL != fm && fm->nb->depth > mx->nb->depth )
			mx = fm;
		tmp = tmp->sibling;
	}

	return mx;

}


/*
 * Add the block to the main/incomplete chain
 * Return true if the parent hash is not in any of the main/incomplete chains
 */
bool Blockchain::add_received_block( uint32_t chain_id, BlockHash parent, BlockHash hash, network_block nb, bool &added )
{

	added = false;


	// If block is already in the chain, then do nothing
	if ( find_block_by_hash( this->chains[chain_id], hash ) != NULL ) return false;

	added = true;


	// If parent hash is already in the tree, then just add the child
	if ( find_block_by_hash( this->chains[chain_id], parent ) != NULL ){


		// Check if child hash is in incompletes
		block_incomplete *bi= this->is_incomplete_hash( this->inchains[chain_id], hash);

		if (bi != NULL){


			block *parent_block = find_block_by_hash( this->chains[chain_id], parent);
			block *child_block = bi->b;
			this->chains[chain_id] = insert_subtree_by_hash( this->chains[chain_id], child_block );

			add_subtree_to_received_non_full( child_block, chain_id  );


			child_block->parent = parent_block;
			block *tmp = parent_block->child;
			if (NULL == tmp)
				parent_block->child = child_block;
			else{
				while(tmp->sibling != NULL)
					tmp = tmp->sibling;
				tmp->sibling = child_block;
			}


			this->inchains[chain_id] = this->remove_one_chain( this->inchains[chain_id], bi );


		}
		else{
			// Just add the (parent, hash)
			add_block_by_parent_hash( &(this->chains[chain_id]), parent, hash);

			// Add to the non-full-blocks
			if ( received_non_full_blocks.find(hash) == received_non_full_blocks.end() && !have_full_block(chain_id, hash) ){
				received_non_full_blocks.insert( make_pair( hash, make_pair( chain_id, 0) ) );
				total_received_blocks ++;
			}

		}


		// Add full block info
		block *bz = find_block_by_hash( this->chains[chain_id], hash );
		if( NULL != bz ){
			bz->nb = new network_block(nb);
			added = true;

			//  Update deepest
			uint32_t old_depth = deepest[chain_id]->nb->depth;
			block *deep_last = find_max_depth( bz );
			if( deep_last->nb->depth > old_depth)
				deepest[ chain_id] = deep_last;

		}

	}

	// Else, need to add to incomplete chain and ask for more 
	else{

		if ( is_in_incomplete( this->inchains[chain_id], parent, hash) ) {
			added = false;
			return false;
		}

		// Add this to incomplete chain
    	this->inchains[chain_id] = add_block_to_incomplete( this->inchains[chain_id], parent, hash );


    	block *bz = find_incomplete_block( this->inchains[chain_id], hash );
		if( NULL != bz ){
			bz->nb = new network_block(nb);
		}



		// Ask for parent hash
		return true;

	}

	return false;
	
}





void Blockchain::specific_print_blockchain()
{

    uint32_t tot_max_depth = 0, tot_no_nodes = 0, tot_no_in_nodes = 0;
    for( int i=0; i<CHAINS; i++){
        tot_max_depth += get_deepest_child_by_chain_id(i)->nb->depth;
        tot_no_nodes += find_number_of_nodes(  this->chains[i]);
        tot_no_in_nodes += find_number_of_incomplete_blocks( this->inchains[i]); 
    }
    unsigned long time_of_finish = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
    float secs = (time_of_finish - time_of_start)/ 1000.0;

    tot_no_nodes -= CHAINS;


	printf("\n=============== [BLOCK HEADERS:]   Main tree:  %7d    Side trees: %7d  Total:  %7d\n", tot_no_nodes, tot_no_in_nodes, tot_no_nodes + tot_no_in_nodes );
	printf("\n=============== [MINING RATE  :]   %4d blocks / %.1f secs = %.2f  bps \n", tot_no_nodes+tot_no_in_nodes , secs, (float)(tot_no_nodes+tot_no_in_nodes)/secs );

	printf("\n=============== [BLOCKCHAIN   :]  Lengths:%7d   Mined:%5ld  Usefull:  %1.f %%  \n", 
												tot_max_depth, mined_blocks,   
												100.0*tot_max_depth / (tot_no_nodes+tot_no_in_nodes) );

	printf("\n=============== [LATENCY      :]  Receving(%lu)  : %0.2f secs     Partially ",receving_total, receiving_latency/1000.0/receving_total );
	for(int j=0; j<NO_T_DISCARDS; j++)
		printf("(%lu) ", partially_total[j]);
	printf(": ");
	for(int j=0; j<NO_T_DISCARDS; j++)
		printf("%0.1f secs ", partially_latency[j]/1000.0/partially_total[j]);
	printf("   Committing ");
	for(int j=0; j<NO_T_DISCARDS; j++)
		printf("(%lu) ", commited_total[j]);
	printf(": ");
	for(int j=0; j<NO_T_DISCARDS; j++)
		printf("%0.1f secs ", commited_latency[j]/1000.0/commited_total[j]);
	printf(" \n");



	printf("\n=============== [FULL BLOCKS  :]  Received:  %7ld / %7ld   Waiting:  %7ld   Processed:  %7ld ( %.0f %% ) \n", 
						total_received_blocks, received_non_full_blocks.size(), 
						waiting_for_full_blocks.size(), 
						processed_full_blocks, (total_received_blocks> 0) ? (100.0*processed_full_blocks/total_received_blocks): 0 );





    fflush(stdout);



}



/*
*
*
*	NEW
*
*/

block *Blockchain::find_block_by_hash_and_chain_id( BlockHash hash, uint32_t chain_id )
{
   return find_block_by_hash( this->chains[chain_id], hash );

}

block *Blockchain::find_incomplete_block_by_hash_and_chain_id(BlockHash hash, uint32_t chain_id)
{
	return find_incomplete_block( this->inchains[chain_id], hash );
}



block_incomplete *Blockchain::get_incomplete_chain( uint32_t chain_id)
{
	return this->inchains[chain_id];
}



block *Blockchain::get_deepest_child_by_chain_id( uint32_t chain_id) 
{
	if( NULL == deepest[ chain_id] ){
		printf("Something is wrong with get_deepest_child_by_chain_id\n"); fflush(stdout);
		printf("on [%d] it returns NULL\n", chain_id ); fflush(stdout);
		exit(5);
	}
	return deepest[ chain_id ];
}


bool Blockchain::have_full_block( uint32_t chain_id, BlockHash hash)
{
	block *bz = find_block_by_hash( this->chains[chain_id], hash );
	if( NULL != bz && bz->is_full_block) return true;
	return false;

}


bool Blockchain::still_waiting_for_full_block(  BlockHash hash, unsigned long time_of_now)
{
	if ( waiting_for_full_blocks.find( hash ) == waiting_for_full_blocks.end() ){
		waiting_for_full_blocks.insert( make_pair( hash, time_of_now ));
		return true;
	}
	return false;

}


bool Blockchain::add_block_by_parent_hash_and_chain_id( BlockHash parent_hash, BlockHash new_block, uint32_t chain_id, network_block nb)
{
	add_block_by_parent_hash( &(this->chains[chain_id]), parent_hash, new_block);

	block *bz = find_block_by_hash( this->chains[chain_id], new_block );
	if( NULL != bz ){
		this->deepest[chain_id] = bz;
		bz->nb = new network_block(nb);
	}

}


vector<BlockHash> Blockchain::get_incomplete_chain_hashes( uint32_t chain_id , unsigned long time_of_now )
{
	vector <BlockHash> hashes;
    block_incomplete *t = inchains[chain_id];
    while( NULL != t){
    	block_incomplete *nextt = t->next;
    	if ( time_of_now - t->last_asked  > ASK_FOR_INCOMPLETE_INDIVIDUAL_MILLISECONDS ){
    		t->last_asked = time_of_now;
    		t->no_asks ++;
    		if( t->no_asks > NO_ASKS_BEFORE_REMOVING)
    			this->inchains[chain_id] = this->remove_one_chain( this->inchains[chain_id], t );
    		else
    			hashes.push_back( t->b->hash);
      	}
	    t = nextt;
    }

    return hashes;
}

vector< pair <BlockHash, uint32_t> > Blockchain::get_non_full_blocks( unsigned long time_of_now )
{
	vector< pair <BlockHash, uint32_t> > nfb;
	vector <BlockHash> to_remove;
	
	for( auto it=received_non_full_blocks.begin(); it != received_non_full_blocks.end(); it++ )
		if( time_of_now - it->second.second > ASK_FOR_FULL_BLOCKS_INDIVIDUAL_EACH_MILLISECONDS){
			
			it->second = make_pair ( it->second.first, time_of_now ) ;
			block *bz = find_block_by_hash( this->chains[it->second.first], it->first );


			if( NULL != bz && ! (bz->is_full_block) )
				nfb.push_back( make_pair(it->first, it->second.first));
			else if ( NULL != bz && bz->is_full_block )
				to_remove.push_back( it->first );

			if( nfb.size() >= MAX_ASK_NON_FULL_IN_ONE_GO ) break;

		}


	for( int i=0; i<to_remove.size(); i++)
		if( received_non_full_blocks.find(  to_remove[i]) != received_non_full_blocks.end() )
			received_non_full_blocks.erase( to_remove[i] );
		
	return nfb;
}



void Blockchain::remove_waiting_blocks( unsigned long time_of_now )
{
	
	vector <BlockHash> to_remove;

	for( auto it=waiting_for_full_blocks.begin(); it != waiting_for_full_blocks.end(); it++ ){
		if( time_of_now - it->second > MAX_WAIT_FOR_FULL_BLOCK_MILLSECONDS)
			//waiting_for_full_blocks.erase( (it++)->first );
			to_remove.push_back( it->first );
	}

	for( int i=0; i<to_remove.size(); i++)
		if( waiting_for_full_blocks.find(to_remove[i]) != waiting_for_full_blocks.end() )
			waiting_for_full_blocks.erase( to_remove[i] );

}




void Blockchain::set_block_full( uint32_t chain_id, BlockHash hash, string misc )
{
	if( received_non_full_blocks.find(hash) != received_non_full_blocks.end() )
		received_non_full_blocks.erase(hash);
	if( waiting_for_full_blocks.find(hash) != waiting_for_full_blocks.end() )
		waiting_for_full_blocks.erase(hash);

	block *bz = find_block_by_hash( this->chains[chain_id], hash );
	if (NULL != bz){
		bz->is_full_block = true;
		processed_full_blocks ++ ;

        // Define time_received
        if ( bz -> nb != NULL){
		    unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
		    if( time_of_now > bz->nb->time_mined ){ 	
		    	bz->nb->time_received = time_of_now;
			    receving_total ++;
			    receiving_latency += bz->nb->time_received - bz->nb->time_mined;
		    }
		    else 									
		    	bz->nb->time_received = bz->nb->time_mined;


		    /*
			if ( STORE_BLOCKS && (hash % BLOCKS_STORE_FREQUENCY) == 0  ){
				string filename =  string(FOLDER_BLOCKS)+"/"+my_ip+"-"+to_string(my_port);
	            ofstream file;
	            file.open(filename, std::ios_base::app); 
	            file << hex << hash << dec << " " << (bz->nb->time_received - bz->nb->time_mined) << " " << misc << endl;
	            file.close();
	        }
	        */
	        if ( STORE_BLOCKS && (hash % BLOCKS_STORE_FREQUENCY) == 0  ){
				string filename =  string(FOLDER_BLOCKS)+"/"+my_ip+"-"+to_string(my_port);
	            ofstream file;
	            file.open(filename, std::ios_base::app); 
	            file << "0 " << hex << hash << dec << " " << (bz->nb->time_received - bz->nb->time_mined) << endl;
	            file.close();
	        }

		}

	}
}

void Blockchain::add_mined_block()
{
	mined_blocks++;
}




void Blockchain::update_blocks_commited_time()
{

	unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);


	for( int j=0; j<NO_T_DISCARDS;j ++){


			/*
			 * Update partial times
			 */
			for( int i=0; i<CHAINS; i++){

				// Discard the last 
				block *t = deepest[i];
				int count = 0;
				while( NULL != t && count++ < T_DISCARD[j] )
					t = t->parent;
				if (NULL == t) continue;

				while ( NULL != t  ){
					if (  t->is_full_block &&  NULL != t->nb && 0 == t->nb->time_partial[j]   &&  time_of_now > t->nb->time_mined ){
						t->nb->time_partial[j] = time_of_now;
						partially_total[j] ++;
						partially_latency[j] += t->nb->time_partial[j] - t->nb->time_mined;

				        if ( STORE_BLOCKS && (t->hash % BLOCKS_STORE_FREQUENCY) == 0  ){
							string filename =  string(FOLDER_BLOCKS)+"/"+my_ip+"-"+to_string(my_port);
				            ofstream file;
				            file.open(filename, std::ios_base::app); 
				            file << "1 " << hex << t->hash << dec << " " << (t->nb->time_partial[j] - t->nb->time_mined) << " " << j << endl;
				            file.close();
				        }


					}
					t = t->parent;
				}
			}


			/*
			 * Full commit times
			 */


			// Find the minimal next_rank
			bool stop_this_j = false;
			uint32_t confirm_bar = -1;
			for( int i=0; i<CHAINS; i++){

				// Discard the last 
				block *t = deepest[i];
				int count = 0;
				while( NULL != t && count++ < T_DISCARD[j] )
					t = t->parent;
				if (NULL == t){
					stop_this_j = true;
					break;
					//return;
				}
				
				if ( t->nb == NULL ){
					stop_this_j = true;
					break;
					//return;
				}

				if( stop_this_j ) break;

				if ( t->nb->next_rank < confirm_bar ) confirm_bar = t->nb->next_rank;
			}

			if( stop_this_j) continue;


			if ( confirm_bar < 0) continue;


			// Update commited times
			for( int i=0; i<CHAINS; i++){

				// Discard the last 
				block *t = deepest[i];
				int count = 0;
				while( NULL != t && count++ < T_DISCARD[j] )
					t = t->parent;
				if (NULL == t) continue;

				while ( NULL != t  ){
					if (  t->is_full_block &&  NULL != t->nb && t->nb->next_rank < confirm_bar &&  0 == t->nb->time_commited[j]   &&  time_of_now > t->nb->time_mined ){
						t->nb->time_commited[j] = time_of_now;
						commited_total[j] ++;
						commited_latency[j] += t->nb->time_commited[j] - t->nb->time_mined;
						
						if ( STORE_BLOCKS && (t->hash % BLOCKS_STORE_FREQUENCY) == 0  ){
							string filename =  string(FOLDER_BLOCKS)+"/"+my_ip+"-"+to_string(my_port);
				            ofstream file;
				            file.open(filename, std::ios_base::app); 
				            file << "2 " << hex << t->hash << dec << " " << (t->nb->time_commited[j] - t->nb->time_mined) << " " << j <<  endl;
				            file.close();
				        }
					}
					t = t->parent;
				}

			}

	}

}