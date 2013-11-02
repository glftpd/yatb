#include "datathread.h"
#include "controlthread.h"
#include "tools.h"
#include "config.h"
#include "stringlist.h"
#include "counter.h"
#include "tls.h"

// c wrapper for creating data connection thread
void *makedatathread(void* pData)
{
	debugmsg("-SYSTEM-","[makedatathread] start");
	CDataThread *pDataThread = (CDataThread*)pData;
	try
	{
		pDataThread->dataloop();
	}
	catch(...)
	{
		debugmsg("--DATATHREAD EXEPTION--","");
	}
	debugmsg("-SYSTEM-","[makedatathread] closeconnection");
	pDataThread->closeconnection();
 	
	debugmsg("-SYSTEM-","[makedatathread] end");
	return NULL;
}

CDataThread::CDataThread(int cpsv, int tt, int pp, int rl, int ussl, int actvcon, string usrname, string cip, string pip, string aip,int pport, int aport, int nport, CControlThread *ct,string pcmd)
{
	debugmsg(username,"[datathread] constructor start");
	username = usrname;
	usingssl = ussl;
	activecon = actvcon;
	passiveip = pip;
	activeport = aport;
	passiveport = pport;
	newport = nport;
	activeip = aip;
	clientip = cip;
	relinked = rl;
	transfertype = tt;
	sslprotp = pp;
	datalisten_sock = -1;
	datasite_sock = -1;
	dataclient_sock = -1;
	shouldquit = 0;
	cpsvcmd = cpsv;
	passivecmd = pcmd;
	clientssl = NULL;
	sitessl = NULL;
	sitesslctx = NULL;
	
	buffer = new char[config.buffersize];
	
	controlthread = ct;
	debugmsg(username,"[datathread] constructor end");
}

CDataThread::~CDataThread()
{
	debugmsg(username,"[datathread] destructor start");
	closeconnection();			
	debugmsg(username,"[datathread] delete buffer");
	delete [] buffer;
	
	debugmsg(username,"[datathread] destructor end");
}

void CDataThread::closeconnection(void)
{	
	debugmsg(username, "[closeconnection] start");
	if (usingssl)
	{
		
		if (sitessl != NULL) 
		{
			debugmsg(username,"[closeconnection] close site ssl"); 
			SSL_shutdown(sitessl);
						
		}
		
		if (clientssl != NULL) 
		{
			debugmsg(username,"[closeconnection] close client ssl"); 
			SSL_shutdown(clientssl); 	
		
		}
	}
	
		debugmsg(username,"[closeconnection] close site sock"); 
		close(datasite_sock); 
	
		debugmsg(username,"[closeconnection] close client sock"); 
		close(dataclient_sock); 
	
		debugmsg(username,"[closeconnection] close listen sock");		
		close(datalisten_sock); 	
		
		if (usingssl)
	{			
		
		if (sitessl != NULL) 
		{ 
			debugmsg(username, "[closeconnection] free site ssl");
			SSL_free(sitessl); 
			sitessl = NULL; 
		}
		
		if (clientssl != NULL) 
		{ 
			debugmsg(username, "[closeconnection] free client ssl");
			SSL_free(clientssl); 
			clientssl = NULL; 
		}
		
		if (sitesslctx != NULL) 
		{ 
			debugmsg(username, "[closeconnection free sitesslctx");
			SSL_CTX_free(sitesslctx); 
			sitesslctx = NULL; 
		}
		
		debugmsg(username, "[closeconnection] free ssl error queue");
		ERR_remove_state(0);
	}
	debugmsg(username, "[closeconnection] end");
}








