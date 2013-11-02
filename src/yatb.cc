// define configfile/certfile password here to allow automatic startup
// this is not secure!!
//#define config_key "somekey"
//#define cert_key "somekey"

#include "global.h"
#include "config.h"
#include "counter.h"
#include "lock.h"
#include "controlthread.h"
#include "stringlist.h"
#include "tools.h"
#include "tls.h"
#include "forward.h"
#include "iplist.h"

#ifndef SOLARIS
#define SOLARIS (defined(sun) && (defined(__svr4__) || defined(__SVR4)))
#endif


#if SOLARIS
int
daemon(int nochdir, int noclose)
{
        int fd;

        switch (fork()) {
        case -1:
                return (-1);
        case 0:
                break;
        default:
                exit(0);
        }

        if (setsid() == -1)
                return (-1);

        if (!nochdir)
                (void)chdir("/");

        if (!noclose && (fd = open(PATH_DEVNULL, O_RDWR, 0)) != -1) {
                (void)dup2(fd, STDIN_FILENO);
                (void)dup2(fd, STDOUT_FILENO);
                (void)dup2(fd, STDERR_FILENO);
                if (fd > 2)
                        (void)close (fd);
        }
        return (0);
}
#endif

CConfig config;
CIplist iplist;
list<CControlThread*> conlist;
CCounter totalcounter,daycounter,weekcounter,monthcounter;
int listen_sock;
struct sockaddr_in listen_addr;
long int nr_logins=0;
CStringlist adminlist,fxpfromsitelist,fxptositelist,sslexcludelist,entrylist;
time_t start_time;

int use_blowconf = 1;
long *lock_count;
SSL_CTX *clientsslctx;
CLock list_lock,config_lock,globals_lock,sock_lock;
string bk = "";
string cert_bk;
string conffile,yatbfilename;
string lastday="",lastmonth="";
int lastweek = 0;

void reload(int)
{
	debugmsg("SYSTEM","HUP signal - try to reload config");
	CConfig tmpconf;
	if (!tmpconf.readconf(conffile,bk))
	{
		debugmsg("SYSTEM","failed to reload config");
		
	}
	else
	{
		config = tmpconf;
		iplist.readlist(config.site_ip,config.site_port);
		debugmsg("SYSTEM","config reloaded");						
		//when using entrys disable some options
		if (config.entry_list != "")
		{		
			config.fake_serverstring = 0;
			config.use_ident = 0;		
		}
		adminlist.Insert(config.admin_list);
		fxpfromsitelist.Insert(config.fxp_fromsite_list);
		fxptositelist.Insert(config.fxp_tosite_list);
		sslexcludelist.Insert(config.sslexclude_list);
	}
}

void *trafficthread(void *)
{
	while(1)
	{
		debugmsg("-SYSTEM-","traffic thread start");
		time_t t;
		t = time(NULL);
		string ti = asctime(localtime(&t));
		//debugmsg("-SYSTEM-",ti);
		
		string tmpday = ti.substr(0,3);
		string tmpmonth = ti.substr(4,3);
		int tmpweek = atoi(ti.substr(7,3).c_str());
		
		if (lastday != tmpday)
		{
			debugmsg("-SYSTEM-","new day - reset limit");
			lastday = tmpday;
			daycounter.reset();
		}
		if(lastweek != tmpweek && tmpday == "Mon")
		{
			debugmsg("-SYSTEM-","new week - reset limit");
			lastweek = tmpweek;
			weekcounter.reset();
		}
		if(lastmonth != tmpmonth)
		{
			debugmsg("-SYSTEM-","new month - reset limit");
			lastmonth = tmpmonth;
			monthcounter.reset();
		}
		debugmsg("-SYSTEM-","traffic thread end");
		sleep(60);
		
	}
	return NULL;
}

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
	
	yatbfilename = argv[0];
	unsigned int pos;
	pos = yatbfilename.rfind("/",yatbfilename.length());
	if(pos != string::npos)
	{
		yatbfilename = yatbfilename.substr(pos+1,yatbfilename.length() - pos-1);
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
			k = getpass("Enter config blowfish key: ");
			bk = k;
			memset(k, 0,strlen(k));
		   
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
	
	if(config.crypted_cert)
	{
	  #ifndef cert_key
		char *ck;
		ck = getpass("Enter cert blowfish key: ");
		cert_bk = ck;
		memset(ck, 0,strlen(ck));
	  #endif
	  #ifdef cert_key
		cert_bk = cert_key;
    #endif
	  
	}
	
	adminlist.Insert(config.admin_list);
	fxpfromsitelist.Insert(config.fxp_fromsite_list);
	fxptositelist.Insert(config.fxp_tosite_list);
	sslexcludelist.Insert(config.sslexclude_list);
	entrylist.Insert(config.entry_list);
	if(!iplist.readlist(config.site_ip,config.site_port))
	{
		debugmsg("-SYSTEM-","error in ip/port list");
		return -1;
	}
	
	
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
		config.fake_serverstring = 0;
		config.use_ident = 0;		
	}
	
	start_time = time(NULL);

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
		
	if(!SocketOption(listen_sock,SO_REUSEADDR))
	{
		debugmsg("-SYSTEM-","setsockopt error!");
		if (config.syslog)
		{ 			
			syslog(LOG_ERR, "setsockopt error!");
		}
		return -1;
	}
	
	if (!Bind(listen_sock, config.listen_ip, config.listen_port))
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
	struct sockaddr_in tmpaddr = GetIp("www.glftpd.com",21);
	
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
	signal(SIGHUP,reload);
	
		
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
	
	if(config.use_forwarder)
	{
		// start forward thread
		CForward *fw;
		fw = new CForward(config.forwarder_sport,config.forwarder_dport,config.forwarder_ip);
		if(pthread_create(&fw->tid,&threadattr,makeforwardthread,fw) != 0)
		{
			debugmsg("-SYSTEM-","[main] error creating forward thread",errno);	
			delete fw;	
		}
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
	
	// start traff lmit control thread
	pthread_t tid;
	if(pthread_create(&tid,&threadattr,trafficthread,NULL) != 0)
	{
		debugmsg("-SYSTEM-","could not create traffic control thread");
	}
	
	while(1)
	{				
		int tmp_sock = -1;
		string clientip;
		int clientport;
		int shouldquit = 0;
		
		if (Accept(listen_sock,tmp_sock,clientip,clientport,0,shouldquit))
		{
			if(trafficcheck())
			{
				printsockopt(listen_sock,"listen_sock");
				debugmsg("-SYSTEM-","[main] list create start");
				// create a new connection and put it into the list
				list_lock.Lock();
				
				string sip; // site ip
				int sport; // site port
				
				iplist.getip(sip,sport);
								
				CControlThread *tmp;
				tmp = new CControlThread(tmp_sock,clientip,clientport,sip,sport);
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
				Close(tmp_sock,"tmp_sock");
			}
			
		}
		
	}
	THREAD_cleanup();
	if (clientsslctx != NULL) { SSL_CTX_free(clientsslctx); }
	
	return 0;
}
