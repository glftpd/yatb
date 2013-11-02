#ifndef __COUNTER_H
#define __COUNTER_H

#include "global.h"
#include "lock.h"


class CCounter
{
	public:
	
	CCounter();
	~CCounter();
	
	void addsend(double);
	void addrecvd(double);
	
	double getsend(void);
	double getrecvd(void);
	
	private:
	
	double send,recvd;
	
	CLock lock;
	
};

extern CCounter totalcounter;

#endif
