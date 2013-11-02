#ifndef __DATATHREAD_H
#define __DATATHREAD_H

#include "global.h"
#include "lock.h"

class CControlThread;


class CDataThread
{
	public:
	
	CDataThread(int,int, int, int, int, int, int, string, string, string, string, int, int, int,CControlThread *,string);
	~CDataThread();
	
	pthread_t tid; // thread id
	
	int getQuit(void);
	void setQuit(int);
		
	void closeconnection();
	
	private:
	
	CLock quitlock;	
	int shouldquit;
	friend void *makedatathread(void *pData);
	
	void dataloop(void);

	string fp; // store fingerprint from datachannel
	
	int Write(int ,char *,int ,SSL *);
	int Read(int  ,char *,int &,SSL *);
	
	SSL *sitessl,*clientssl;
	SSL_CTX *sitesslctx,*tmpctx;
	
	int datalisten_sock,datasite_sock,dataclient_sock;

	
	string activeip,username,clientip,passiveip,passivecmd;
	int activeport,passiveport,newport;
	int transfertype,sslprotp,cpsvcmd,sscncmd;
	int relinked, usingssl, activecon;
	char *buffer;
	CControlThread *controlthread;
};




#endif
