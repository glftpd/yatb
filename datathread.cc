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
		
		int tmp = datalisten_sock -1;
		
		Close(datalisten_sock,"datalisten_sock");
		
		Close(dataclient_sock,"dataclient_sock");
		
		Close(datasite_sock,"datasite_sock");		
		
		// some ugly hack
		Close(tmp,"tmp sock");	
		
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
	
	
	#if defined(__linux__) && defined(__i386__)
	stringstream ss;
	ss << username << " started datathread " << gettid();
	if (config.syslog) 
	{			
		syslog(LOG_ERR, ss.str().c_str());
	}	
	#endif
	
	// try to connect to site
	if ((datasite_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		debugmsg(username, "[datathread] unable to create site sock!",errno);		
		
		return;
	}
	PrintSock(datasite_sock,"datasite_sock");
	
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
	PrintSock(dataclient_sock,"dataclient_sock");
	// to store client ip from passive connection
	string clip;
	int clport;
	
	// passive connection - try to listen
	if(!activecon)
	{
		debugmsg(username,"[datathread] passive connection - listen");
		if ((datalisten_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
		{
			debugmsg(username, "[datathread] unable to create listen sock!",errno);		
			controlthread->Write(controlthread->client_sock,"425 Can't open passive connection\r\n",controlthread->clientssl);
			return;
		}
		PrintSock(datalisten_sock,"datalisten_sock");
		
		if (!Bind(datalisten_sock, config.listen_ip, newport))
		{
			debugmsg(username,"Unable to bind to port!");
			controlthread->Write(controlthread->client_sock,"425 Can't open passive connection\r\n",controlthread->clientssl);
			return ;
		}
		
		if(!SocketOption(datalisten_sock,SO_REUSEADDR))
		{
			debugmsg(username,"setsockopt error!");
			controlthread->Write(controlthread->client_sock,"425 Can't open passive connection\r\n",controlthread->clientssl);
			return;
		}
		
		
		if (listen(datalisten_sock, config.pending) == -1)
		{
			debugmsg(username,"Unable to listen!");
			controlthread->Write(controlthread->client_sock,"425 Can't open passive connection\r\n",controlthread->clientssl);
			return ;
		}
		
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
	
	if(usingssl) debugmsg(username, "[datathread] using ssl");
	if(sslprotp) debugmsg(username, "[datathread] protection set to private");
	if(!config.ssl_forward) debugmsg(username, "[datathread] not using ssl forward");
	
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
	
	
	printsockopt(datasite_sock,"datasite_sock");
	printsockopt(dataclient_sock,"dataclient_sock");
	
	debugmsg(username,"[datathread] wait for direction start");
	while(!controlthread->DirectionSet())
	{
		if(shouldquit) return;
	}
	debugmsg(username,"[datathread] wait for direction end");
	
	debugmsg(username,"[datathread] direction: " + controlthread->direction);
	
	// check ssl/fxp settings
	if(!activecon)
	{
		debugmsg(username,"[datathread] clientip: " + clientip + " source ip: " + clip);
		if(clip == clientip)
		{
			// direct download
			debugmsg(username, "[datathread] passive - direct dl");
			if(config.enforce_tls && (!sslprotp || !usingssl))
			{
				if(config.use_ssl_exclude && sslexcludelist.IsInList(username))
				{
					// allowed
				}
				else
				{
					controlthread->Write(controlthread->client_sock,"427 Use SSL!\r\n",controlthread->clientssl);
					return;
				}
			}
		}
		else
		{
			// fxp
			debugmsg(username, "[datathread] passive - fxp");
			if(config.enforce_tls_fxp && (!sslprotp || !usingssl))
			{
				if(config.use_ssl_exclude && sslexcludelist.IsInList(username))
				{
					// allowed
				}
				else
				{
					controlthread->Write(controlthread->client_sock,"427 Use SSL!\r\n",controlthread->clientssl);
					return;
				}
			}
			if(controlthread->direction == "download")
			{
				if(config.use_fxpfromsite_list && !fxpfromsitelist.IsInList(username))
				{
					controlthread->Write(controlthread->client_sock,"427 FXP not allowed!\r\n",controlthread->clientssl);
					return;
				}
			}
			if(controlthread->direction == "upload")
			{
				if(config.use_fxptosite_list && !fxptositelist.IsInList(username))
				{
					controlthread->Write(controlthread->client_sock,"427 FXP not allowed!\r\n",controlthread->clientssl);
					return;
				}
			}
		}
	}
	else
	{
		debugmsg(username,"[datathread] clientip: " + clientip + " activeip: " + activeip);
		if(activeip == clientip)
		{
			// direct download
			debugmsg(username, "[datathread] active - direct dl");
			if(config.enforce_tls && (!sslprotp || !usingssl))
			{
				if(config.use_ssl_exclude && sslexcludelist.IsInList(username))
				{
					// allowed
				}
				else
				{
					controlthread->Write(controlthread->client_sock,"427 Use SSL!\r\n",controlthread->clientssl);
					return;
				}
			}
		}
		else
		{
			// fxp
			debugmsg(username, "[datathread] active - fxp");
			if(config.enforce_tls_fxp && (!sslprotp || !usingssl))
			{
				if(config.use_ssl_exclude && sslexcludelist.IsInList(username))
				{
					// allowed
				}
				else
				{
					controlthread->Write(controlthread->client_sock,"427 Use SSL!\r\n",controlthread->clientssl);
					return;
				}
			}
			if(controlthread->direction == "download")
			{
				if(config.use_fxpfromsite_list && !fxpfromsitelist.IsInList(username))
				{
					controlthread->Write(controlthread->client_sock,"427 FXP not allowed!\r\n",controlthread->clientssl);
					return;
				}
			}
			if(controlthread->direction == "upload")
			{
				if(config.use_fxptosite_list && !fxptositelist.IsInList(username))
				{
					controlthread->Write(controlthread->client_sock,"427 FXP not allowed!\r\n",controlthread->clientssl);
					return;
				}
			}
		}
			
	}
	
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
			break;
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
		debugmsg(username,"[DataRead] read failed!",errno);
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
		debugmsg(username,"[DataWrite] write failed!",errno);
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

