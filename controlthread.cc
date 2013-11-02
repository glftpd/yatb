#include "controlthread.h"
#include "datathread.h"
#include "tools.h"
#include "config.h"
#include "stringlist.h"
#include "lock.h"
#include "tls.h"


// c wrapper for creating main connection thread
void *makethread(void* pData)
{
 debugmsg("-SYSTEM-","[makethread] start");
 CControlThread *pConnection = (CControlThread*)pData;

 pConnection->mainloop();
 debugmsg("-SYSTEM-","[makethread] delete pConnection");
 delete pConnection;
 debugmsg("-SYSTEM-","[makethread] end");
 return NULL;
}



// constructor
CControlThread::CControlThread(int fd,struct sockaddr_in addr)
{
	debugmsg("-SYSTEM-","[konstruktor] start");
	client_sock = fd;
	client_addr = addr;
	site_sock = -1;
	sitessl = NULL;
	clientssl = NULL;
	datathread = NULL;	
	sitesslctx = NULL;
	gotfirstcmd = 0;
	gotfirstreply = 0;
	clientsslbits = 0;
	sitesslbits = 0;
	gotusercmd = 0;
	gotpasscmd = 0;
	gotportcmd = 0;
	gotwelcomemsg = 0;
	relinked = 0;
	gotpasvcmd = 0;
	sslprotp = 0;
	activecon = 0;
	transfertype = 1; // set to ascii mode
	usingssl = 0;
	cpsvcmd = 0;
	shouldquit = 0;
	username = "-EMPTY-";
	
	if (config.entry_list == "")
	{
		using_entry = 0;
	}
	else
	{
		using_entry = 1;
	}
	globals_lock.Lock();
	nr_threads++;
	globals_lock.UnLock();
	debugmsg("-SYSTEM-","[konstruktor] end");
	client_ip = "";
}

// destructor
CControlThread::~CControlThread()
{
	debugmsg(username,"[controlthread] destructor start");
	deletedatathread();
	
	list_lock.Lock();
	conlist.remove(this);
	list_lock.UnLock();
	
	if (usingssl)
	{
		
		if (sitessl != NULL) 
		{
			debugmsg(username,"[controlthread] close site ssl"); 
			SSL_shutdown(sitessl);
						
		}
		
		if (clientssl != NULL) 
		{
			debugmsg(username,"[controlthread] close client ssl"); 
			SSL_shutdown(clientssl); 	
		
		}
	}
	
	if (client_sock > 0) 
	{ 
		debugmsg(username, "[controlthread] close client sock");
		close(client_sock); 
	}
	
	if (site_sock > 0) 
	{ 
		debugmsg(username, "[controlthread] close site sock");
		close(site_sock); 
	}
	
	if (usingssl)
	{			
		
		if (sitessl != NULL) 
		{ 
			debugmsg(username, "[controlthread] free site ssl");
			SSL_free(sitessl); 
			sitessl = NULL; 
		}
		
		if (clientssl != NULL) 
		{ 
			debugmsg(username, "[controlthread] free client ssl");
			SSL_free(clientssl); 
			clientssl = NULL; 
		}
		
		if (sitesslctx != NULL) 
		{ 
			debugmsg(username, "[controlthread] free sitesslctx");
			SSL_CTX_free(sitesslctx); 
			sitesslctx = NULL; 
		}
		
		debugmsg(username, "[controlthread] free ssl error queue");
		ERR_remove_state(0);
	}
	globals_lock.Lock();
	nr_threads--;
	globals_lock.UnLock();	
	
	debugmsg(username,"[controlthread] destructor end");

}


void CControlThread::deletedatathread(void)
{
	if (datathread != NULL) 
	{	
		debugmsg(username,"[deletedatathread] set shouldquit=1");
		datathread->shouldquit = 1;
		
		debugmsg(username,"[deletedatathread] join datathread");
		//datathread->closeconnection();
		
		if(pthread_join(datathread->tid,NULL) != 0)
		{
			debugmsg(username,"[deletedatathread] error joining thread",errno);
		}
		debugmsg(username,"[deletedatathread] delete datathread");
		delete datathread; 
		datathread = NULL; 
	}	
}

// send string
int CControlThread::control_write(int sock,string s,SSL *sslcon)
{	
	debugmsg(username,"[control_write] start");
	writelock.Lock();
	stringstream ss;
	ss << s.length();
	debugmsg(username,"[control_write] size: " + ss.str());
	if (sock == site_sock) { debugmsg(username, "[control_write] to site:\n" + s); }
	if (sock == client_sock) { debugmsg(username, "[control_write] to client:\n" + s); }
	fd_set writefds;
	struct timeval tv;

	int maxsize = config.buffersize; // max size in bytes of packet
	int total = 0;
	int bytesleft = s.length();
	int blocksize;
	if (bytesleft > maxsize)
	{
		blocksize = maxsize;
	}
	else
	{
		blocksize = bytesleft;
	}
	int n,len;
	len = s.length();
	while(total < len)
	{
		tv.tv_sec = config.read_write_timeout;
		tv.tv_usec = 0;
		FD_ZERO(&writefds);
		FD_SET(sock,&writefds);
		if (select(sock+1,NULL,&writefds,NULL,&tv) == -1)
		{
			debugmsg(username, "[control_write] select error!",errno);
			writelock.UnLock();
			return 0;
		}
		if (FD_ISSET(sock,&writefds))
		{
			if (!sslcon)
			{
				n = send(sock,s.c_str()+total,blocksize,0);
			}
			else
			{
				n = SSL_write(sslcon, s.c_str()+total, blocksize);
			}
		}
		if (n < 0)
		{
			if (sslcon != NULL)
			{
				int err = SSL_get_error(sslcon,n);
				
				if (err == SSL_ERROR_WANT_READ) { continue; }
				if (err == SSL_ERROR_WANT_WRITE) { continue; }
				if (err == SSL_ERROR_WANT_X509_LOOKUP) { continue; }				
				
			}
			stringstream ss;
			ss << "n: " << n << " bytesleft: " << bytesleft;
			debugmsg(username,"[control_write] error: " + ss.str());
			break;
		}
		total += n;
		bytesleft -= n;
		if (bytesleft > maxsize)
		{
			blocksize = maxsize;
		}
		else
		{
			blocksize = bytesleft;
		}
	}
	if (sock == client_sock) 
	{		
		localcounter.addsend(total);
		totalcounter.addsend(total);
				
	}

	if (bytesleft == 0)
	{
		debugmsg(username,"[control_write] end");
		writelock.UnLock();
		return 1;
	}
	else
	{
		debugmsg(username,"[control_write] send error");
		writelock.UnLock();
		return 0;
	}
}

