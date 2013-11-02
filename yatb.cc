// define configfile password here to allow automatic startup
//#define config_key "somekey"

#include "global.h"
#include "config.h"
#include "counter.h"
#include "lock.h"
#include "controlthread.h"
#include "stringlist.h"
#include "tools.h"
#include "tls.h"

CConfig config;
list<CControlThread*> conlist;
CCounter totalcounter;
int listen_sock;
struct sockaddr_in listen_addr;
int nr_logins=0;
CStringlist adminlist,fxpfromsitelist,fxptositelist,sslexcludelist,entrylist;
time_t start_time;
int nr_threads = 0;
int use_blowconf = 1;
long *lock_count;
SSL_CTX *clientsslctx;
CLock list_lock,config_lock,globals_lock;
string bk = "";
string conffile;



pthread_attr_t threadattr;

int main(int argc,char *argv[])
{		
	
	pthread_attr_init(&threadattr);
  pthread_attr_setdetachstate(&threadattr,PTHREAD_CREATE_DETACHED);
	
	if (argc < 2 || argc > 3)
	{		
		cout << version << "\n";
		cout << "Builddate: " << builddate << "\n";
		cout << "Using " << SSLeay_version(0) << "\n";
		cout << "Usage:\n\t yatb configfile\n";
		cout << "\t or yatb -u configfile for uncrypted conf file\n";
		return -1;
	}
	
	if (argc == 3)
	{
		string t(argv[1]);
		if (t == "-u")
		{			
			use_blowconf = 0;
		}
		else
		{
			cout << version << "\n";
			cout << "Builddate: " << builddate << "\n";
			cout << "Using " << SSLeay_version(0) << "\n";
			cout << "Usage:\n\t yatb configfile\n";
			cout << "\t or yatb -u configfile for uncrypted conf file\n";
			return -1;
		}
	}
	if (use_blowconf == 1)
	{
		conffile = argv[1];
		
		#ifndef config_key
			char *k;
			k = getpass("Enter blowfish key: ");
			bk = k;
		#endif
		
		#ifdef config_key
			bk = config_key;
		#endif
		
		if (!config.readconf(conffile,bk))
		{		
			return -1;
		}
	}
	else
	{
		conffile = argv[2];
		if (!config.readconf(conffile,bk))
		{		
			return -1;
		}
	}
	
	adminlist.Insert(config.admin_list);
	fxpfromsitelist.Insert(config.fxp_fromsite_list);
	fxptositelist.Insert(config.fxp_tosite_list);
	sslexcludelist.Insert(config.sslexclude_list);
	entrylist.Insert(config.entry_list);
	
	
	
	// fork or exit
	if (!config.debug || (!config.log_to_screen && config.debug))
	{	
		debugmsg("-SYSTEM-","[main] try to run as daemon");		
    if( daemon(1,1) != 0)
    {    	
    	debugmsg("-SYSTEM-","[main] error while forking!",errno);
    }
    else
    {    	
    	debugmsg("-SYSTEM-","[main] running as daemon now");
    }
     
	}
	
	//when using entrys disable some options
	if (config.entry_list != "")
	{		
		config.fake_server_string = 0;
		config.use_ident = 0;		
	}
	
	start_time = time(NULL);
	
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(config.listen_port);
	if (config.listen_ip != "")
	{
		listen_addr.sin_addr.s_addr = inet_addr(config.listen_ip.c_str());
	}
	else
	{
		listen_addr.sin_addr.s_addr = INADDR_ANY;
	}
	memset(&(listen_addr.sin_zero), '\0', 8);

	srand(12);

	if(config.syslog)
	{		
		 openlog("yatb",LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);		
	}
	
	if((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		debugmsg("-SYSTEM-","Unable to create socket!");
		if (config.syslog) 
		{			
			syslog(LOG_ERR, "Unable to create socket!");
		}
		return -1;
	}
	
	int yes = 1;
	if (setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
	{
		debugmsg("-SYSTEM-","setsockopt error!");
		if (config.syslog)
		{ 			
			syslog(LOG_ERR, "setsockopt error!");
		}
		return -1;
	}
	
	if (bind(listen_sock, (struct sockaddr *)&listen_addr, sizeof(struct sockaddr)) == -1)
	{
		debugmsg("-SYSTEM-","Unable to bind to port!");
		if (config.syslog) 
		{			
			syslog(LOG_ERR, "Unable to bind to port!");
		}
		return -1;
	}
	

	if (listen(listen_sock, config.pending) == -1)
	{
		debugmsg("-SYSTEM-","Unable to listen!");
		if (config.syslog) 
		{			
			syslog(LOG_ERR, "Unable to listen!");
		}
		return -1;
	}
	
	if (!ssl_setup())
	{
		return -1;
	}
	
	
	//make gethostbyname working after chroot
	struct sockaddr_in tmpaddr = GetIp("www.glftpd.at",21);
	
	char *cwd = getcwd(NULL, 4096);	
	if (chroot(cwd))
	{
		debugmsg("-SYSTEM-"," - WARNING: - Could not chroot");
		if (config.syslog) 
		{			
			syslog(LOG_ERR, " - WARNING: - Could not chroot" );
		}
	}	
	else
	{
		
		chdir("/");
	}	
	free(cwd);
	
		
	signal(SIGPIPE, SIG_IGN);
	
	
		
	int pid = getpid();
	
	ofstream pidfile(config.pidfile.c_str(), ios::out | ios::trunc);
	if (!pidfile)
	{
		debugmsg("-SYSTEM-"," - WARNING: - Error creating pid file!");
		if (config.syslog) 
		{			
			syslog(LOG_ERR, " - WARNING: - Error creating pid file!");
		}
	}
	else
	{
		
		pidfile << pid << "\n";
		pidfile.close();
	}
	
	if(setuid(config.uid) < 0)
	{
		debugmsg("-SYSTEM-"," - WARNING: - Could not set uid!");
		if (config.syslog) 
		{			
			syslog(LOG_ERR, " - WARNING: - Could not set uid!");
		}
			
	}
	
	
	if (getuid() == 0)
	{
		debugmsg("-SYSTEM-","Could not run as root!");
		if (config.syslog) 
		{			
			syslog(LOG_ERR, "Could not run as root!");
		}
		THREAD_cleanup();
		if (clientsslctx != NULL) { SSL_CTX_free(clientsslctx); }
		return -1;
	}
	
	while(1)
	{
		
		debugmsg("-SYSTEM-","[main] accept start");
		struct sockaddr_in tmp_addr;
		socklen_t size = sizeof(tmp_addr);
		int tmp_sock;
		
		tmp_sock = accept(listen_sock,(struct sockaddr *)&tmp_addr,&size);
		if (tmp_sock > 0)
		{
			if (config.thread_limit == 0 || nr_threads < config.thread_limit)
			{
				debugmsg("-SYSTEM-","[main] list create start");
				// create a new connection and put it into the list
				list_lock.Lock();
			
				CControlThread *tmp;
				tmp = new CControlThread(tmp_sock,tmp_addr);
				conlist.push_back(tmp);
			
				list_lock.UnLock();
				debugmsg("-SYSTEM-","[main] try to start controlthread");
				if(pthread_create(&tmp->tid,&threadattr,makethread,tmp) != 0)
				{
					debugmsg("-SYSTEM-","[main] error creating thread",errno);
					list_lock.Lock();
					conlist.remove(tmp);
					list_lock.UnLock();
				}
				
				
				debugmsg("-SYSTEM-","[main] list create end");
			}
			else
			{
				debugmsg("-SYSTEM-","[main] maximum # of threads reached");
				close(tmp_sock);
			}
		}
		debugmsg("-SYSTEM-","[main] accept end");
			
		

	}
	THREAD_cleanup();
	if (clientsslctx != NULL) { SSL_CTX_free(clientsslctx); }
	
	return 0;
}
