#ifndef __FORWARD_H
#define __FORWARD_H

#include "global.h"

class CForward
{
	public:
	
	CForward(int,int,string);
	~CForward();
	pthread_t tid; // thread id
	
	friend void *makeforwardthread(void *pData);
	
	private:
	
	int listen_sock;
	void mainloop(void);
	int sport,dport;
	string fw_ip;
	
};

#endif
