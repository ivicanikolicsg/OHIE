#ifndef GET_IP_H
#define GET_IP_H

#include <iostream>     ///< cout
#include <cstring>      ///< memset
#include <errno.h>      ///< errno
#include <sys/socket.h> ///< socket
#include <netinet/in.h> ///< sockaddr_in
#include <arpa/inet.h>  ///< getsockname
#include <unistd.h>     ///< close

#include <iostream>

using namespace std;

string get_my_local_ip();


#endif