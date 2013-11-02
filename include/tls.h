#ifndef __TLS_H
#define __TLS_H

#include "global.h"

int THREAD_setup(void);
int THREAD_cleanup(void);

#define MUTEX_TYPE	pthread_mutex_t
#define MUTEX_SETUP(x)	pthread_mutex_init(&(x),NULL)
#define MUTEX_CLEANUP(x)	pthread_mutex_destroy(&(x))
#define MUTEX_LOCK(x)	pthread_mutex_lock(&(x))
#define MUTEX_UNLOCK(x)	pthread_mutex_unlock(&(x))
#define THREAD_ID	pthread_self()



extern SSL_CTX *clientsslctx, *connectsslctx;
extern string cert_bk;

struct CRYPTO_dynlock_value
{
	MUTEX_TYPE mutex;
};


int decrypt_cert(string);
int kill_file(string);

int ssl_setup(void);

#endif