// read string
int CControlThread::control_read(int sock,SSL *sslcon,string &str)
{
	debugmsg(username,"[control_read] start");
	string tmp = "";
	fd_set readfds;
	struct timeval tv;
	int rc;
	char *buffer;
	buffer = new char[config.buffersize];
	
	while(1)
	{		
		for (int i=0;i<config.buffersize;i++) { buffer[i] = 0; }
		tv.tv_sec = 0;
		tv.tv_usec = 500;
		FD_ZERO(&readfds);
		FD_SET(sock,&readfds);
		if (select(sock+1,&readfds,NULL,NULL,&tv) == -1)
		{
			debugmsg(username, "[control_read] nothing more to read",errno);
			str = tmp;
			if (sock == client_sock) 
			{
				localcounter.addrecvd(str.length());
				totalcounter.addrecvd(str.length());
				
			}
			stringstream ss;
			ss << str.length();
			debugmsg(username,"[control_read] size: " + ss.str());
			debugmsg(username,"[control_read] end");
			delete [] buffer;
			return 1;
		}
		if (FD_ISSET(sock,&readfds))
		{
			if (sslcon == NULL)
			{
				rc = recv(sock,buffer,config.buffersize,0);
			}
			else
			{
				rc = SSL_read(sslcon,buffer,config.buffersize);
			}
			if (rc == 0)
			{
				if (sock == client_sock)
				{
					debugmsg(username, "[control_read] client closed connection!",errno);
				}
				if (sock == site_sock)
				{
					debugmsg(username, "[control_read] site closed connection!",errno);
				}
				debugmsg(username,"[control_read] end");
				delete [] buffer;
				return 0;
			}
			else if (rc < 0)
			{
				if (sslcon != NULL)
				{
					int err = SSL_get_error(sslcon,rc);
					
					if (err == SSL_ERROR_WANT_READ) { continue; }
					if (err == SSL_ERROR_WANT_WRITE) { continue; }
					if (err == SSL_ERROR_WANT_X509_LOOKUP) { continue; }					
					
				}
				debugmsg(username, "[control_read] read error!");
				debugmsg(username,"[control_read] end");
				delete [] buffer;
				return 0;
			}
			else
			{				
				char *tmpstr;
				tmpstr = new char[rc+1];
				memcpy(tmpstr,buffer,rc);
				tmpstr[rc] = '\0';
				
				tmp += tmpstr;
				
				delete [] tmpstr;
				if (rc < config.buffersize)
				{
					// reached end of line?
					if (tmp[tmp.length() - 1] == '\n')
					{		
						// fix missing \r's
						correctReply(tmp);		
						str = tmp;		
						delete [] buffer;
						return 1;		
													
					}
				}
			}
		}
	}
}



