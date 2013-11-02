#include "counter.h"


CCounter::CCounter()
{
	send = 0;
	recvd = 0;
}

CCounter::~CCounter()
{
}

void CCounter::addsend(double d)
{
	lock.Lock();
	send += d;
	lock.UnLock();
}

void CCounter::addrecvd(double d)
{
	lock.Lock();
	recvd += d;
	lock.UnLock();
}

double CCounter::getsend(void)
{
	return send;
}

double CCounter::getrecvd(void)
{
	return recvd;
}
