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
	void reset(void);
	
	double getsend(void);
	double getrecvd(void);
	double gettotal(void);
	
	private:
	
	double send,recvd;
	
	CLock lock;
	
};

extern CCounter totalcounter,daycounter,weekcounter,monthcounter;

#endif
