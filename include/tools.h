#ifndef __TOOLS_H
#define __TOOLS_H

#include "global.h"

void debugmsg(string ,string ,int err=0);
void cmddebugmsg(string ,string );

string upper(string str,int length);

int random_range(int lowest_number, int highest_number);

string ltrim( const string &str, const string &whitespace = "\t ");
string rtrim( const string &str, const string &whitespace = "\t ");
string trim( const string &str, const string &whitespace = "\t ");
string crcut(string );

int setnonblocking(int);
int setblocking(int);

void correctReply(string &);

int Connect(int &,string,int,int,int &);
int Accept(int ,int &,string &,int &,int, int &);
int Bind(int &,string,int);
int SocketOption(int &,int);
int IsNumeric(char);
int FtpCode(string ,int &);
int IsEndline(string);
int Ident(string,int,string,string &);
int control_read(int ,SSL *,string &);
int control_write(int ,string ,SSL *);
struct sockaddr_in GetIp(string,int);
int Ident(string,int,int, string, string &,int);
int SslConnect(int &,SSL **,SSL_CTX **);
int SslAccept(int &,SSL **,SSL_CTX **);

int ParsePortCommand(string,string &,int &);
int ParsePsvCommand(string ,string &, int &);

int DataWrite(int ,char *,int ,SSL *);
int DataRead(int  ,char *,int &,SSL *,int,int);
int Close(int &,string);
int GetSock(int &);
void PrintSock(int ,string );

string traffic2str(double);

int printsockopt(int, string );

int filesize(string ,int &);
int readfile(string ,unsigned char **,int );
int writefile(string ,unsigned char *,int );
int decrypt(string ,unsigned char *,unsigned char *,int );
int encrypt(string ,unsigned char *,unsigned char *,int );
int GetLine(int,SSL **,string &);
int Login(int &,string ,int ,string ,string ,int ,SSL **,SSL_CTX **,string &);

int trafficcheck(void);


#if defined(__linux__) && defined(__i386__)
pid_t gettid(void);
#endif

struct sockaddr_in GetIp(string,int);

#endif
