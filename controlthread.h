#ifndef __CONTROLTHREAD_H
#define __CONTROLTHREAD_H


#include "global.h"
#include "counter.h"
#include "lock.h"

class CDataThread;


class CControlThread
{
	public:

	pthread_t tid; // thread id


	CControlThread(int,struct sockaddr_in);
	~CControlThread();
	
	

	private:

	friend void *makethread(void *pData);
	friend class CDataThread;
	
	CDataThread *datathread;

	void deletedatathread(void);
	struct sockaddr_in client_addr,site_addr;
	void mainloop(void);

	
	int control_read(int,SSL*,string&);
	int control_write(int,string,SSL*);
	
	int trytls(void);

	int tryrelink(int);

	
	CCounter localcounter;
	
	int client_sock,site_sock; //connection sockets
	int datalisten_sock,datasite_sock,dataclient_sock; // data connections sockets
	int gotfirstreply; // got the first site reply
	int gotfirstcmd; // got first user command
	int gotusercmd;
	int gotpasscmd;
	int gotwelcomemsg;
	int gotpasvcmd;
	int gotportcmd;
	
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
	
	string passivecmd,portcmd;
	
	string username;

	int shouldquit;

	int using_entry;
	CLock writelock;
	string client_ip;
};

extern list<CControlThread*> conlist;

extern int listen_sock;

extern int nr_logins,nr_threads;

extern time_t start_time;

extern string bk;

extern string conffile;

#endif

