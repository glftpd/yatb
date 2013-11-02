#include "tls.h"
#include "tools.h"
#include "config.h"

static MUTEX_TYPE *mutex_buf = NULL;

static DH *globaldh = NULL;

static void locking_function(int mode, int n, const char * file, int line)
{
	stringstream ss;
	ss << mode << n << file << line;
	if (mode & CRYPTO_LOCK)
	{
		MUTEX_LOCK(mutex_buf[n]);
	}
	else
	{
		MUTEX_UNLOCK(mutex_buf[n]);
	}
}

static unsigned long id_function(void)
{
	return ((unsigned long)THREAD_ID);
}

static struct CRYPTO_dynlock_value * dyn_create_function(const char *file, int line)
{
	stringstream ss;
	ss << file << line;
	struct CRYPTO_dynlock_value *value;
	value = (struct CRYPTO_dynlock_value *)malloc(sizeof(struct CRYPTO_dynlock_value));
	if (!value)
	{
		return NULL;
	}
	MUTEX_SETUP(value->mutex);
	return value;
}

static void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l, const char *file, int line)
{
	stringstream ss;
	ss << mode << file << line;
	if (mode & CRYPTO_LOCK)
	{
		MUTEX_LOCK(l->mutex);
	}
	else
	{
		MUTEX_UNLOCK(l->mutex);
	}
}

static void dyn_destroy_function(struct CRYPTO_dynlock_value *l, const char *file, int line)
{
	stringstream ss;
	ss << file << line;
	MUTEX_CLEANUP(l->mutex);
	free(l);
}


int THREAD_setup(void)
{
	int i;
	
	mutex_buf = (MUTEX_TYPE *)malloc(CRYPTO_num_locks() * sizeof(MUTEX_TYPE));
	if (!mutex_buf)
	{
		return 0;
	}
	for (i = 0; i < CRYPTO_num_locks(); i++)
	{
		MUTEX_SETUP(mutex_buf[i]);
	}
	CRYPTO_set_id_callback(id_function);
	CRYPTO_set_locking_callback(locking_function);
	
	CRYPTO_set_dynlock_create_callback(dyn_create_function);
	CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
	CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
	
	return 1;
}

int THREAD_cleanup(void)
{
	int i;
	
	if (!mutex_buf)
	{
		return 0;
	}
	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);
	CRYPTO_set_dynlock_create_callback(NULL);
	CRYPTO_set_dynlock_lock_callback(NULL);
	CRYPTO_set_dynlock_destroy_callback(NULL);
	for (i = 0; i < CRYPTO_num_locks(); i++)
	{
		MUTEX_CLEANUP(mutex_buf[i]);
	}
	free(mutex_buf);
	mutex_buf = NULL;
	return 1;
}










DH *tmp_dh_cb(SSL *ssl, int is_export, int keylength)
{
	stringstream ss;
	ss << is_export << keylength << ssl;
	
	return globaldh;
	
}

int decrypt_cert(string tmpcert)
{
	int size;
	if (!filesize(config.cert_path,size))
	{
		debugmsg("-SYSTEM-", "cert decrypt error");
		return 0;
	}
	
	unsigned char *in,*out;
	in = new unsigned char [size];
	out = new unsigned char [size];
	in = readfile(config.cert_path,size);
	if(!decrypt(cert_bk,in,out,size))
	{
		debugmsg("-SYSTEM-", "cert decrypt error");
		memset(in, 0,size);
		memset(out, 0,size);
		delete in;
		delete out;
		return 0;
	}
	if (!writefile(tmpcert,out,size))
	{
		memset(in, 0,size);
		memset(out, 0,size);
		delete in;
		delete out;
		debugmsg("-SYSTEM-", "cert decrypt error");
		return 0;
	}
	memset(in, 0,size);
	memset(out, 0,size);
	delete in;
	delete out;
	return 1;
}


int kill_cert(string tmpcert)
{
    int size;
	if (!filesize(tmpcert,size))
	{
		debugmsg("-SYSTEM-", "error getting filesize");
		return 0;
	}
    unsigned char *out;
    out = new unsigned char [size];
    memset(out, 0,size);
    if (!writefile(tmpcert,out,size))
	{		
		delete out;
		debugmsg("-SYSTEM-", "cert write error");
		return 0;
	}
    memset(out, 1,size);
    if (!writefile(tmpcert,out,size))
	{		
		delete out;
		debugmsg("-SYSTEM-", "cert write error");
		return 0;
	}
    memset(out, 2,size);
    if (!writefile(tmpcert,out,size))
	{		
		delete out;
		debugmsg("-SYSTEM-", "cert write error");
		return 0;
	}
    delete out;
    string command = "rm " + tmpcert;
    system(command.c_str());
    return 1;
}


int ssl_setup()
{
	clientsslctx = NULL;
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	if (RAND_status()) { debugmsg("-SYSTEM-","RAND_status ok"); }
	else { cout << "RAND_status not ok\n"; return 0; }
	clientsslctx = 	SSL_CTX_new(SSLv23_server_method());
	if (clientsslctx == NULL)
	{
		debugmsg("-SYSTEM-", "error creating ctx");
		if(config.syslog)
		{
			syslog(LOG_ERR, "error creating ctx");
		}
		return 0;
	}

  
	SSL_CTX_set_default_verify_paths(clientsslctx);
	SSL_CTX_set_options(clientsslctx,SSL_OP_ALL);
	SSL_CTX_set_mode(clientsslctx,SSL_MODE_AUTO_RETRY);
	
	string certfile = "certtmp";
	
	if(!config.crypted_cert)
	{
		certfile = config.cert_path;
	}
	else
	{
		if(!decrypt_cert(certfile))
		{
			debugmsg("-SYSTEM-", "cert decrypt error");
			return 0;
		}
	}
	
	if (SSL_CTX_use_certificate_file(clientsslctx,certfile.c_str(),SSL_FILETYPE_PEM) <= 0)
	{
		if(config.syslog)
		{
			syslog(LOG_ERR, "error loading cert file!");
		}
		debugmsg("-SYSTEM-", "error loading cert file!");
		return 0;
	}
	if (SSL_CTX_use_PrivateKey_file(clientsslctx, certfile.c_str(), SSL_FILETYPE_PEM) <=0 )
	{
		if(config.syslog)
		{
			syslog(LOG_ERR, "error loading private key!");
		}
		debugmsg("-SYSTEM-", "error loading private key!");
		return 0;
	}
	
	FILE *fp = fopen(certfile.c_str(), "r");
	if (fp == NULL) 
	{ 
		debugmsg("SYSTEM","[tmp_dh_cb] could not open file!"); 
		return 0;
	}
	globaldh = PEM_read_DHparams(fp, NULL, NULL, NULL);
	fclose(fp);

    if(config.crypted_cert)
    {
        if(!kill_cert(certfile))
        {
            debugmsg("SYSTEM","error deleting tmp cert"); 
        }
    }
    
	if ( !SSL_CTX_check_private_key(clientsslctx))
	{
		if(config.syslog)
		{
			syslog(LOG_ERR, "key invalid");
		}
		debugmsg("-SYSTEM-", "key invalid");
		return 0;
	}
	
	SSL_CTX_set_session_cache_mode(clientsslctx,SSL_SESS_CACHE_OFF);

	SSL_CTX_set_tmp_dh_callback(clientsslctx, tmp_dh_cb);
	char	*tls_cipher_list = "ALL:!EXP";
	SSL_CTX_set_cipher_list(clientsslctx, tls_cipher_list);

	if(!THREAD_setup())
	{
		return 0;
	}
	
	return 1;
}
