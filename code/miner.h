#ifndef MINER_H
#define MINER_H

#include "Blockchain.hpp"

uint32_t mine_new_block( Blockchain *bc);
void miner( Blockchain *bc );

#endif
