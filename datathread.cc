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
	debugmsg("SYSTEM","[makedatathread] start");
	CDataThread *pDataThread = (CDataThread*)pData;

	pDataThread->dataloop();
 	pDataThread->closeconnection();
	debugmsg("SYSTEM","[makedatathread] end");
	return NULL;
}

CDataThread::CDataThread(int cpsv, int tt, int pp, int rl, int ussl, int actvcon, string usrname, struct sockaddr_in caddr,  CControlThread *ct)
{
	debugmsg(username,"[datathread] constructor start");
	username = usrname;
	usingssl = ussl;
	activecon = actvcon;

	activeport = -1;
	passiveport = -1;
	siteport = -1;
	activeip = "";
	relinked = rl;
	transfertype = tt;
	sslprotp = pp;
	datalisten_sock = -1;
	datasite_sock = -1;
	dataclient_sock = -1;
	
	cpsvcmd = cpsv;
	client_addr = caddr;
	//using ip from idnt cmd when running with entrys
	if(ct->using_entry)
	{
		client_addr.sin_addr.s_addr = inet_addr(ct->client_ip.c_str());
	}
	buffer = new char[config.buffersize];
	controlthread = ct;
	globals_lock.Lock();
	nr_threads++;
	globals_lock.UnLock();
	debugmsg(username,"[datathread] constructor end");
}

CDataThread::~CDataThread()
{
	debugmsg(username,"[datathread] destructor start");
				
	debugmsg(username,"[datathread] delete buffer");
	delete [] buffer;
	globals_lock.Lock();
	nr_threads--;
	globals_lock.UnLock();
	debugmsg(username,"[datathread] destructor end");
}

void CDataThread::closeconnection(void)
{

	
	if (datasite_sock > 0) 
	{
		debugmsg(username,"[datathread] close site sock"); 
		close(datasite_sock); 
		datasite_sock = -1;
	}
	
	if (dataclient_sock > 0) 
	{
		debugmsg(username,"[datathread] close client sock"); 
		close(dataclient_sock); 
		dataclient_sock = -1;
	}
	
	if (datalisten_sock > 0) 
	{ 
		debugmsg(username,"[datathread] close listen sock");		
		close(datalisten_sock); 
		datalisten_sock = -1;
	}
	

	
}

void CDataThread::getactive_data(string portcmd)
{
	debugmsg(username,"[getactive_data] start");
	debugmsg(username,"[getactive_data] " + portcmd);
	unsigned int startpos;
		
	startpos = portcmd.find(" ",0);
	if (startpos == string::npos) { return; }
	portcmd = portcmd.substr(startpos+1,portcmd.length());
	startpos = portcmd.find(",",0);
	if (startpos == string::npos) { return; }
	portcmd.replace(startpos,1,".");
	startpos = portcmd.find(",",0);
	if (startpos == string::npos) { return; }
	portcmd.replace(startpos,1,".");
	startpos = portcmd.find(",",0);
	if (startpos == string::npos) { return; }
	portcmd.replace(startpos,1,".");
	startpos = portcmd.find(",",0);
	if (startpos == string::npos) { return; }
	activeip = portcmd.substr(0,startpos);
	debugmsg(username,"[getactive_data] " + activeip);
	string tmpport;
	tmpport = portcmd.substr(startpos+1,portcmd.length());
	startpos = tmpport.find(",",0);
	if (startpos == string::npos) { return; }
	string p1,p2;
	p1 = tmpport.substr(0,startpos);
	p2 = tmpport.substr(startpos+1,tmpport.length()-1);
	activeport = 256 * atoi(p1.c_str()) + atoi(p2.c_str());
	stringstream ss;
	ss << activeport;
	debugmsg(username,"[getactive_data] " + ss.str());
	debugmsg(username,"[getactive_data] end");
}


