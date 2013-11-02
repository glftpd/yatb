#ifndef __CONTROLTHREAD_H
#define __CONTROLTHREAD_H


#include "global.h"
#include "counter.h"
#include "lock.h"
#include "iplist.h"

class CDataThread;


class CControlThread
{
	public:

	pthread_t tid; // thread id


	CControlThread(int,string,int,string,int);
	~CControlThread();
	int DirectionSet(void); //returns 1 if transfer direction is set
	void SetDirection(int); 
	

	private:

	friend void *makethread(void *pData);
	friend class CDataThread;
	
	CDataThread *datathread;

	void deletedatathread(void);
	
	void mainloop(void);
		
	int Read(int ,SSL *,string &);
	int Write(int ,string ,SSL *);
	
	int trytls(void);

	int tryrelink(int);
	int directionset;
	string direction;
	string CreatePsvCommand(int);
	
	CCounter localcounter;
	CLock rwlock,directionlock;
	
	int client_sock,site_sock; //connection sockets
	int datalisten_sock,datasite_sock,dataclient_sock; // data connections sockets
	int gotfirstreply; // got the first site reply
	int gotfirstcmd; // got first user command
	int gotusercmd;
	int gotpasscmd;
	int gotwelcomemsg;
	int gotpasvcmd;
	int gotportcmd;
	int sscn;
	int cpsvcmd; // ssl fxp
	int sslprotp; // using secure dirlisting
	int activecon;
	int relinked; // conenction is relinked?
	int usingssl; // using a ssl connection?
	int transfertype; // ASCII or BINARY (1/2)
	time_t connect_time;
	
	SSL *sitessl,*clientssl;
	SSL_CTX *sitesslctx;
	
	int clientsslbits,sitesslbits;
	
	string passivecmd,portcmd,idndtcmd;
	
	string username;

	int shouldquit;
	
	int using_entry;
	CLock writelock;
	string clientip;
	int clientport;
	string site_ip;
	int site_port;
};

extern list<CControlThread*> conlist;

extern int listen_sock;

extern long int nr_logins;

extern time_t start_time;

extern string bk;

extern string conffile;

extern string yatbfilename;

extern CIplist iplist;

#endif

