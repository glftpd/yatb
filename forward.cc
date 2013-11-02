#include "tools.h"
#include "global.h"
#include "config.h"
#include "forward.h"

void *makeforwardthread(void* pData)
{
	debugmsg("-SYSTEM-","[makeforwardthread] start");
	CForward *pf = (CForward*)pData;
	try
	{
		pf->mainloop();
	}
	catch(...)
	{
		debugmsg("--FORWARD EXEPTION--","");
	}
	debugmsg("-SYSTEM-","[makeforwardthread] delete pf");
	delete pf;
	debugmsg("-SYSTEM-","[makeforwardthread] end");
	return NULL;
}



class CFwData
{
	public:
	
	friend void *makeforwarddatathread(void *pData);
	
	CFwData(int cs,int dp,string ip)
	{
		dport = dp;
		server_ip = ip;
		client_sock = cs;
		buffer = new char[config.buffersize];
		sitessl = NULL;
		clientssl = NULL;
		sitesslctx = NULL;
	}
	
	~CFwData()
	{
		close(client_sock);
		close(server_sock);
		delete [] buffer;
	}
	
	pthread_t tid; // thread id
	
	private:
	char *buffer;
	int client_sock,server_sock;
	int dport;
	string server_ip;
	SSL *sitessl,*clientssl;
	SSL_CTX *sitesslctx;
	
	void mainloop(void)
	{
		pthread_detach(pthread_self());
		int shouldquit = 0;
		if ((server_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
		{			
			return;
		}
		
		if (config.connect_ip != "")
		{
			debugmsg("-FW-THREAD-","[datathread] try to set connect ip for site connect");
			
			if(!Bind(server_sock,config.connect_ip,0))
			{
				debugmsg("-FW-THREAD-","[datathread] connect ip - could not bind",errno);
				return;
			}
		}
		
		debugmsg("-FW-THREAD-","[datathread] try to connect to site");
		if(!Connect(server_sock,server_ip,dport,config.connect_timeout,shouldquit))
		{
			
			debugmsg("-FW-THREAD-", "[datathread] could not connect to site!",errno);
			return;
		}
		
		fd_set data_readfds;
		
		while(1)
		{
			FD_ZERO(&data_readfds);
			FD_SET(client_sock,&data_readfds);
			FD_SET(server_sock,&data_readfds);
			struct timeval tv;
			tv.tv_sec = config.read_write_timeout;
			tv.tv_usec = 0;
			int tmpsock;
	
			if (server_sock > client_sock)
			{
				tmpsock = server_sock;
			}
			else
			{
				tmpsock = client_sock;
			}
			
			if (select(tmpsock+1, &data_readfds, NULL, NULL, &tv) < 1)
			{
				debugmsg("-FW-THREAD-","[datathread] read timeout",errno);
				return;
			}
	
			// read from site - send to client
			if (FD_ISSET(server_sock, &data_readfds))
			{


				memset(buffer,'\0',1);
		
				int rc;
				if(!DataRead(server_sock,buffer,rc,sitessl))
				{					
					break;
				}
	
				if(!DataWrite(client_sock,buffer,rc,clientssl))
				{					
					break;
				}
				

			}
			// read from client - send to site
			if (FD_ISSET(client_sock, &data_readfds))
			{
	
				memset(buffer,'\0',1);
	
				int rc;
				if(!DataRead(client_sock,buffer,rc,clientssl))
				{					
					break;
				}
				
				if(!DataWrite(server_sock,buffer,rc,sitessl))
				{				
					break;
				}
	
			}
			
		}
	}
	
};

void *makeforwarddatathread(void* pData)
{
	debugmsg("-SYSTEM-","[makeforwarddatathread] start");
	CFwData *pf = (CFwData*)pData;
	try
	{
		pf->mainloop();
	}
	catch(...)
	{
		debugmsg("--FORWARD EXEPTION--","");
	}
	debugmsg("-SYSTEM-","[makeforwarddatathread] delete pf");
	delete pf;
	debugmsg("-SYSTEM-","[makeforwarddatathread] end");
	return NULL;
}

CForward::CForward(int sp,int dp, string ip)
{
	sport = sp;
	dport = dp;
	fw_ip = ip;
}

CForward::~CForward()
{
}

void CForward::mainloop(void)
{
	if((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		debugmsg("-SYSTEM-","Unable to create forwardsocket!");
		
		return ;
	}
		
	if(!SocketOption(listen_sock,SO_REUSEADDR))
	{
		debugmsg("-SYSTEM-","setsockopt error!");
		
		return ;
	}
	
	if (!Bind(listen_sock, config.listen_ip, sport))
	{
		debugmsg("-SYSTEM-","Unable to bind to port!");
		
		return ;
	}
	

	if (listen(listen_sock, config.pending) == -1)
	{
		debugmsg("-SYSTEM-","Unable to listen!");
		
		return ;
	}
	
	if(setuid(config.uid) < 0)
	{
		debugmsg("-SYSTEM-"," - WARNING: - Could not set uid!");
					
	}
	
	while(1)
	{				
		int tmp_sock;
		string clientip;
		int clientport;
		int shouldquit = 0;
		
		if (Accept(listen_sock,tmp_sock,clientip,clientport,0,shouldquit))
		{
			CFwData *dt;
			dt = new CFwData(tmp_sock,dport,fw_ip);
			if(pthread_create(&dt->tid,NULL,makeforwarddatathread,dt) != 0)
			{
				debugmsg("-SYSTEM-","[main] error creating forward thread",errno);	
				delete dt;	
			}
		}
	}
	
}