string CDataThread::getpassive_data(string passivecmd)
{
	// add some more checks here??
	debugmsg(username,"[getpassive_data] start");
	debugmsg(username,"[getpassive_data] " + passivecmd);
	unsigned int startpos,endpos;
	startpos = passivecmd.find("(",0);
	endpos = passivecmd.find(")",0);
	if (startpos == string::npos || endpos == string::npos)
	{
		debugmsg(username,"[getpassive_data] parse error");
		return "";
	}
	
	// split passive mode string
	string tmp = passivecmd.substr(1+startpos,endpos-startpos-1);

	endpos = tmp.find(",",0);
	string ip1 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	endpos = tmp.find(",",0);
	string ip2 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	endpos = tmp.find(",",0);
	string ip3 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	endpos = tmp.find(",",0);
	string ip4 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	endpos = tmp.find(",",0);
	string port1 = tmp.substr(0,endpos);

	string port2 = tmp.substr(endpos+1,tmp.length());

	siteport = (atoi(port1.c_str()) * 256 + atoi(port2.c_str()));
	passiveport = siteport + config.add_to_passive_port;

	string newpassivecmd = "227 Entering Passive Mode (";
	
	if (config.use_port_range && relinked)
	{
		passiveport = random_range(config.port_range_start,config.port_range_end);
	}
	string tmpip;
	if (config.listen_ip != "") 
	{ 
		tmpip = config.listen_ip; 
	}
	else
	{
		debugmsg(username,"[getpassive_data] try to get current ip start");
		struct ifreq ifa;
		struct sockaddr_in *i;
		memset(&ifa,0,sizeof( struct ifreq ) );
		strcpy(ifa.ifr_name,config.listen_interface.c_str());
		
		int rc = ioctl(listen_sock, SIOCGIFADDR, &ifa);
		
		if(rc != -1)
		{
			i = (struct sockaddr_in*)&ifa.ifr_addr;
			tmpip = inet_ntoa(i->sin_addr);
		}
		else
		{
			tmpip = "0.0.0.0";
			debugmsg(username,"[getpassive_data] ioctl error",errno);
		}
		debugmsg(username,"[getpassive_data] try to get current ip end");
	}
	
	debugmsg(username,"[getpassive_data] ip: " + tmpip);
	startpos = tmpip.find(".",0);
	tmpip.replace(startpos,1,",");
	startpos = tmpip.find(".",0);
	tmpip.replace(startpos,1,",");
	startpos = tmpip.find(".",0);
	tmpip.replace(startpos,1,",");


	newpassivecmd += tmpip + ",";
	stringstream ss;
	ss << (int)(passiveport / 256) << "," << (passiveport % 256) << ")\r\n";

	newpassivecmd += ss.str();
	
	debugmsg(username,"[getpassive_data] passive cmd: " + newpassivecmd);
	
	debugmsg(username,"[getpassive_data] end");
	return newpassivecmd;
}


int CDataThread::data_write(int sock,char *data,int nrbytes)
{
	
	int total = 0;
	int bytesleft = nrbytes;
	int rc,len;
	len = nrbytes;
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(sock,&writefds);
	struct timeval tv;
	while(total < nrbytes)
	{
		tv.tv_sec = config.read_write_timeout;
		tv.tv_usec = 0;
		if (select(sock+1, NULL, &writefds, NULL, NULL) < 1)
		{
			debugmsg(username,"[data_write] select error!",errno);
			return 0;
		}
		if (FD_ISSET(sock, &writefds))
		{			
			rc = send(sock,data+total,bytesleft,0);
		}
		else
		{
			debugmsg(username,"[data_write] error!",errno);
			return 0;
		}
		if (rc <= 0) 
		{
			if (rc == 0)
			{
				debugmsg(username,"[data_write] connection closed",errno);
			}
			debugmsg(username,"[data_write] error!");  
			return 0; 
		}
		total += rc;
		bytesleft -= rc;
	}
	return 1;	
	
}

