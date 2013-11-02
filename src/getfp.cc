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

SSL *datassl = NULL;
SSL_CTX *sslctx = NULL;


int sock = -1, dataclient_sock = -1;
string ip,user,pass,message,activeip;
int port,activeport;
int usessl;

void Check()
{
	if(Login(sock,ip,port,user,pass,usessl,&clientssl,&clientsslctx,message))
	{		
		cout << "Control channel Fingerprint: " << fingerprint(clientssl) << "\n";		
		cout << "Cipher: " << SSL_get_cipher(clientssl) << "\n";
		string tmp,pasv;
		control_write(sock,"TYPE A\r\n",clientssl);
		GetLine(sock,&clientssl,tmp);
		int shouldquit = 0;
		//cout << tmp;
		control_write(sock,"PROT P\r\n",clientssl);
		GetLine(sock,&clientssl,tmp);

		//control_write(sock,"SSCN ON\r\n",clientssl);
		//GetLine(sock,&clientssl,tmp);
		//cout << tmp;
		control_write(sock,"PASV\r\n",clientssl);
		GetLine(sock,&clientssl,pasv);
		//cout << pasv;

		int port;
		ParsePsvCommand(pasv,ip,port);
				
		
		if (!GetSock(dataclient_sock))
		{
			cout << "Error getting socket\n";
			return;
		}		
		if(!Connect(dataclient_sock,ip,port,5,shouldquit))
		{				
			cout << "Can't connect to data port\n";
			cout << "ip: " << ip << " - port: " << port << "\n";
			return;
		}

		control_write(sock,"LIST -al\r\n",clientssl);
		GetLine(sock,&clientssl,tmp);
		//cout << tmp;
		
		
		if(!SslConnect(dataclient_sock,&datassl,&sslctx,shouldquit,config.data_cipher))
		{
			cout << "ssl connect failed\n";
			return;
		}
		cout << "Data channel Fingerprint (pasv): " << fingerprint(datassl) << "\n";
		cout << "Cipher: " << SSL_get_cipher(datassl) << "\n";
		GetLine(sock,&clientssl,tmp);
	}
	else
	{
		cout << message << "\n";
	}
}



int main(int argc,char *argv[])
{
	
	
	if(argc != 5 )
	{
		cout << "usage: getfp ip port user pass\n";
		cout << "example: getfp 192.168.1.1 20000 test test2\n";
		return 0;
	}
	cout << "GetFP v0.3 (c) _hawk_\n";	
	SSL_load_error_strings();
	SSL_library_init();
		
	ip = argv[1];
	port = atoi(argv[2]);
	user = argv[3];
	pass = argv[4];
	

	if(!GetSock(sock))
	{
		cout << "Socket error\n" ;
		return 0;
	}
	usessl = 1;
	//config.control_cipher = "ALL:!ADH:!EXP";
	//config.data_cipher = "ALL:!ADH:!EXP";
	//config.debug = 1;
	//config.log_to_screen =1;
	Check();
	
	if (clientssl != NULL) 
	{		
		SSL_shutdown(clientssl); 	
	}

	if (datassl != NULL) 
	{		
		SSL_shutdown(datassl); 	
	}
	
	Close(dataclient_sock,"sock");
	Close(sock,"sock");
			
	if (datassl != NULL) 
	{ 		
		SSL_free(datassl); 
		datassl = NULL; 
	}

	if (clientssl != NULL) 
	{ 		
		SSL_free(clientssl); 
		datassl = NULL; 
	}
	
	
}

