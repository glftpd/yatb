#include "tls.h"
#include "tools.h"
#include "config.h"

static MUTEX_TYPE *mutex_buf = NULL;

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



int decrypt_cert(string tmpcert)
{
	int size;
	if (!filesize(config.cert_path,size))
	{
		debugmsg("-SYSTEM-", "cert decrypt error");
		return 0;
	}
	
	unsigned char *in,*out;
	
	out = new unsigned char [size];
	readfile(config.cert_path,&in,size);
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


int kill_file(string filename)
{
	debugmsg("-SYSTEM-","trying to kill " + filename);
	string command = "shred -n 25 -f -z " + filename;
	system(command.c_str());
    int size;
	if (!filesize(filename,size))
	{
		debugmsg("-SYSTEM-", "error getting filesize");
		return 0;
	}
	stringstream ss;
	ss << "file size is: " << size;
	debugmsg("-SYSTEM-",ss.str());
    unsigned char *out;
    out = new unsigned char [size];
    unsigned char *pattern;
    pattern = new unsigned char[3];
    int counter;
    
    // pattern 1
    pattern[0] = 0x55; pattern[1] = 0x55; pattern[2] = 0x55;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
    
    // pattern 2
    pattern[0] = 0xAA; pattern[1] = 0xAA; pattern[2] = 0xAA;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
    
    // pattern 3
    pattern[0] = 0x92; pattern[1] = 0x49; pattern[2] = 0x24;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
    
    // pattern 4
    pattern[0] = 0x49; pattern[1] = 0x24; pattern[2] = 0x92;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 5
	pattern[0] = 0x24; pattern[1] = 0x92; pattern[2] = 0x49;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 6
	pattern[0] = 0x00; pattern[1] = 0x00; pattern[2] = 0x00;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 7
	pattern[0] = 0x11; pattern[1] = 0x11; pattern[2] = 0x11;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 8
	pattern[0] = 0x22; pattern[1] = 0x22; pattern[2] = 0x22;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 9
	pattern[0] = 0x33; pattern[1] = 0x33; pattern[2] = 0x33;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 10
	pattern[0] = 0x44; pattern[1] = 0x44; pattern[2] = 0x44;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 11
	pattern[0] = 0x55; pattern[1] = 0x55; pattern[2] = 0x55;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 12
	pattern[0] = 0x66; pattern[1] = 0x66; pattern[2] = 0x66;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 12
	pattern[0] = 0x77; pattern[1] = 0x77; pattern[2] = 0x77;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 13
	pattern[0] = 0x88; pattern[1] = 0x88; pattern[2] = 0x88;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 14
	pattern[0] = 0x99; pattern[1] = 0x99; pattern[2] = 0x99;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 15
	pattern[0] = 0xAA; pattern[1] = 0xAA; pattern[2] = 0xAA;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 16
	pattern[0] = 0xBB; pattern[1] = 0xBB; pattern[2] = 0xBB;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 17
	pattern[0] = 0xCC; pattern[1] = 0xCC; pattern[2] = 0xCC;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 18
	pattern[0] = 0xDD; pattern[1] = 0xDD; pattern[2] = 0xDD;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 19
	pattern[0] = 0xEE; pattern[1] = 0xEE; pattern[2] = 0xEE;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 20
	pattern[0] = 0xFF; pattern[1] = 0xFF; pattern[2] = 0xFF;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 21
    pattern[0] = 0x92; pattern[1] = 0x49; pattern[2] = 0x24;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
    
    // pattern 22
    pattern[0] = 0x49; pattern[1] = 0x24; pattern[2] = 0x92;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 23
	pattern[0] = 0x24; pattern[1] = 0x92; pattern[2] = 0x49;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 24
    pattern[0] = 0x6D; pattern[1] = 0xB6; pattern[2] = 0xDB;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
    
    // pattern 25
    pattern[0] = 0xB6; pattern[1] = 0xDB; pattern[2] = 0x6D;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
	// pattern 26
	pattern[0] = 0xDB; pattern[1] = 0x6D; pattern[2] = 0xB6;
    counter = 0;
    for (int i=0; i < size;i++)
    {
    	out[i] = pattern[counter];
    	counter++;
    	if(counter == 3) counter = 0;
    }
    
    if (!writefile(filename,out,size))
	{
		delete pattern;		
		delete out;
		debugmsg("-SYSTEM-", "del file write error",errno);
		if(unlink(filename.c_str()) != 0)
	    {
	    	debugmsg("-SYSTEM-","cannot unlink file",errno);
	    	
	    }
		return 0;
	}
	
    delete pattern;
    delete out;
    
    if(unlink(filename.c_str()) != 0)
    {
    	debugmsg("-SYSTEM-","cannot unlink file",errno);
    	return 0;
    }
    return 1;
}
/*
static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
	if(preverify_ok == 2)
	{
		// do nothing
	}
	if(ctx == NULL)
	{
		// do nothing
	}
	return 1;
}
*/
int ssl_setup()
{
	clientsslctx = NULL;
	
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
	connectsslctx =	SSL_CTX_new(SSLv23_client_method());
	if (connectsslctx == NULL)
	{
		debugmsg("-SYSTEM-", "error connectcreating ctx");
		if(config.syslog)
		{
			syslog(LOG_ERR, "error creating connectctx");
		}
		return 0;
	}
  
	SSL_CTX_set_default_verify_paths(connectsslctx);
	SSL_CTX_set_options(connectsslctx,SSL_OP_ALL);
	SSL_CTX_clear_mode(connectsslctx,SSL_MODE_AUTO_RETRY);
	
	SSL_CTX_set_default_verify_paths(clientsslctx);
	SSL_CTX_set_options(clientsslctx, SSL_OP_SINGLE_DH_USE | SSL_OP_SINGLE_ECDH_USE | SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 /*SSL_OP_ALL*/);
	SSL_CTX_clear_mode(clientsslctx,SSL_MODE_AUTO_RETRY);

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
	
	debugmsg("-SYSTEM-", "try to load cert file");
	if (SSL_CTX_use_certificate_chain_file(clientsslctx,certfile.c_str()) <= 0)
	{		
		if(config.syslog)
		{
			syslog(LOG_ERR, "error loading cert file!");
		}
		debugmsg("-SYSTEM-", "error loading cert file!");
		return 0;
	}
	else 
	{
		SSL_CTX_use_certificate_chain_file(connectsslctx,certfile.c_str());
		debugmsg("-SYSTEM-", "try to load private key");
		if (SSL_CTX_use_PrivateKey_file(clientsslctx, certfile.c_str(), SSL_FILETYPE_PEM) <=0 )
		{			
			if(config.syslog)
			{
				syslog(LOG_ERR, "error loading private key!");
			}
			debugmsg("-SYSTEM-", "error loading private key!");
			return 0;
		}
		else
		{
			SSL_CTX_use_PrivateKey_file(connectsslctx, certfile.c_str(), SSL_FILETYPE_PEM);
		}
	}
    if(config.crypted_cert)
    {
        if(!kill_file(certfile))
        {
            debugmsg("SYSTEM","error deleting tmp cert"); 
        }
    }
    debugmsg("-SYSTEM-", "try to check private key");
	if ( !SSL_CTX_check_private_key(clientsslctx))
	{
		if(config.syslog)
		{
			syslog(LOG_ERR, "key invalid");
		}
		debugmsg("-SYSTEM-", "key invalid");
		return 0;
	}
    std::string sessionId("yatb");
    	
    SSL_CTX_set_session_cache_mode(clientsslctx,SSL_SESS_CACHE_SERVER);
    SSL_CTX_set_session_id_context(clientsslctx,(const unsigned char*)sessionId.c_str(),sessionId.size());
    
    SSL_CTX_set_session_cache_mode(connectsslctx,SSL_SESS_CACHE_OFF);

    SSL_CTX_set_dh_auto(clientsslctx, true);

    SSL_CTX_set_ecdh_auto(clientsslctx, 1);

	if(!THREAD_setup())
	{
		return 0;
	}
	
	return 1;
}
