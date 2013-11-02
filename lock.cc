#include "lock.h"


CLock::CLock() 
{
	pthread_mutex_init(&m_lock, NULL);
}
      
CLock::~CLock() 
{
	pthread_mutex_destroy(&m_lock); 
}
      
void CLock::Lock(void) 
{
	pthread_mutex_lock(&m_lock);
}
      
void CLock::UnLock(void) 
{
	pthread_mutex_unlock(&m_lock);
}


