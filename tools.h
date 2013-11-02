#ifndef __TOOLS_H
#define __TOOLS_H

#include "global.h"

void debugmsg(string ,string ,int err=0);

string upper(string str,int length);

int random_range(int lowest_number, int highest_number);

string ltrim( const string &str, const string &whitespace = "\t ");
string rtrim( const string &str, const string &whitespace = "\t ");
string trim( const string &str, const string &whitespace = "\t ");

int setnonblocking(int);
int setblocking(int);

void correctReply(string &);

int Connect(int &,struct sockaddr_in &,int,int &);
int Accept(int &,int &,struct sockaddr_in &,int, int &);

int IsEndline(string);

string traffic2str(double);

struct sockaddr_in GetIp(string,int);

#endif
