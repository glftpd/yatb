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

void CCounter::reset(void)
{
	lock.Lock();
	recvd = 0;
	send = 0;
	lock.UnLock();
}

double CCounter::getsend(void)
{
	double tmp;
	lock.Lock();
	tmp = send;
	lock.UnLock();
	return tmp;
}

double CCounter::getrecvd(void)
{
	double tmp;
	lock.Lock();
	tmp = recvd;
	lock.UnLock();
	return tmp;
}

double CCounter::gettotal(void)
{
	double tmp;
	lock.Lock();
	tmp = recvd + send;
	lock.UnLock();
	return tmp;
}