int CControlThread::tryrelink(int state)
{
	debugmsg(username,"[relink] start");
	if (site_sock > -1) { close(site_sock); }
	if (sitesslctx != NULL) { SSL_CTX_free(sitesslctx); sitesslctx = NULL; }
	if (sitessl != NULL) { SSL_free(sitessl); sitessl = NULL; }
	
	

	if ((site_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		debugmsg(username, "[relink] could not create socket",errno);
		return 0;
	}
	
	setnonblocking(site_sock);
	
	site_addr = GetIp(config.relink_ip,config.relink_port);
	
	
	if (config.listen_ip != "")
	{
		struct sockaddr_in connect_addr;
		connect_addr = GetIp(config.listen_ip,0);
		
		if(bind(site_sock,(struct sockaddr *)&connect_addr, sizeof(struct sockaddr)) != 0)
		{
			debugmsg(username,"[controlthread] connect ip - could not bind",errno);
			return 0;
		}
	}
	
	if(!Connect(site_sock,site_addr,config.connect_timeout,shouldquit))
	{
		debugmsg(username, "[relink] could not connect to relinksite!",errno);
		if(config.showconnectfailmsg) { control_write(client_sock,"427 Login failed!\r\n",clientssl); }
		return 0;
	}
	
	string s;
	if(! control_read(site_sock,NULL,s))
	{
		return 0;
	}
	debugmsg(username,"[relink] " +s);
	if(!control_write(site_sock,"USER " + config.relink_user + "\r\n",NULL))
	{
		return 0;
	}
	if(!control_read(site_sock,NULL,s))
	{
		return 0;
	}
	debugmsg(username,"[relink] " + s);
	if(!control_write(site_sock,"PASS " + config.relink_pass + "\r\n",NULL))
	{
		return 0;
	}
	if(!control_read(site_sock,NULL,s))
	{
		return 0;
	}
	debugmsg(username,"[relink] " + s);

	if (state)
	{
		fd_set readfds;
		if (!control_write(client_sock,"331 Password required.\r\n",clientssl))
		{
			return 0;
		}
		while (1)
		{
			FD_ZERO(&readfds);
			FD_SET(client_sock,&readfds);
			if (select(client_sock+1, &readfds, NULL, NULL, NULL) <= 0)
			{
				return 0;
			}
			if (FD_ISSET(client_sock,&readfds))
			{
				string tmp;
				if(!control_read(client_sock,clientssl,tmp))
				{
					return 0;
				}
				if (upper(tmp,5).find("PASS",0) != string::npos)
				{
					break;
				}
				else
				{
					if (!control_write(client_sock,"500 : Command not understood.\r\n",clientssl))
					{
						return 0;
					}
				}
			}
		}
	}
	if (!control_write(client_sock,s,clientssl))
	{
		return 0;
	}
	relinked = 1;

	gotusercmd = 1;
	gotpasscmd = 1;
	gotwelcomemsg = 1;
	debugmsg(username,"[relink] done");
	return 1;
}


int CControlThread::trytls(void)
{
	debugmsg(username,"[trytls] trying tls connection");
		
	setblocking(client_sock);
	setblocking(site_sock);

	sitesslctx = SSL_CTX_new(TLSv1_client_method());
	SSL_CTX_set_options(sitesslctx,SSL_OP_ALL);
	SSL_CTX_set_mode(sitesslctx,SSL_MODE_AUTO_RETRY);
	SSL_CTX_set_session_cache_mode(sitesslctx,SSL_SESS_CACHE_OFF);
	if (sitesslctx == NULL)
	{
		debugmsg(username, "[trytls] sitesslctx failed!");
		return 0;
	}


	sitessl = SSL_new(sitesslctx);
	if (sitessl == NULL)
	{
		debugmsg(username, "[trytls] site ssl failed!");
		return 0;
	}
	SSL_set_fd(sitessl,site_sock);
	debugmsg(username,"[trytls] try to connect to site..");
	if (SSL_connect(sitessl))
	{
		SSL_get_cipher_bits(sitessl,&sitesslbits);

		// get reply

	}
	else
	{
		debugmsg(username, "[trytls] TLS Connection failed!");
		return 0;
	}




	clientssl = SSL_new(clientsslctx);
	if (clientssl == NULL)
	{
		debugmsg(username, "[trytls] clientssl failed!");
		return 0;
	}

	if (!SSL_set_fd(clientssl,client_sock))
	{
		debugmsg(username,"[trytls] " +  (string)ERR_error_string(ERR_get_error(), NULL));
		return 0;
	}
	
	
	
	debugmsg(username,"[trytls] try ssl accept");
	int err;
	
	if( (err = SSL_accept(clientssl)) <= 0)
	{
		debugmsg(username, "[trytls] accept failed!");
		debugmsg(username,"[trytls] " +  (string)ERR_error_string(ERR_get_error(), NULL));
		
		return 0;
	}
	
	
	setnonblocking(site_sock);
	setnonblocking(client_sock);
	
	debugmsg(username, "[trytls] end trytls");
	usingssl = 1;
	return 1;
}



void CControlThread::mainloop(void)
{
	debugmsg(username,"[controlthread] start");
	
	fd_set readfds;
	if (using_entry)
	{
		debugmsg(username,"[controlthread] using entry_list");
		debugmsg(username,"[controlthread] entry_list: " + entrylist.GetList());
		// only allow connects from entry bnc
		string cip = inet_ntoa(client_addr.sin_addr);
		if(!entrylist.IsInList(cip))
		{
			debugmsg(username,"[controlthread] connect ip: " + cip);
			debugmsg(username,"[controlthread] connect ip not in entry_list");
			return;
		}
	}
		
	connect_time = time(NULL);

	
	string ident_reply;
	string ident_user;
	ident_reply = "ERROR";
	ident_user = "*";
	username = "-BEFORE-IDENT-";
	// try to get ident reply
	if (config.use_ident)
	{
		debugmsg(username,"[controlthread] try ident");
		int ident_sock;
		ident_sock = socket(AF_INET,SOCK_STREAM,0);
		if (ident_sock != -1)
		{

			struct sockaddr_in ident_addr;
						
			ident_addr.sin_family = AF_INET;
			ident_addr.sin_port = htons(113);
			ident_addr.sin_addr.s_addr = client_addr.sin_addr.s_addr;
			memset(&(ident_addr.sin_zero), '\0', 8);
			
			setnonblocking(ident_sock);
			
			if (config.listen_ip != "")
			{
				debugmsg(username,"[controlthread] try to set connect ip for ident");
				struct sockaddr_in connect_addr;				
				connect_addr = GetIp(config.listen_ip,0);
								
				if(bind(ident_sock,(struct sockaddr *)&connect_addr, sizeof(struct sockaddr)) != 0)
				{
					debugmsg(username,"[controlthread] ident connect ip - could not bind",errno);
					return;
				}
			}
			
			if (Connect(ident_sock,ident_addr,config.ident_timeout,shouldquit) )
			{				
				debugmsg(username,"[controlthread] try to read ident reply");
				FD_ZERO(&readfds);
				FD_SET(ident_sock, &readfds);
				stringstream ss;
				ss << ntohs(client_addr.sin_port) << " , " << config.listen_port << "\r\n";
				if (!control_write(ident_sock,ss.str(),NULL))
				{
					if(config.enforce_ident)
					{
						
						
						return;
					}
				}
				struct timeval tv;
				tv.tv_sec = config.ident_timeout;
				tv.tv_usec = 0;
				
				if (select(ident_sock+1, &readfds, NULL, NULL, &tv) <= 0)
				{
					debugmsg(username, "[controlthread] ident select error!",errno);

				}
				else
				{
					if (FD_ISSET(ident_sock, &readfds))
					{

						if(!control_read(ident_sock,NULL,ident_reply))
						{
							if(config.enforce_ident)
							{					
								
								return;
							}
						}
						debugmsg(username,"[controlthread] ident: " + ident_reply);
					}
					
				}
				close(ident_sock);
			}
			else
			{
				debugmsg(username,"[controlthread] could not connect to ident port");
				if (config.enforce_ident)
				{
					
					
					return;
				}
			}
		}
		else
		{
			debugmsg(username,"[controlthread] unable to create ident sock!");
			if (config.enforce_ident)
			{
				
				
				return;
			}
		}
	}

	//parse ident reply
	if (config.use_ident)
	{
		if (upper(ident_reply,ident_reply.length()).find("ERROR",0) == string::npos)
		{
			// no error - get username
			string idnt;
			idnt=ident_reply.substr(ident_reply.rfind(":",ident_reply.length())+1);
			idnt=idnt.substr(0,idnt.find("\r"));
			ident_user = trim(idnt);
			if (ident_user[ident_user.length() - 1] == '\n') { ident_user = ident_user.substr(0,ident_user.length() - 1); }
		}
		else
		{
			ident_user = "*";
			if (config.enforce_ident)
			{				
				return;
			}
			
		}
	}

	username = "-AFTER-IDENT-";

	
	if ((site_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		debugmsg(username, "[controlthread] unable to create site sock!",errno);		
		
		return;
	}

	// try to connect to site
	site_addr = GetIp(config.site_ip,config.site_port);
	
	if (config.connect_ip != "")
	{
		debugmsg(username,"[controlthread] try to set connect ip for site connect");
		struct sockaddr_in connect_addr;
		connect_addr = GetIp(config.connect_ip,0);
		
		
		if(bind(site_sock,(struct sockaddr *)&connect_addr, sizeof(struct sockaddr)) != 0)
		{
			debugmsg(username,"[controlthread] connect ip - could not bind",errno);
			return;
		}
	}
	
	setnonblocking(site_sock);
	setnonblocking(client_sock);
	
	
	debugmsg(username,"[controlthread] try to connect to site");
	
	
	
	if(!Connect(site_sock,site_addr,config.connect_timeout,shouldquit))
	{
		if(config.showconnectfailmsg) { control_write(client_sock,config.connectfailmsg + "\r\n",clientssl); }
		debugmsg(username, "[controlthread] could not connect to site!",errno);
		return;
	}
	

	if (config.fake_server_string)
	{
		if (!control_write(client_sock,config.server_string + "\r\n",clientssl))
		{
			
			return;
		}
	}
	
	//set client/site sock to keepalive
	int yes = 1;
	
	if (setsockopt(client_sock,SOL_SOCKET,SO_KEEPALIVE,&yes,sizeof(int)) == -1)
	{
		debugmsg(username, "[controlthread] client setsockopt error!",errno);

	}	
	
	if (setsockopt(site_sock,SOL_SOCKET,SO_KEEPALIVE,&yes,sizeof(int)) == -1)
	{
		debugmsg(username, "[controlthread] site setsockopt error!",errno);

	}
	
	stringstream idnt_cmd;
	idnt_cmd << "IDNT " << ident_user << "@" << inet_ntoa(client_addr.sin_addr) << ":" << inet_ntoa(client_addr.sin_addr) << "\r\n";
	debugmsg("-SYSTEM-","IDNT cmd: " + idnt_cmd.str());
	if (config.use_ident)
	{
		if (!control_write(site_sock,idnt_cmd.str(),sitessl))
		{			
			
			return;
		}
	}
	if(!using_entry)
	{
		debugmsg(username,"[controlthread] try to get welcome msg");
		string serverwelcomemsg,tmp;
		do
		{
			if(!control_read(site_sock,sitessl,tmp))
			{		
				
				return;
			}	
			serverwelcomemsg += tmp;
		}
		while (!IsEndline(tmp));
		
		if (!config.fake_server_string )
		{
			if (!control_write(client_sock,serverwelcomemsg,clientssl))
			{
				
				
				return;
			}
		}
	}

	username = "-BEFORE-MAINLOOP-";

	// main loop
	debugmsg(username,"[controlthread] entering mainloop");
	while (1)
	{
		FD_ZERO(&readfds);
		FD_SET(site_sock, &readfds);
		FD_SET(client_sock, &readfds);
		int tmpsock;
		if (site_sock > client_sock)
		{
			tmpsock = site_sock;
		}
		else
		{
			tmpsock = client_sock;
		}
		
		debugmsg(username,"[controlthread] before select");
		
		if (select(tmpsock+1, &readfds, NULL, NULL, NULL) <= 0)
		{
			debugmsg(username, "[controlthread] select error!",errno);
			return;
		}
		
		// read from site
		if (FD_ISSET(site_sock, &readfds))
		{
			debugmsg(username,"[controlthread] read from site");
			string s;
			if(!control_read(site_sock,sitessl,s))
			{
				
				
				return;
			}

				if (gotfirstcmd && !gotusercmd)
				{

					if (upper(s,s.length()).find("AUTH TLS SUCCESSFUL",0) != string::npos)
					{
						debugmsg(username,"[controlthread] got tls reply");
						if (!control_write(client_sock,s,clientssl))
						{							
							return;
						}
						if (!trytls())
						{							
							return;
						}
					}
					else if (upper(s,s.length()).find("AUTH SSL SUCCESSFUL",0) != string::npos)
					{
						debugmsg(username,"[controlthread] got ssl reply");
						sslprotp = 1;
						if (!control_write(client_sock,s,clientssl))
						{							
							return;
						}
						if (!trytls())
						{							
							return;
						}
					}
					else
					{
						if (!control_write(client_sock,s,clientssl))
						{							
							return;
						}
					}
				}
				else if (gotusercmd && !gotpasscmd)
				{

					debugmsg(username,"[controlthread] after user cmd");
					// wrong user?
					if (config.enforce_tls)
					{
						if (!usingssl && !config.use_ssl_exclude)
						{
							if (!control_write(client_sock,"427 Use AUTH TLS!\r\n",clientssl))
							{
								
								
								return;
							}
							
							
							return;
						}
					}
					if (IsEndline(s) && upper(s,s.length()).find(upper(config.user_access_denied,config.user_access_denied.length()),0) != string::npos)
					{
						debugmsg(username,"[controlthread] reply: " + s);
						debugmsg(username,"[controlthread] user incorrect");

						if (config.trytorelink)
						{
							debugmsg(username,"[controlthread] trying to relink");
							if (!tryrelink(1))
							{
								
								
								return;
							}
						}
						else
						{
							if (!control_write(client_sock,s,clientssl))
							{
								
								
								return;
							}
							
							
							return;
						}
					}
					else
					{
						if (!control_write(client_sock,s,clientssl))
						{
							
							
							return;
						}
					}
				}
				else if (gotusercmd && gotpasscmd && !gotwelcomemsg)
				{

					debugmsg(username,"[controlthread] after user & pass");
					
					if (IsEndline(s) && upper(s,s.length()).find(upper(config.user_login_success,config.user_login_success.length()),0) != string::npos)
					{
						gotwelcomemsg++;
						debugmsg(username,"[controlthread] login successfull");
						nr_logins++;
						if (!control_write(client_sock,s,clientssl))
						{
							
							
							return;
						}
						if(config.enforce_tls && !usingssl && config.use_ssl_exclude && !sslexcludelist.IsInList(username))
						{
							if (!control_write(client_sock,"427 Use AUTH TLS!\r\n",clientssl))
							{
								
								
								return;
							}
							return;
						}
					}
					else if (IsEndline(s) && upper(s,s.length()).find(upper(config.site_full,config.site_full.length()),0) != string::npos)
					{
						gotwelcomemsg++;
						debugmsg(username,"[controlthread] site full");
						if(!control_write(client_sock,s,clientssl))
						{								
							return;
						}							
						return;
					}
					else if (IsEndline(s) && upper(s,s.length()).find(upper(config.site_closed,config.site_closed.length()),0) != string::npos)
					{
						gotwelcomemsg++;
						debugmsg(username,"[controlthread] site closed");
						if(!control_write(client_sock,s,clientssl))
						{								
							return;
						}							
						return;
					}
					else
					{
						if (IsEndline(s))
						{
							gotwelcomemsg++;
							debugmsg(username,"[controlthread] login incorrect");
	
							if (config.trytorelink)
							{
								debugmsg(username,"[controlthread] trying to relink");
								if (!tryrelink(0))
								{
									
									
									return;
								}
							}
							else
							{
								if(!control_write(client_sock,s,clientssl))
								{								
									return;
								}							
								
							}
						}
						else
						{
							if(!control_write(client_sock,s,clientssl))
							{								
								return;
							}				
						}

					}
				}
				else if (gotpasvcmd && config.bounce_data_con && !gotportcmd)
				{			
					
					debugmsg(username,"[controlthread] create datathread");
					deletedatathread();
					
					if (config.thread_limit == 0 || nr_threads < config.thread_limit)
					{
						datathread = new CDataThread(cpsvcmd, transfertype, sslprotp, relinked, usingssl, activecon, username, client_addr,  this);
						debugmsg(username,"[controlthread] call datathread getactive_data");
						string pcmd = datathread->getpassive_data(s);
						if (!control_write(client_sock,pcmd,clientssl))
						{							
							return;
						}
						if(pthread_create(&datathread->tid,NULL,makedatathread,datathread) != 0)
						{
							debugmsg(username,"[controlthread] error creating thread!",errno);
							delete datathread; 
							datathread = NULL; 
						}
						debugmsg(username,"[controlthread] datathread created");
					}
					else
					{
						debugmsg(username,"[controlthrad] maximum # of threads reached");
					}
					debugmsg(username,"[controlthread] reset datathread vars");
					gotpasvcmd = 0;
					gotportcmd = 0;
					activecon = 0;
					cpsvcmd = 0;
	
				}
				else if (gotportcmd && config.bounce_data_con)
				{
					activecon = 1;
					debugmsg(username,"[controlthread] create datathread");
					deletedatathread();	
					
					if (config.thread_limit == 0 || nr_threads < config.thread_limit)
					{
						datathread = new CDataThread(cpsvcmd, transfertype, sslprotp, relinked, usingssl, activecon, username, client_addr,  this);
						debugmsg(username,"[controlthread] call datathread getactive_data");
						datathread->getactive_data(portcmd);
						debugmsg(username,"[controlthread] call datathread getpassive_data");
						string pcmd = datathread->getpassive_data(s);
						if (!control_write(client_sock,"200 PORT command successful.\r\n",clientssl))
						{		
							
							return;
						}
						if(pthread_create(&datathread->tid,NULL,makedatathread,datathread) != 0)
						{
							debugmsg(username,"[controlthread] error creating thread!",errno);
							delete datathread; 
							datathread = NULL; 
						}
						debugmsg(username,"[controlthread] datathread created");
					}
					else
					{
						debugmsg(username,"[controlthrad] maximum # of threads reached");
					}
					debugmsg(username,"[controlthread] reset datathread vars");
					gotpasvcmd = 0;
					gotportcmd = 0;
					activecon = 0;
					cpsvcmd = 0;
				

				}
				
				else
				{
					if (!control_write(client_sock,s,clientssl))
					{
						
						
						return;
					}
					
				}

		}
		// read from client
		if (FD_ISSET(client_sock, &readfds))
		{
			debugmsg(username,"[controlthread] read from client");
			string s;
			if(!control_read(client_sock,clientssl,s))
			{
				
				
				return;
			}

			if (upper(s,5).find("IDNT",0) != string::npos)
			{
				debugmsg(username,"[controlthread] idnt command");
				if (config.entry_list != "")
				{
					if(!control_write(site_sock,s,sitessl))
					{					
						
						return;
					}
					unsigned int pos1,pos2;
					pos1 = s.find("@",0);
					if (pos1 != string::npos)
					{
						s = s.substr(pos1+1,s.length()-pos1-3);
						pos2 = s.find(":",0);
						if (pos2 != string::npos)
						{
							client_ip = s.substr(0,pos2);
							debugmsg(username,"[controlthread] client ip: " + client_ip);
						}
						else
						{
							debugmsg(username,"[controlthread] strange IDNT cmd");
							return;
						}
					}
					else
					{
						debugmsg(username,"[controlthread] strange IDNT cmd");
						return;
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
					}
				
			}
			else if (upper(s,6).find("ABOR",0) != string::npos)
			{

				s = "ABOR\r\n";
				if(!control_write(site_sock,s,sitessl))
				{					
					
					return;
				}
				debugmsg(username,"[controlthread] abort command");
				if(config.bounce_data_con)
				{
					deletedatathread();	
				}
			}


			// first command?

			else if (upper(s,9).find("AUTH TLS",0) != string::npos)
			{
				if(!control_write(site_sock,s,sitessl))
				{
					
					
					return;
				}
				debugmsg(username,"[controlthread] auth tls msg");
				if (!gotfirstcmd)
				{
					gotfirstcmd++;

				}
			}
			
			else if (upper(s,9).find("AUTH SSL",0) != string::npos)
			{
				if(!control_write(site_sock,s,sitessl))
				{
					
					
					return;
				}
				debugmsg(username,"[controlthread] auth ssl msg");
				if (!gotfirstcmd)
				{
					gotfirstcmd++;

				}
			}
			else if (upper(s,7).find("PROT P",0) != string::npos)
			{
				debugmsg(username,"[controlthread] prot p msg");
				if (usingssl) { sslprotp = 1; }
				if(!control_write(site_sock,s,sitessl))
				{
					
					
					return;
				}
			}
			else if (upper(s,7).find("PROT C",0) != string::npos)
			{
				debugmsg(username,"[controlthread] prot c msg");
				sslprotp = 0;
				if(!control_write(site_sock,s,sitessl))
				{
					
					
					return;
				}

			}
			else if (upper(s,7).find("TYPE A",0) != string::npos)
			{
				transfertype = 1;
				if(!control_write(site_sock,s,sitessl))
				{
					
					
					return;
				}
			}
			else if (upper(s,7).find("TYPE I",0) != string::npos)
			{
				transfertype = 2;
				if(!control_write(site_sock,s,sitessl))
				{
					
					
					return;
				}
			}
			else if (!gotusercmd)
			{
				if(!control_write(site_sock,s,sitessl))
				{
					
					
					return;
				}
				if (upper(s,5).find("USER",0) != string::npos)
				{

					debugmsg(username,"[controlthread] user msg");
					gotusercmd++;
					if (!gotfirstcmd)
					{
						gotfirstcmd++;

					}
					unsigned int pos,pos2;
					pos2 = s.find("!",0);
					pos = s.find(" ",0);
					if (pos2 != string::npos) { pos = pos2; }
					// return if empty user command is send
					if (pos == string::npos) { return; }
					username = s.substr(pos+1,s.length()-pos-3);

				}
			}
			else if (!gotpasscmd)
			{
				if(!control_write(site_sock,s,sitessl))
				{
					
					
					return;
				}
				if (upper(s,5).find("PASS",0) != string::npos)
				{

					debugmsg(username,"[controlthread] pass msg");
					gotpasscmd++;
				}
			}
			else if (upper(s,5).find("PASV",0) != string::npos)
			{
				if (config.bounce_data_con)
				{
					deletedatathread();
					gotpasvcmd = 1;
				}
				if(!control_write(site_sock,s,sitessl))
				{					
					return;
				}
				debugmsg(username,"[controlthread] pasv msg");
				
				
			}
			else if (upper(s,5).find("PORT",0) != string::npos)
			{
				deletedatathread();
				debugmsg(username,"[controlthread] port command");
				gotportcmd = 1;
				portcmd = s;
				if (!config.bounce_data_con)
				{
					if(!control_write(site_sock,s,sitessl))
					{
						
						
						return;
					}
					
				}
				else
				{				
					if (!control_write(site_sock,"PASV\r\n",sitessl))
					{
						
											
						return;
					}
				}
			}
			
			else if(upper(s,5).find("QUIT",0) != string::npos)
			{
				if (config.send_traffic_info)
				{
					stringstream ss;
					
					ss << "221- Download this session: " << traffic2str(localcounter.getsend()) << "\r\n";
										
					ss << "221- Upload this session: " << traffic2str(localcounter.getrecvd()) << "\r\n";
										
					if (!control_write(client_sock,ss.str(),clientssl))
					{						
						
						return;
					}
				}
				if(!control_write(site_sock,s,sitessl))
				{
					
					
					return;
				}
			}
			// bnchelp command
			else if(config.usecommands && upper(s,config.helpcmd.length()+1).find(upper(config.helpcmd,0),0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					stringstream ss;
					ss << "230- --== yatb help ==--\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.admincmd + "show' - show added admins\r\n";
					ss << "230- '" + config.admincmd + "add user1[,user2]' - add new admin(s)\r\n";
					ss << "230- '" + config.admincmd + "del user1[,user2]' - delete admin(s)\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.tositecmd + "show' - show added users allowed to fxp to site\r\n";
					ss << "230- '" + config.tositecmd + "add user1[,user2]' - add new user(s)\r\n";
					ss << "230- '" + config.tositecmd + "del user1[,user2]' - delete user(s)\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.fromsitecmd + "show' - show added users allowed to fxp from site\r\n";
					ss << "230- '" + config.fromsitecmd + "add user1[,user2]' - add new user(s)\r\n";
					ss << "230- '" + config.fromsitecmd + "del user1[,user2]' - delete user(s)\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.sslexcludecmd + "show' - show added users excluded from using ssl\r\n";
					ss << "230- '" + config.sslexcludecmd + "add user1[,user2]' - add new user(s)\r\n";
					ss << "230- '" + config.sslexcludecmd + "del user1[,user2]' - delete user(s)\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.infocmd + "' - show some infos\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.helpcmd + "' - show this help\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.reloadcmd + "' - reload config\r\n";
					ss << "230-\r\n";
					ss << "230 --== yatb help end ==--\r\n";
					if (!control_write(client_sock,ss.str(),clientssl))
					{
						
						
						return;
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						
						return;
					}

				}
			}
			//bncinfo command
			else if(config.usecommands && upper(s,config.infocmd.length() + 1).find(upper(config.infocmd,0),0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					time_t now;
					now = time(NULL);
					globals_lock.Lock();
					stringstream ss;
					ss << "230- --== stats ==--\r\n";
					ss << "230- " << version << "\r\n";
					ss << "230- Builddate: " << builddate << "\r\n";
					ss << "230- Using " << SSLeay_version(0) << "\r\n";
					if (config.enforce_tls)
					{
						ss << "230- enforce ssl is [On]\r\n";
					}
					else
					{
						ss << "230- enforce ssl is [Off]\r\n";
					}
					if (config.enforce_ident)
					{
						ss << "230- enforce ident is [On]\r\n";
					}
					else
					{
						ss << "230- enforce ident is [Off]\r\n";
					}
					if (config.enforce_tls_fxp)
					{
						ss << "230- enforce ssl fxp is [On]\r\n";
					}
					else
					{
						ss << "230- enforce ssl fxp is [Off]\r\n";
					}
					if (config.use_fxpfromsite_list)
					{
						ss << "230- from site list is [On]\r\n";
					}
					else
					{
						ss << "230- from site list is [Off]\r\n";
					}
					if (config.use_fxptosite_list)
					{
						ss << "230- to site list is [On]\r\n";
					}
					else
					{
						ss << "230- to site list is [Off]\r\n";
					}
					if (config.use_ssl_exclude)
					{
						ss << "230- ssl exclude list is [On]\r\n";
					}
					else
					{
						ss << "230- ssl exclude list is [Off]\r\n";
					}
					if (config.trytorelink)
					{
						ss << "230- try to relink is [On]\r\n";
					}
					else
					{
						ss << "230- try to relink is [Off]\r\n";
					}
					ss << "230- current connections: " << conlist.size() << "\r\n";
					ss << "230- logins so far: " << nr_logins << "\r\n";
					
					
					ss << "230- Download this session: " << traffic2str(localcounter.getsend()) << "\r\n";
							
					ss << "230- Upload this session: " << traffic2str(localcounter.getrecvd()) << "\r\n";
										
					ss << "230- Download total: " << traffic2str(totalcounter.getsend()) << "\r\n";
										
					ss << "230- Upload total: " << traffic2str(totalcounter.getrecvd()) << "\r\n";
					
					
					ss << "230- Users online: \r\n";
					
					list<CControlThread*>::iterator it;
					for (it = conlist.begin(); it != conlist.end();it++)
					{
						ss << "230- '" << (*it)->username;
						if ((*it)->relinked) { ss << "[RL]"; }
						ss << "'  -  [" << traffic2str((*it)->localcounter.getsend()) 
							<< "] DL  -  [" << traffic2str((*it)->localcounter.getrecvd());
							if (using_entry)
							{
								ss << "] UL  -  [" << (*it)->client_ip << "]  -  ";
							}
							else
							{
								ss << "] UL  -  [" << inet_ntoa((*it)->client_addr.sin_addr) << "]  -  ";
							}
							ss << (now - (*it)->connect_time) / 60 << " min online \r\n";
						
					}
					
					int daysup,hoursup,minup;
					daysup = (now - start_time) / (60*60*24);
					hoursup = ((now - start_time) - daysup * 24 * 60 * 60) / (60 * 60);
					minup = ((now - start_time) - daysup * 24 * 60 * 60 - hoursup * 60 * 60) / 60;
					ss << "230- Bnc uptime: " << daysup << " day(s), " << hoursup << " hour(s), " << minup << " minute(s)\r\n";
					ss << "230- # of current threads: " << nr_threads << "\r\n";
					ss << "230 --== stats ==--\r\n";
					globals_lock.UnLock();
					if (!control_write(client_sock,ss.str(),clientssl))
					{
						
						
						return;
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}

				}
			}
			
			else if (upper(s,5).find("CPSV",0) != string::npos)
			{
				if (config.bounce_data_con)
				{
					deletedatathread();
					gotpasvcmd = 1;
					sslprotp = 1;
					cpsvcmd = 1;
					if(!control_write(site_sock,"CPSV\r\n",sitessl))
					{					
						
						return;
					}
				}
				else
				{
					if (!control_write(site_sock,s,sitessl))
					{							
											
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.reloadcmd.length() + 1).find(upper(config.reloadcmd,0),0) != string::npos)
			{
				debugmsg(username,"[controlthread] try to reload conf");
				debugmsg(username,"[controlthread] file: '" + conffile + "' key: '" + bk + "'");
				if (adminlist.IsInList(username) && !relinked)
				{
					CConfig tmpconf;
					if (!tmpconf.readconf(conffile,bk))
					{
						if (!control_write(client_sock,"230 failed to reload config.\r\n",clientssl))
						{							
							
							return;
						}
					}
					else
					{
						config = tmpconf;
						/*config.enforce_tls = tmpconf.enforce_tls;
						config.enforce_tls_fxp = tmpconf.enforce_tls_fxp;
						config.use_fxpfromsite_list = tmpconf.use_fxpfromsite_list;
						config.use_fxptosite_list = tmpconf.use_fxptosite_list;
						config.use_ssl_exclude = tmpconf.use_ssl_exclude;
						config.use_ident = tmpconf.use_ident;
						config.enforce_ident = tmpconf.enforce_ident;
						config.trytorelink = tmpconf.trytorelink;*/
						
						if (!control_write(client_sock,"230 config reloaded.\r\n",clientssl))
						{							
							
							return;
						}
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.admincmd.length() + 5).find(upper(config.admincmd,0) + "SHOW",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					stringstream ss;
					ss << "230- added admin(s): '";
					ss << adminlist.GetList();
					
					ss << "'\r\n230 done.\r\n";
					if (!control_write(client_sock,ss.str(),clientssl))
					{
						
						
						return;
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.admincmd.length() + 4).find(upper(config.admincmd,0) + "ADD",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						adminlist.Insert(s);
						
						if (!control_write(client_sock,"230 admin(s) added.\r\n",clientssl))
						{
							
							
							return;
						}
					}
					else
					{
						if (!control_write(client_sock,"230 no admins to add!\r\n",clientssl))
						{
							
							
							return;
						}
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.admincmd.length() + 4).find(upper(config.admincmd,0) + "DEL",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						adminlist.Remove(s);
						
						if (!control_write(client_sock,"230 admin(s) removed.\r\n",clientssl))
						{
							
							
							return;
						}
					}
					else
					{
						if (!control_write(client_sock,"230 no admins to remove!\r\n",clientssl))
						{
							
							
							return;
						}
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.fromsitecmd.length() + 5).find(upper(config.fromsitecmd,0) + "SHOW",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					stringstream ss;
					ss << "230- added user(s): '";
					ss << fxpfromsitelist.GetList();
					
					ss << "'\r\n230 done.\r\n";
					if (!control_write(client_sock,ss.str(),clientssl))
					{
						
						
						return;
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.fromsitecmd.length() + 4).find(upper(config.fromsitecmd,0) + "ADD",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						fxpfromsitelist.Insert(s);
						
						if (!control_write(client_sock,"230 user(s) added.\r\n",clientssl))
						{
							
							
							return;
						}

					}
					else
					{
						if (!control_write(client_sock,"230 no user to add!\r\n",clientssl))
						{
							
							
							return;
						}
					}				
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.fromsitecmd.length() + 4).find(upper(config.fromsitecmd,0) + "DEL",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						fxpfromsitelist.Remove(s);
						
						if (!control_write(client_sock,"230 user(s) removed.\r\n",clientssl))
						{
							
							
							return;
						}
					}
					else
					{
						if (!control_write(client_sock,"230 no user to remove!\r\n",clientssl))
						{
							
							
							return;
						}
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.tositecmd.length() + 5).find(upper(config.tositecmd,0) + "SHOW",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					stringstream ss;
					ss << "230- added user(s): '";
					ss << fxptositelist.GetList();
					
					ss << "'\r\n230 done.\r\n";
					if (!control_write(client_sock,ss.str(),clientssl))
					{
						
						
						return;
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.tositecmd.length() + 4).find(upper(config.tositecmd,0) + "ADD",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						fxptositelist.Insert(s);
						
						if (!control_write(client_sock,"230 user(s) added.\r\n",clientssl))
						{
							
							
							return;
						}

					}
					else
					{
						if (!control_write(client_sock,"230 no user to add!\r\n",clientssl))
						{
							
							
							return;
						}
					}				
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.tositecmd.length() + 4).find(upper(config.tositecmd,0) + "DEL",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						fxptositelist.Remove(s);
						
						if (!control_write(client_sock,"230 user(s) removed.\r\n",clientssl))
						{
							
							
							return;
						}
					}
					else
					{
						if (!control_write(client_sock,"230 no user to remove!\r\n",clientssl))
						{
							
							
							return;
						}
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.sslexcludecmd.length() + 5).find(upper(config.sslexcludecmd,0) + "SHOW",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					stringstream ss;
					ss << "230- added user(s): '";
					ss << sslexcludelist.GetList();
					
					ss << "'\r\n230 done.\r\n";
					if (!control_write(client_sock,ss.str(),clientssl))
					{
						
						
						return;
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.sslexcludecmd.length() + 4).find(upper(config.sslexcludecmd,0) + "ADD",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						sslexcludelist.Insert(s);
						
						if (!control_write(client_sock,"230 user(s) added.\r\n",clientssl))
						{
							
							
							return;
						}

					}
					else
					{
						if (!control_write(client_sock,"230 no user to add!\r\n",clientssl))
						{
							
							
							return;
						}
					}				
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.sslexcludecmd.length() + 4).find(upper(config.sslexcludecmd,0) + "DEL",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						sslexcludelist.Remove(s);
						
						if (!control_write(client_sock,"230 user(s) removed.\r\n",clientssl))
						{
							
							
							return;
						}
					}
					else
					{
						if (!control_write(client_sock,"230 no user to remove!\r\n",clientssl))
						{
							
							
							return;
						}
					}
				}
				else
				{
					if (!control_write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{
						
						
						return;
					}
				}
			}
			else
			{
				if (!control_write(site_sock,s,sitessl))
				{
					
					
					return;
				}
			}

		}

	}
	
}

