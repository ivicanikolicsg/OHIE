#ifndef CRYPTO_STUFF_H
#define CRYPTO_STUFF_H


#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/pem.h>

#include "params.h"

using namespace std;


string sha256(const string str);

void prepare_ecc_crypto( string filename_key_pem );

string sign_message( string message );
bool verify_message( string message, string signature);





#endif