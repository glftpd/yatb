#ifndef __DATATHREAD_H
#define __DATATHREAD_H

#include "global.h"

class CControlThread;


class CDataThread
{
	public:
	
	CDataThread(int, int, int, int, int, int, string, string, string, string, int, int, int,CControlThread *,string);
	~CDataThread();
	
	pthread_t tid; // thread id
	
	
	
	void closeconnection();
	
	int shouldquit;	
	
	private:
	
	friend void *makedatathread(void *pData);
	
	void dataloop(void);
	
	
	SSL *sitessl,*clientssl;
	SSL_CTX *sitesslctx;
	
	int datalisten_sock,datasite_sock,dataclient_sock;

	
	string activeip,username,clientip,passiveip,passivecmd;
	int activeport,passiveport,newport;
	int transfertype,sslprotp,cpsvcmd;
	int relinked, usingssl, activecon;
	char *buffer;
	CControlThread *controlthread;
};




#endif
