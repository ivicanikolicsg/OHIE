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


#include "crypto_stuff.h"

using namespace std;

EC_KEY* ecKey;

string dummy_signature = string(DUMMY_SIGNATURE);

string sha256(const string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}


EC_KEY* get_private_key_from_file(string filename)
{

    ifstream file(filename);
    if (!file.is_open()){
        cout << "Cannot open file with private key :"<<filename<<endl;
        cout << "Exiting...";
        exit(1);
    }
    stringstream strStream;
    strStream << file.rdbuf();//read the file
    string strpkey = strStream.str();
    file.close();


    EC_KEY *eckey = NULL;
    EC_POINT *pub_key = NULL;
    const EC_GROUP *group = NULL;
    BIGNUM *start;
    BIGNUM *res;
    BN_CTX *ctx;

    start = BN_new();
    ctx = BN_CTX_new(); // ctx is an optional buffer to save time from allocating and deallocating memory whenever required

    res = start;
    BN_hex2bn(&res, strpkey.c_str());
    eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    group = EC_KEY_get0_group(eckey);
    pub_key = EC_POINT_new(group);

    EC_KEY_set_private_key(eckey, res);

    /* pub_key is a new uninitialized `EC_POINT*`.  priv_key res is a `BIGNUM*`. */
    if (!EC_POINT_mul(group, pub_key, res, NULL, NULL, ctx))
    printf("Error at EC_POINT_mul.\n");


    EC_KEY_set_public_key(eckey, pub_key);
    BN_free(start);

    return eckey;
}


void prepare_ecc_crypto( string filename_key )
{
    ecKey = get_private_key_from_file( filename_key );
}

string sign_message( string message )
{

    if( !SIGN_TRANSACTIONS || !VERIFY_TRANSACTIONS ) return dummy_signature;
//    if (!VERIFY_TRANSACTIONS) return  dummy_signature ;  


    string h = sha256( message );

    unsigned char *sig;
    unsigned int siglen = ECDSA_size(ecKey);

    sig  = (unsigned char *)OPENSSL_malloc(siglen);
    int es = ECDSA_sign(0, reinterpret_cast<const unsigned char *>(h.c_str()), h.size(), sig, &siglen, ecKey);
    stringstream ss;
    for(int i=0; i<siglen; ++i){
        ss << setw(2) << setfill('0') << std::hex << (int)sig[i];
    }

    return ss.str();
}


bool verify_message( string message, string signature)
{


    // If no need to verify transactions, then just return true
    if (!VERIFY_TRANSACTIONS) return signature.size() == dummy_signature.size() ;  

    int siglen = signature.size()/2;
    unsigned char *sig = (unsigned char *)malloc( siglen + 1);

    for(int i=0; i<siglen; i++)
        sig[i] = strtol( signature.substr(2*i,2).c_str(), NULL, 16 );
    sig[siglen] = '\0';

    string h = sha256( message );

    return (1==ECDSA_verify(0, reinterpret_cast<const unsigned char *>(h.c_str()), h.size(), sig, siglen, ecKey));

}
