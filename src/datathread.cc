#include "datathread.h"
#include "controlthread.h"
#include "tools.h"
#include "config.h"
#include "stringlist.h"
#include "counter.h"
#include "tls.h"
#include "whitelist.h"
#include "fpwhitelist.h"

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

int CDataThread::getQuit(void)
{
	int tmp;
	quitlock.Lock();
	tmp = shouldquit;
	quitlock.UnLock();
	return tmp;
}


void CDataThread::setQuit(int q)
{
	quitlock.Lock();
	shouldquit = q;
	quitlock.UnLock();
}

CDataThread::CDataThread(int sscn,int cpsv, int tt, int pp, int rl, int ussl, int actvcon, string usrname, string cip, string pip, string aip,int pport, int aport, int nport, CControlThread *ct,string pcmd)
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
	tmpctx = NULL;
	buffer = new char[config.buffersize];
	sscncmd = sscn;
	controlthread = ct;	
	debugmsg(username,"[datathread] constructor end");
	fp = "NO-FINGERPRINT";
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
		
		Close(datalisten_sock,"datalisten_sock");
		
		Close(dataclient_sock,"dataclient_sock");
		
		Close(datasite_sock,"datasite_sock");		
		
		
		
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
		
		if (tmpctx != NULL) 
		{ 
			debugmsg(username, "[closeconnection free tmpctx");
			SSL_CTX_free(tmpctx); 
			tmpctx = NULL; 
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
	if (!GetSock(datasite_sock))
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
	
	
	
	
	// to store client ip from passive connection
	string clip;
	int clport;
	
	// passive connection - try to listen
	if(!activecon)
	{
		int trie = 1;
		int pasv_error = 0;
		if (!GetSock(datalisten_sock))
		{
			debugmsg(username, "[datathread] unable to create listen sock!",errno);
			return;
		}		
		
		if(!SocketOption(datalisten_sock,SO_REUSEADDR))
		{
			debugmsg(username,"[datathread] setsockopt error!");			
			return;
		}
		while(trie < 4)
		{
			// pick random port here if bind fails?

			debugmsg(username,"[datathread] passive connection - listen");
			
			
			if (!Bind(datalisten_sock, config.listen_ip, newport))
			{
				debugmsg(username,"Unable to bind to port!");
				pasv_error = 1;
			}
			else
			{
				if(!SocketOption(datalisten_sock,SO_REUSEADDR))
				{
					debugmsg(username,"setsockopt error!");
					pasv_error = 1;
				}
				else
				{
					if (listen(datalisten_sock, config.pending) == -1)
					{
						debugmsg(username,"Unable to listen!");
						pasv_error = 1;
					}
				}
			}
			
			trie++;
			if(pasv_error == 1 && trie < 4)
			{
				pasv_error = 0;
				usleep(90000);
			}
			else
			{
				break;
			}
		}
				
		if(pasv_error)
		{
			controlthread->Write(controlthread->client_sock,"425 Can't open passive connection!\r\n",controlthread->clientssl);
			return;
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
		if (!GetSock(dataclient_sock))
		{
			debugmsg(username, "[datathread] unable to create client sock!",errno);		
			
			return;
		}
		
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
	if(sscncmd) debugmsg(username, "[datathread] sscn set to 1 (client method)");
	
	
	// ssl stuff
	if(!config.translate_nosslfxp && ((!config.ssl_forward && usingssl && sslprotp && !relinked) || (relinked && config.ssl_relink)))
	{
		if(!SslConnect(datasite_sock,&sitessl,&sitesslctx,shouldquit,config.data_cipher))
		{
			debugmsg(username, "[datathread] ssl connect to site failed",errno);
			return;
		}
		debugmsg(username,"[datathread] site fingerprint: " + fingerprint(sitessl) + "\r\n");
		// show fingerprint of site
		//controlthread->Write(controlthread->client_sock,"220 Site (SSL connect) datachannel fingerprint: " + fingerprint(sitessl) + "\r\n",controlthread->clientssl);
		
	}
	while(!controlthread->DirectionSet())
	{
		if(getQuit()) return;
	}
	if((cpsvcmd || sscncmd) && !controlthread->dirlisting)
	//if((cpsvcmd || sscncmd) )
	{
		debugmsg(username,"[datathread] ssl client method");
		if((usingssl && relinked && sslprotp) || (!config.ssl_forward && usingssl && sslprotp) || (relinked && config.ssl_relink))
		{
			debugmsg(username,"[datathread] client ssl connect");
			if(!SslConnect(dataclient_sock,&clientssl,&connectsslctx,shouldquit,config.data_cipher))
			{
				debugmsg(username, "[datathread] ssl connect to client failed",errno);
				return;
			}
			fp = fingerprint(clientssl);
			debugmsg(username,"[datathread] client fingerprint: " + fingerprint(clientssl) + "\r\n");
		}
	}
	else 
	{
		debugmsg(username,"[datathread] ssl server method");
		if((usingssl && relinked && sslprotp) || (!config.ssl_forward && usingssl && sslprotp) || (relinked && config.ssl_relink))
		{
			debugmsg(username,"[datathread] client ssl accept");
			if(!SslAccept(dataclient_sock,&clientssl,&clientsslctx,shouldquit,config.data_cipher))
			{
				debugmsg(username, "[datathread] ssl accept from client failed",errno);
				return;
			}
			fp = fingerprint(clientssl);
			debugmsg(username,"[datathread] client fingerprint: " + fingerprint(clientssl) + "\r\n");
		}
	}
	
	if(transfertype == 1)
	{
		debugmsg(username, "[datathread] ascii transfer");
	}
	else
	{
		debugmsg(username, "[datathread] binary transfer");
	}
	
	printsockopt(datasite_sock,"datasite_sock");
	printsockopt(dataclient_sock,"dataclient_sock");
	
	debugmsg(username,"[datathread] wait for direction start");
	while(!controlthread->DirectionSet())
	{
		if(getQuit()) return;
	}
	debugmsg(username,"[datathread] wait for direction end");
	if(controlthread->dirlist_ip == "" && controlthread->dirlisting)
	{
		if(!activecon)
		{
			debugmsg(username,"[datathread] dirlist ip=" + controlthread->dirlist_ip);
			controlthread->dirlist_ip = clip;			
		}
		else
		{
			debugmsg(username,"[datathread] dirlist ip=" + controlthread->dirlist_ip);
			controlthread->dirlist_ip = activeip;			
		}
	}
	
	
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
				if((config.use_ssl_exclude && sslexcludelist.IsInList(username)) || relinked)
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
			if(config.use_fxpiplist)
			{
				// check for ip in fp list
				if(config.fp_new_ip_msg)
				{
					string msg;
					if(fpwhitelist.CheckIp(fp,clip,msg) == 1)
					{
						// fp added, ip not added -> send msg
						controlthread->admin_msg = "site msg " + config.fp_msg_nick + msg + "\r\n";
						debugmsg(username,"[datathread] ADMIN MSG: site msg " + config.fp_msg_nick + msg + "\r\n");
					}
				}

				// first check fingerprint
				if(!fpwhitelist.IsInList(fp) && !adminlist.IsInList(username))
				{
					// check if ip is allowed or user is admin
					if(!whitelist.IsInList(clip) && !adminlist.IsInList(username) )
					{
						if(clip != controlthread->dirlist_ip)
						{
							debugmsg(username,"[datathread] fxp not allowed - clip: " + clip + " dirlistip: " + controlthread->dirlist_ip);
							controlthread->Write(controlthread->client_sock,"427 passive FXP fp: " + fp + " - not allowed!\r\n",controlthread->clientssl);
							return;
						}
					}
				}
			}
			if(config.enforce_tls && config.enforce_tls_fxp && (!sslprotp || !usingssl))
			{
				if((config.use_ssl_exclude && sslexcludelist.IsInList(username)) || relinked)
				{
					// allowed
				}
				else
				{
					controlthread->Write(controlthread->client_sock,"427 Use SSL FXP!\r\n",controlthread->clientssl);
					return;
				}
			}
			if(controlthread->direction == "download")
			{
				if(config.use_fxpfromsite_list && !fxpfromsitelist.IsInList(username))
				{
					controlthread->Write(controlthread->client_sock,"427 passive FXP (dl) not allowed!\r\n",controlthread->clientssl);
					return;
				}
			}
			if(controlthread->direction == "upload")
			{
				if(config.use_fxptosite_list && !fxptositelist.IsInList(username))
				{
					controlthread->Write(controlthread->client_sock,"427 passive FXP (ul) not allowed!\r\n",controlthread->clientssl);
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
				if((config.use_ssl_exclude && sslexcludelist.IsInList(username)) || relinked)
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
			if(config.use_fxpiplist)
			{
				// check for ip in fp list
				if(config.fp_new_ip_msg)
				{
					string msg;
					if(fpwhitelist.CheckIp(fp,activeip,msg) == 1)
					{
						// fp added, ip not added -> send msg
						controlthread->admin_msg = "site msg " + config.fp_msg_nick + msg + "\r\n";
						debugmsg(username,"[datathread] ADMIN MSG: site msg " + config.fp_msg_nick + msg + "\r\n");
					}
				}

				// first check fingerprint
				if(!fpwhitelist.IsInList(fp) && !adminlist.IsInList(username))
				{
					

					// check ip ip is allowed or user is admin
					if(!whitelist.IsInList(activeip) && !adminlist.IsInList(username))
					{
						if(activeip != controlthread->dirlist_ip)
						{
							debugmsg(username,"[datathread] fxp not allowed - clip: " + clip + " dirlistip: " + controlthread->dirlist_ip);
							controlthread->Write(controlthread->client_sock,"427 active FXP fp: " + fp + " - not allowed!\r\n",controlthread->clientssl);
							return;
						}
					}
				}
			}
			if(config.enforce_tls && config.enforce_tls_fxp && (!sslprotp || !usingssl))
			{
				if((config.use_ssl_exclude && sslexcludelist.IsInList(username)) || relinked)
				{
					// allowed
				}
				else
				{
					controlthread->Write(controlthread->client_sock,"427 Use SSL FXP!\r\n",controlthread->clientssl);
					return;
				}
			}
			if(controlthread->direction == "download")
			{
				if(config.use_fxpfromsite_list && !fxpfromsitelist.IsInList(username))
				{
					controlthread->Write(controlthread->client_sock,"427 active FXP (dl) not allowed!\r\n",controlthread->clientssl);
					return;
				}
			}
			if(controlthread->direction == "upload")
			{
				if(config.use_fxptosite_list && !fxptositelist.IsInList(username))
				{
					controlthread->Write(controlthread->client_sock,"427 active FXP (ul) not allowed!\r\n",controlthread->clientssl);
					return;
				}
			}
		}
			
	}
	
	fd_set data_readfds;
	
	
	debugmsg(username,"[datathread] entering dataloop");
	if(config.speed_write)
	{
		if(controlthread->direction == "download")
		{
			while (!getQuit())
			{
				int rc;
				if(!SRead(datasite_sock,buffer,rc,sitessl))
				{					
					break;
				}			
				
				if(!SWrite(dataclient_sock,buffer,rc,clientssl))
				{					
					break;
				}
			}
		}
		else
		{
			while (!getQuit())
			{
				int rc;
				if(!SRead(dataclient_sock,buffer,rc,clientssl))
				{					
					break;
				}			
				
				if(!SWrite(datasite_sock,buffer,rc,sitessl))
				{					
					break;
				}
			}
		}
	}
	else
	{	
		while (!getQuit())
		{
			for(int i=0; i < config.read_write_timeout * 2;i++)
			{
				FD_ZERO(&data_readfds);
				FD_SET(dataclient_sock,&data_readfds);
				FD_SET(datasite_sock,&data_readfds);
				struct timeval tv;
				tv.tv_sec = 0;
				tv.tv_usec = 500000;
					
				int tmpsock;
		
				if (datasite_sock > dataclient_sock)
				{
					tmpsock = datasite_sock;
				}
				else
				{
					tmpsock = dataclient_sock;
				}
				
				if (select(tmpsock+1, &data_readfds, NULL, NULL, &tv) > 1)
				{
					break;
				}
				if (getQuit()) 
				{
					closeconnection();
					return;
				}
			}
			// just to make sure - should not happen
			if(datasite_sock < 0 || dataclient_sock < 0) break;
			
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
			else if (FD_ISSET(dataclient_sock, &data_readfds))
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
			else
			{
				debugmsg(username,"[datathread] fd_isset error",errno);
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
	if(!DataRead(sock ,buffer,nrbytes,ssl,transfertype,usingssl))
	{
		debugmsg(username,"[DataRead] read failed!",errno);
		return 0;
	}
	
	if(sock == dataclient_sock)
	{
		controlthread->localcounter.addrecvd(nrbytes);
		totalcounter.addrecvd(nrbytes);
		daycounter.addrecvd(nrbytes);
		weekcounter.addrecvd(nrbytes);
		monthcounter.addrecvd(nrbytes);
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
		daycounter.addsend(nrbytes);
		weekcounter.addsend(nrbytes);
		monthcounter.addsend(nrbytes);
	}
	else if(sock == datasite_sock)
	{
	}
	return 1;
}


// speed optimized read/write
int CDataThread::SWrite(int sock,char *data,int nrbytes,SSL *ssl)
{
	// no select - using blocking sockets
	int total = 0;
	int bytesleft = nrbytes;
	int rc,len;
	len = nrbytes;
	int count = 0;
	
	while(total < nrbytes)
	{
		if(ssl == NULL)
		{		
			rc = send(sock,data+total,bytesleft,0);
		}
		else
		{
			rc = SSL_write(ssl,data+total,bytesleft);
		}
		if(rc > 0)
		{
			total += rc;
			bytesleft -= rc;
		}
		else if (rc == 0)
		{					
			return 0;
		}
		else
		{
			if(count == config.retry_count) { return 0; } // not more then x retries
			if (ssl != NULL)
			{
				int err = SSL_get_error(ssl,rc);
				
				if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_X509_LOOKUP) 
				{	
					count++; 
					//sleep(1); // wait 1 milisecond
					continue;
				}
				else
				{							
					return 0;
				}
			}
			return 0; 
		}
	}
	return 1;
}

int CDataThread::SRead(int sock ,char *buffer,int &nrbytes,SSL *ssl)
{
	int count = 0;
	while(1)
	{
			int rc;		
			if (ssl == NULL)
			{
				rc = recv(sock,buffer,config.buffersize,0);
			}
			else
			{
				rc = SSL_read(ssl,buffer,config.buffersize);
			}
	  				
			if (rc > 0) 
			{ 
				nrbytes = rc; 
				return 1; 
			}
			else  if(rc == 0)
			{	
				nrbytes=0; 
				return 0; 
			}
			else
			{	
				if(count == config.retry_count) { return 0; } // not more then x retries
				if (ssl != NULL)
				{
					int err = SSL_get_error(ssl,rc);
					
					if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_X509_LOOKUP) 
					{
						count++; 
						//sleep(1); // wait 1 milisecond
						continue;
					}
					else
					{
						return 0;
					}
					
				}
								
				nrbytes=0; 
				return 0; 
			}
		}
  	
		
	return 0;
}