int CDataThread::data_read(int sock ,char *buffer,int &nrbytes)
{
	
		if ((transfertype == 1) && config.ssl_ascii_cache  && usingssl && !relinked)
		{			
			int sslasciiread = 0;
      int rc = 1;
      while (rc > 0 && (sslasciiread < config.buffersize))
      {  
      	debugmsg(username,"[data_read] ssl_ascii_cache mode");    	
      	fd_set readfds;
      	FD_ZERO(&readfds);
				FD_SET(sock,&readfds);
				struct timeval tv;
				tv.tv_sec = 0;
				tv.tv_usec = 5;
				if (select(sock+1, &readfds, NULL, NULL, &tv) < 1)
				{
					break;
				}
				if (FD_ISSET(sock, &readfds))
				{
      		rc = recv(sock,buffer + sslasciiread,config.buffersize - sslasciiread,0);
      	}
      	else
      	{
      		break;
      	}
      	
      	if (rc > 0) 
        { 
           sslasciiread += rc; 
                
        }
        else
        {
        	break;
        }
			}
			
      if (sslasciiread > 0) { nrbytes = sslasciiread; return 1; }
      else if (sslasciiread == 0) { nrbytes=0; return 0; }
      
      
		}
		else
		{
			
    	int rc = recv(sock,buffer,config.buffersize,0);
    				
			if (rc > 0) { nrbytes = rc; return 1; }
			else 
			{
				if (rc == 0)
				{
					debugmsg(username,"[data_read] connection closed",errno);
				}
				debugmsg(username,"[data_read] error!"); 
				nrbytes=0; 
				return 0; 
			}
			
		}

	
	
	return 0;
}

void CDataThread::dataloop(void)
{
	debugmsg(username,"[datathread] dataloop start");

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
			controlthread->control_write(controlthread->client_sock,"427 Could not bind to dataport!\r\n",controlthread->clientssl);
				
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
		struct sockaddr_in connect_addr;
		connect_addr = GetIp(config.connect_ip,0);
		
		bind(datasite_sock,(struct sockaddr *)&connect_addr, sizeof(struct sockaddr));
	}
	setnonblocking(datasite_sock);
	if(!Connect(datasite_sock,datasite_addr,config.connect_timeout,0))
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
				
		if(!Accept(datalisten_sock,dataclient_sock,datalisten_addr,config.connect_timeout,0))
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
		if(!Connect(dataclient_sock,active_addr,config.connect_timeout,0))
		{
			//controlthread->control_write(controlthread->client_sock,"427 Connect Error/Timeout!\r\n",controlthread->clientssl);
			debugmsg(username,"[datathread] fxp connect failed",errno);			
			return;
		}
	
	}

	
	fd_set data_readfds;
	
	setblocking(datasite_sock);
	setblocking(dataclient_sock);
	
	debugmsg(username,"[datathread] entering dataloop");
	while (1)
	{

		FD_ZERO(&data_readfds);
		FD_SET(dataclient_sock,&data_readfds);
		FD_SET(datasite_sock,&data_readfds);


		int tmpsock;


		if (datasite_sock > dataclient_sock)
		{
			tmpsock = datasite_sock;
		}
		else
		{
			tmpsock = dataclient_sock;
		}
		
		if (select(tmpsock+1, &data_readfds, NULL, NULL, NULL) < 1)
		{
			debugmsg(username,"[datathread] read timeout",errno);
			break;
		}

		// read from site - send to client
		if (FD_ISSET(datasite_sock, &data_readfds))
		{


			memset(buffer,'\0',1);
	
				int rc;
				if(!data_read(datasite_sock,buffer,rc))
				{
					debugmsg(username,"[datathread] read from site failed");
					break;
				}

				if(!data_write(dataclient_sock,buffer,rc))
				{
					debugmsg(username,"[datathread] send to client failed");
					break;
				}
				else
				{
					controlthread->localcounter.addsend(rc);
					totalcounter.addsend(rc);
					
				}

		}
		// read from client - send to site
		if (FD_ISSET(dataclient_sock, &data_readfds))
		{

			memset(buffer,'\0',1);

				int rc;
				if(!data_read(dataclient_sock,buffer,rc))
				{
					debugmsg(username,"[datathread] read from client failed");
					break;
				}
				else
				{
					controlthread->localcounter.addrecvd(rc);
					totalcounter.addrecvd(rc);
					
				}

				if(!data_write(datasite_sock,buffer,rc))
				{
					debugmsg(username,"[datathread] send to site failed");
					break;
				}

		}

	}

	debugmsg(username,"[datathread] end");
	
	return;

}



