#ifndef __LOCK_H
#define __LOCK_H

#include "global.h"


class CLock
{
	private:
	
	pthread_mutex_t m_lock;
	
	public:
	
	CLock();
	
	~CLock();
	
	void Lock(void);
	
	void UnLock(void);	

};

extern CLock list_lock,config_lock,globals_lock,sock_lock;


#endif