void CDataThread::dataloop(void)
{
	debugmsg(username,"[datathread] dataloop start");
	
	// try to connect to site
	if ((datasite_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		debugmsg(username, "[datathread] unable to create site sock!",errno);		
		
		return;
	}
	
	if (config.connect_ip != "")
	{
		debugmsg(username,"[datathread] try to set connect ip for site connect");
		
		if(!Bind(datasite_sock,config.connect_ip,0))
		{
			debugmsg(username,"[datathread] connect ip - could not bind",errno);
			return;
		}
	}
	
	debugmsg(username,"[datathread] try to connect to site");
	if(!Connect(datasite_sock,passiveip,passiveport,config.connect_timeout,shouldquit))
	{
		
		debugmsg(username, "[datathread] could not connect to site!",errno);
		return;
	}
	
	
	
	if ((dataclient_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		debugmsg(username, "[datathread] unable to create client sock!",errno);		
		
		return;
	}
	
	
	// passive connection - try to listen
	if(!activecon)
	{
		debugmsg(username,"[datathread] passive connection - listen");
		if ((datalisten_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
		{
			debugmsg(username, "[datathread] unable to create listen sock!",errno);		
			
			return;
		}
		if(!SocketOption(datalisten_sock,SO_REUSEADDR))
		{
			debugmsg(username,"setsockopt error!");
			return;
		}
		if (!Bind(datalisten_sock, config.listen_ip, newport))
		{
			debugmsg(username,"Unable to bind to port!");
			
			return ;
		}
		if (listen(datalisten_sock, config.pending) == -1)
		{
			debugmsg(username,"Unable to listen!");
			
			return ;
		}
		string clip;
		int clport;
		if (!controlthread->Write(controlthread->client_sock,passivecmd,controlthread->clientssl))
		{								
			return;
		}
		if(!Accept(datalisten_sock,dataclient_sock,clip,clport,config.connect_timeout,shouldquit))
		{
			debugmsg(username,"Accept failed!");
			
			return ;
		}
	}
	// active connection - try to connect
	else
	{
		debugmsg(username,"[datathread] active connection");
		if (config.listen_ip != "")
		{
			debugmsg(username,"[datathread] try to set connect ip for client connect");
			
			if(!Bind(dataclient_sock,config.listen_ip,0))
			{
				debugmsg(username,"[datathread] connect ip - could not bind",errno);
				return;
			}
		}
		debugmsg(username,"[datathread] active connect to: " + activeip);
		if(!Connect(dataclient_sock,activeip,activeport,config.connect_timeout,shouldquit))
		{			
			debugmsg(username, "[datathread] could not connect to client!",errno);
			return;
		}
		
		debugmsg(username,"[datathread] active connection successfull");
	}
	
	// ssl stuff
	if(!config.ssl_forward && usingssl && sslprotp)
	{
		if(!SslConnect(datasite_sock,&sitessl,&sitesslctx))
		{
			debugmsg(username, "[datathread] ssl connect failed",errno);
			return;
		}
	}
	
	if((usingssl && relinked && sslprotp) || (!config.ssl_forward && usingssl && sslprotp))
	{
		if(!SslAccept(dataclient_sock,&clientssl,&clientsslctx))
		{
			debugmsg(username, "[datathread] ssl accept failed",errno);
			return;
		}
	}
	
/*
	stringstream ss;
	ss << "active port: " << activeport << " passiveport: " << passiveport;
	debugmsg(username,"[datathread] " + ss.str());
	if (sslprotp) { debugmsg(username,"[datathread] sslprotp = 1"); }
	else { debugmsg(username,"[datathread] sslprotp = 0"); }

	datalisten_sock = -1;
	dataclient_sock = -1;
	datasite_sock = -1;
	
	struct sockaddr_in datalisten_addr,datasite_addr;

	
	if(config.listen_ip == "")
	{
		datalisten_addr.sin_family = AF_INET;
		datalisten_addr.sin_port = htons(passiveport);
		datalisten_addr.sin_addr.s_addr = INADDR_ANY;
		memset(&(datalisten_addr.sin_zero), '\0', 8);
	}
	else
	{
		datalisten_addr = GetIp(config.listen_ip,passiveport);
		
	}
	

	
	if (!relinked)
	{
		datasite_addr = GetIp(config.site_ip,siteport);
		
	}
	else
	{
		datasite_addr = GetIp(config.relink_ip,siteport);
		
	}
	

	// bind listen socket if passive connection
	if (!activecon)
	{
		if((datalisten_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			debugmsg(username,"[datathread] Unable to create data socket!",errno);
						
			return;
		}
		debugmsg(username,"[datathread] bind to port");
		if (bind(datalisten_sock, (struct sockaddr *)&datalisten_addr, sizeof(struct sockaddr)) == -1)
		{
			debugmsg(username,"[datathread] Unable to bind to data port!",errno);
			//controlthread->control_write(controlthread->client_sock,"427 Could not bind to dataport!\r\n",controlthread->clientssl);
				
			return;
		}
		int yes = 1;
		if (setsockopt(datalisten_sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
		{
			debugmsg(username,"[datathread] setsockopt error!",errno);
			
			return;
		}
		debugmsg(username,"[datathread] listen");
		if (listen(datalisten_sock, config.pending) == -1)
		{
			debugmsg(username,"[datathread] Unable to listen!",errno);
				
			return;
		}
		setnonblocking(datalisten_sock);
	}


	if((datasite_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		debugmsg(username,"[datathread] Unable to create socket!",errno);
		
		return;
	}
	debugmsg(username,"[datathread] try to connect to site");
	if (config.connect_ip != "")
	{
		if(!Bind(datasite_sock,config.connect_ip,0))
		{
			debugmsg(username,"[datathread] unable to bind");
			return;
		}		
	}
	setnonblocking(datasite_sock);
	
	if(!Connect(datasite_sock,datasite_addr,config.connect_timeout,shouldquit))
	{
		debugmsg(username,"[datathread] could not connect site data port",errno);		
		//controlthread->control_write(controlthread->client_sock,"427 Connect Error/Timeout!\r\n",controlthread->clientssl);
		return;
	}
	
	debugmsg(username,"[datathread] connected to site");
		
	
	if (transfertype == 1) { debugmsg(username,"[datathread] ascii transfer"); }
	if (transfertype == 2) { debugmsg(username,"[datathread] binary transfer"); }
	
	

	if (!activecon)
	{
		
		if(!Accept(datalisten_sock,dataclient_sock,datalisten_addr,config.connect_timeout,shouldquit))
		{
			//controlthread->control_write(controlthread->client_sock,"427 Accept Error/Timeout!\r\n",controlthread->clientssl);
			debugmsg(username, "[datathread] accept error",errno);				
			return;
		}
		
		else
		{
			
			if (datalisten_addr.sin_addr.s_addr != client_addr.sin_addr.s_addr)
			{
				if(config.enforce_tls_fxp && !sslprotp)
				{
					
					if(config.use_ssl_exclude && sslexcludelist.IsInList(username) || relinked)
					{
					}
					else
					{
						debugmsg(username,"[datathread] tls fxp required");
						controlthread->control_write(controlthread->client_sock,"427 TLS FXP required!\r\n",controlthread->clientssl);
											
						return;
					}
				}
				if (config.use_fxpfromsite_list && !adminlist.IsInList(username) && !fxpfromsitelist.IsInList(username))
				{
					debugmsg(username,"[datathread] fxp from site not allowed");
					
					controlthread->control_write(controlthread->client_sock,"427 FXP from site not allowed!\r\n",controlthread->clientssl);
						
					return;
				}
			}
			else
			{
				if (config.enforce_tls && !sslprotp)
				{
					
					if(config.use_ssl_exclude && sslexcludelist.IsInList(username) || relinked)
					{
					}
					else
					{
						debugmsg(username,"[datathread] tls required");
						controlthread->control_write(controlthread->client_sock,"427 TLS required!\r\n",controlthread->clientssl);
											
						return;
					}
				}
			}
		}

		
		
	}
	else
	{
		debugmsg(username,"[datathread] active conn!");
		struct sockaddr_in active_addr;
		dataclient_sock = socket(AF_INET,SOCK_STREAM,0);
		if (dataclient_sock < 1) 
		{ 
			debugmsg(username,"[datathread] dataclientsock < 1",errno);
			
			
			return; 
		}
		active_addr = GetIp(activeip,activeport);
		
		setnonblocking(dataclient_sock);
		if (active_addr.sin_addr.s_addr != client_addr.sin_addr.s_addr)
		{
			if (config.enforce_tls_fxp && !sslprotp)
			{
				
				if(config.use_ssl_exclude && sslexcludelist.IsInList(username) || relinked)
				{
				}
				else
				{
					debugmsg(username,"[datathread] tls fxp required");
					controlthread->control_write(controlthread->client_sock,"427 TLS FXP required!\r\n",controlthread->clientssl);
								
					return;
				}
			}
			if (config.use_fxptosite_list && !adminlist.IsInList(username) && !fxptositelist.IsInList(username))
			{
				debugmsg(username,"[datathread] fxp to site not allowed");
				
				controlthread->control_write(controlthread->client_sock,"427 FXP to site not allowed!\r\n",controlthread->clientssl);
				
				
				return;
			}
		}
		else
		{
			if (config.enforce_tls && !sslprotp)
			{
				if(config.use_ssl_exclude && sslexcludelist.IsInList(username) || relinked)
				{
				}
				else
				{
					debugmsg(username,"[datathread] tls required");
				
					controlthread->control_write(controlthread->client_sock,"427 TLS required!\r\n",controlthread->clientssl);
								
					return;
				}
			}
		}
		if (config.listen_ip != "")
		{
			debugmsg(username,"[datathread] set connection ip");
			struct sockaddr_in connect_addr;
			connect_addr = GetIp(config.listen_ip,0);
			
			bind(dataclient_sock,(struct sockaddr *)&connect_addr, sizeof(struct sockaddr));
		}
		
		if(!Connect(dataclient_sock,active_addr,config.connect_timeout,shouldquit))
		{
			//controlthread->control_write(controlthread->client_sock,"427 Connect Error/Timeout!\r\n",controlthread->clientssl);
			debugmsg(username,"[datathread] fxp connect failed",errno);			
			return;
		}
	
	}
*/
	
	fd_set data_readfds;
	
	
	
	debugmsg(username,"[datathread] entering dataloop");
	while (!shouldquit)
	{

		FD_ZERO(&data_readfds);
		FD_SET(dataclient_sock,&data_readfds);
		FD_SET(datasite_sock,&data_readfds);
		struct timeval tv;
		tv.tv_sec = config.read_write_timeout;
		tv.tv_usec = 0;
			
		int tmpsock;

		if (datasite_sock > dataclient_sock)
		{
			tmpsock = datasite_sock;
		}
		else
		{
			tmpsock = dataclient_sock;
		}
		
		if (select(tmpsock+1, &data_readfds, NULL, NULL, &tv) < 1)
		{
			debugmsg(username,"[datathread] read timeout",errno);
			return;
		}

		// read from site - send to client
		if (FD_ISSET(datasite_sock, &data_readfds))
		{


			memset(buffer,'\0',1);
	
			int rc;
			if(!Read(datasite_sock,buffer,rc,sitessl))
			{					
				break;
			}

			if(!Write(dataclient_sock,buffer,rc,clientssl))
			{					
				break;
			}
				

		}
		// read from client - send to site
		if (FD_ISSET(dataclient_sock, &data_readfds))
		{

			memset(buffer,'\0',1);

			int rc;
			if(!Read(dataclient_sock,buffer,rc,clientssl))
			{					
				break;
			}
			
			if(!Write(datasite_sock,buffer,rc,sitessl))
			{				
				break;
			}

		}

	}

	debugmsg(username,"[datathread] call close connection");
	closeconnection();
	debugmsg(username,"[datathread] end");
	return;

}

int CDataThread::Read(int sock ,char *buffer,int &nrbytes,SSL *ssl)
{
	if(!DataRead(sock ,buffer,nrbytes,ssl))
	{
		debugmsg(username,"[DataRead] read failed!");
		return 0;
	}
	if(sock == dataclient_sock)
	{
		controlthread->localcounter.addrecvd(nrbytes);
		totalcounter.addrecvd(nrbytes);
	}
	else if(sock == datasite_sock)
	{
	}
	return 1;
}

int CDataThread::Write(int sock,char *data,int nrbytes,SSL *ssl)
{
	if(!DataWrite(sock,data,nrbytes,ssl))
	{
		debugmsg(username,"[DataWrite] write failed!");
		return 0;
	}
	if(sock == dataclient_sock)
	{
		controlthread->localcounter.addsend(nrbytes);
		totalcounter.addsend(nrbytes);
	}
	else if(sock == datasite_sock)
	{
	}
	return 1;
}

