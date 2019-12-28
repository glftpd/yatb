#ifndef __CONTROLTHREAD_H
#define __CONTROLTHREAD_H


#include "global.h"
#include "counter.h"
#include "lock.h"
#include "iplist.h"

class CDataThread;

void *makethread(void* pData);

class CControlThread
{
	public:

	pthread_t tid; // thread id


	CControlThread(int,string,int,string,int);
	~CControlThread();
	int DirectionSet(void); //returns 1 if transfer direction is set
	void SetDirection(int); 
	

	private:
	pthread_attr_t threadattr;
	friend void *makethread(void *pData);
	friend class CDataThread;
	
	CDataThread *datathread;

	int deletedatathread(void);
	
	void mainloop(void);
		
	int Read(int ,SSL *,string &);
	int TryRead(int ,SSL *,string &);
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
	int gotauthsslmsg;
	int sendadminmsg; // store if a admin msg was send
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
	
	string _site_ip; // to store site ip for security check in Write()

	int using_entry;
	CLock writelock;
	string clientip;
	int clientport;
	string site_ip;
	string dirlist_ip; // ip used for (first) dirlisting
	int site_port;
	int dirlisting; // transfer is a dirlisting
	string admin_msg; // msg for admin from datathread (fpwl)
};

extern list<CControlThread*> conlist;

extern int listen_sock,use_blowconf;

extern long int nr_logins;

extern time_t start_time;

extern string bk,ip_bk;

extern string conffile;

extern string yatbfilename;

extern CIplist iplist;

#endif

