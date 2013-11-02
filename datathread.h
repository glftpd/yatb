#ifndef __DATATHREAD_H
#define __DATATHREAD_H

#include "global.h"

class CControlThread;


class CDataThread
{
	public:
	
	CDataThread(int, int, int, int, int, int, string, struct sockaddr_in,CControlThread *);
	~CDataThread();
	
	pthread_t tid; // thread id
	
	void getactive_data(string);
	string getpassive_data(string);
	
	private:
	
	friend void *makedatathread(void *pData);
	
	void dataloop(void);
	void closeconnection();
	
	int data_write(int,char*,int);
	int data_read(int,char *,int &);
	
	int datalisten_sock,datasite_sock,dataclient_sock;

	struct sockaddr_in client_addr;
	string activeip,username;
	int activeport,passiveport,siteport;
	int transfertype,sslprotp,cpsvcmd;
	int relinked, usingssl, activecon;
	char *buffer;
	CControlThread *controlthread;
};




#endif
