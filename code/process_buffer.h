#ifndef PROCESS_BUFFER_H
#define PROCESS_BUFFER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>


#include "MyServer.hpp"
#include "Blockchain.hpp"
#include "requests.h"
#include "crypto_stuff.h"
#include "misc.h"

using namespace std;

void process_buffer( string &m, tcp_server *ser, Blockchain *bc );



#endif