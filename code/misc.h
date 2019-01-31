#ifndef MISC_H
#define MISC_H

#include <iostream>
#include <vector>

using namespace std;


vector<string> split(const string& str, const string& delim);
int safe_stoi( string s, bool &pr);
unsigned long safe_stoull( string s, bool &pr);




#endif