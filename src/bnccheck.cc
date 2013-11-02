#include "global.h"

#include "tools.h"
#include "config.h"
#include "lock.h"
#include "counter.h"
#include <sys/time.h>

// some dummy objects to use tools.h
CConfig config;
CLock sock_lock,config_lock;
CCounter monthcounter,weekcounter,daycounter;
int use_blowconf;


SSL_CTX  *clientsslctx = NULL;
SSL *clientssl = NULL;
int sock = -1;

string red = "4";
string colorend = "";
string green = "3";

struct timeval start_time,end_time;



int main(int argc,char *argv[])
{
	
	int usessl,usecolor;
	if(argc < 5 || argc > 7)
	{
		cout << "usage: bnccheck ip port user pass [ssl] [color]\n";
		cout << " ssl and color are optional and default is ssl=1 (on), color=0 (off)\n";
		return 0;
	}
	else if(argc == 5)
	{
		usessl = 1;
		usecolor = 0;
	}
	else if(argc == 6)
	{
		usessl = atoi(argv[5]);
		usecolor = 0;
	}
	else if(argc == 7)
	{
		usessl = atoi(argv[5]);
		usecolor = atoi(argv[6]);
	}
	
	if(usecolor == 0)
	{
		red = "";
		colorend = "";
		green = "";
	}
	
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	
	string ip = argv[1];
	int port = atoi(argv[2]);
	string user = argv[3];
	string pass = argv[4];
	if(!GetSock(sock))
	{
		cout << red << "Socket error" << colorend;
		return 0;
	}
	gettimeofday(&start_time,NULL);
	string message;
	if(Login(sock,ip,port,user,pass,usessl,&clientssl,&clientsslctx,message))
	{
		cout << green << message << colorend;
	}
	else
	{
		cout << red << message << colorend;
	}
	gettimeofday(&end_time,NULL);
	cout << " - " << (end_time.tv_sec - start_time.tv_sec) << "." << abs((end_time.tv_usec - start_time.tv_usec)) / 1000 << "s\n";
	if (clientssl != NULL) 
	{		
		SSL_shutdown(clientssl); 	
	}
	
	Close(sock,"sock");
	
	if (clientssl != NULL) 
	{ 		
		SSL_free(clientssl); 
		clientssl = NULL; 
	}
		
	
	
}

