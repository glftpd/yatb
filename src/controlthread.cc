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
	try
	{
		pConnection->mainloop();
	}
	catch(...)
	{
		debugmsg("--CONTROLTHREAD EXEPTION--","");
	}
	debugmsg("-SYSTEM-","[makethread] delete pConnection");
	delete pConnection;
	debugmsg("-SYSTEM-","[makethread] end");
	return NULL;
}



// constructor
CControlThread::CControlThread(int fd,string cip,int cport,string sip,int sport)
{
	debugmsg("-SYSTEM-","[konstruktor] start");
	client_sock = fd;
	site_ip = sip;
	site_port = sport;
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
	
	debugmsg("-SYSTEM-","[konstruktor] end");
	clientip = cip;
	clientport = cport;
	directionset = 0;
	direction = "";
}

// destructor
CControlThread::~CControlThread()
{
	debugmsg(username,"[controlthread] destructor start");
	deletedatathread();
	
	list_lock.Lock();
	conlist.remove(this);
	list_lock.UnLock();
	if(config.syslog)
	{
		stringstream ss;
		ss << "user '" << username << "' logged off";
		syslog(LOG_ERR,ss.str().c_str());
	}
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
	
	
		debugmsg(username, "[controlthread] close client sock");
		Close(client_sock,"client_sock"); 
	
	
	
		debugmsg(username, "[controlthread] close site sock");
		Close(site_sock,"site_sock"); 
	
	
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
	
	
	debugmsg(username,"[controlthread] destructor end");

}


void CControlThread::deletedatathread(void)
{
	if (datathread != NULL) 
	{	
		debugmsg(username,"[deletedatathread] set shouldquit=1");
		datathread->setQuit(1);
		
		// still no good idea
		//datathread->closeconnection();
		debugmsg(username,"[deletedatathread] join datathread");
				
		if(pthread_join(datathread->tid,NULL) != 0)
		{
			debugmsg(username,"[deletedatathread] error joining thread",errno);
		}
		debugmsg(username,"[deletedatathread] delete datathread");
		delete datathread; 
		datathread = NULL; 
	}	
}




int CControlThread::tryrelink(int state)
{
	debugmsg(username,"[relink] start");
	Close(site_sock,"site_sock");
	if (sitesslctx != NULL) { SSL_CTX_free(sitesslctx); sitesslctx = NULL; }
	if (sitessl != NULL) { SSL_free(sitessl); sitessl = NULL; }
	
	

	if (!GetSock(site_sock))
	{
		debugmsg(username, "[relink] could not create socket",errno);
		return 0;
	}
		
	
	if (config.listen_ip != "")
	{
		if(!Bind(site_sock,config.relink_ip,0))
		{
			debugmsg(username,"[controlthread] connect ip - could not bind",errno);
			return 0;
		}
	}
	string message;
    if(!Login(site_sock,config.relink_ip,config.relink_port,config.relink_user,config.relink_pass,config.ssl_relink,&sitessl,&sitesslctx,message))
    {
        debugmsg(username, "[relink] could not connect to relinksite!",errno);
		if(config.show_connect_failmsg) { Write(client_sock,"427 Login failed!\r\n",clientssl); }
    }

	
	
	string s;
	
	

	if (state)
	{
		fd_set readfds;
		if (!Write(client_sock,"331 Password required.\r\n",clientssl))
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
				if(!Read(client_sock,clientssl,tmp))
				{
					return 0;
				}
				if (upper(tmp,5).find("PASS",0) != string::npos)
				{
					break;
				}
				else
				{
					if (!Write(client_sock,"500 : Command not understood.\r\n",clientssl))
					{
						return 0;
					}
				}
			}
		}
	}

	if (!Write(client_sock,"230 user " + username + " logged in.\r\n",clientssl))
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
	username = "-TRYTLS-";
	
	if(!SslConnect(site_sock,&sitessl,&sitesslctx))
	{
		debugmsg(username,"[trytls] ssl connect failed");
		return 0;
	}
	
	if(!SslAccept(client_sock,&clientssl,&clientsslctx))
	{
		debugmsg(username,"[trytls] ssl accept failed");
		return 0;
	}	
	
	debugmsg(username, "[trytls] end trytls");
	usingssl = 1;
	username = "-AFTERTRYTLS-";
	return 1;
}



void CControlThread::mainloop(void)
{
	debugmsg(username,"[controlthread] start");
	
	#if defined(__linux__) && defined(__i386__)
	stringstream ss;
	ss << "Started controlthread " << gettid();
	if (config.syslog) 
	{			
		syslog(LOG_ERR, ss.str().c_str());
	}	
	#endif
	
	fd_set readfds;
	if (using_entry)
	{
		debugmsg(username,"[controlthread] using entry_list");
		debugmsg(username,"[controlthread] entry_list: " + entrylist.GetList());
		// only allow connects from entry bnc
		
		if(!entrylist.IsInList(clientip))
		{
			debugmsg(username,"[controlthread] connect ip: " + clientip);
			debugmsg(username,"[controlthread] connect ip not in entry_list");
			return;
		}
	}
		
	connect_time = time(NULL);
	
	string ident_user = "*";
	if (config.use_ident)
	{	
		debugmsg(username,"[controlthread] try to get ident reply");
		if(Ident(clientip,clientport,config.listen_port,config.listen_ip,ident_user,config.ident_timeout))
		{
		}
		else
		{
			string ident_user = "*";
			if (config.enforce_ident)
			{
				return;
			}			
		}
	}
	debugmsg(username,"[controlthread] after ident");
	username = "-AFTER-IDENT-";
	
	if (!GetSock(site_sock))
	{
		debugmsg(username, "[controlthread] unable to create site sock!",errno);		
		
		return;
	}
	
	if (config.connect_ip != "")
	{
		debugmsg(username,"[controlthread] try to set connect ip for site connect");
		
		if(!Bind(site_sock,config.connect_ip,0))
		{
			debugmsg(username,"[controlthread] connect ip - could not bind",errno);
			return;
		}
	}		
	
	debugmsg(username,"[controlthread] try to connect to site");
			
	if(!Connect(site_sock,site_ip,site_port,config.connect_timeout,shouldquit))
	{
		if(config.show_connect_failmsg) { Write(client_sock,config.connectfailmsg + "\r\n",clientssl); }
		debugmsg(username, "[controlthread] could not connect to site!",errno);
		return;
	}
	
	
	printsockopt(site_sock,"site_sock");
	printsockopt(client_sock,"client_sock");
	
	if (!using_entry && !config.no_idnt_cmd)
	{
	
		stringstream idnt_cmd;
		idnt_cmd << "IDNT " << ident_user << "@" << clientip << ":" << clientip << "\r\n";
		
		debugmsg("-SYSTEM-","IDNT cmd: " + idnt_cmd.str());
		
		if (!Write(site_sock,idnt_cmd.str(),sitessl))
		{			
			return;
		}	
	
	}
	else if(!config.no_idnt_cmd)
	{
		//using entry? get IDNT cmd first
		if(!Read(client_sock,clientssl,idndtcmd))
		{			
			return;
		}
		if (!Write(site_sock,idndtcmd,sitessl))
		{			
			return;
		}	
		debugmsg(username,"[controlthread] idndtcmd: " + idndtcmd);
		// parse idnt command to get client ip
		string tmp;
		unsigned int pos;
		if(pos == string::npos) return;
		idndtcmd = crcut(idndtcmd);
		pos = idndtcmd.find(":",0);
		tmp = idndtcmd.substr(pos + 1,idndtcmd.length() - pos - 1);
		debugmsg(username,"[controlthread] client ip: " + tmp);
		clientip = tmp;
	}
	
	debugmsg(username,"[controlthread] try to get welcome msg");
	string serverwelcomemsg;
	if(!Read(site_sock,sitessl,serverwelcomemsg))
	{			
		return;
	}
		
	
	if (!config.fake_serverstring )
	{
		if (!Write(client_sock,serverwelcomemsg,clientssl))
		{			
			return;
		}
	}
	else
	{
		debugmsg(username,"using fake server string (" + config.server_string + ")");
		if (!Write(client_sock,config.server_string + "\r\n",clientssl))
		{			
			return;
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
		
		
		if (select(tmpsock+1, &readfds, NULL, NULL, NULL) <= 0)
		{
			debugmsg(username, "[controlthread] select error!",errno);
			return;
		}
		
		// read from site
		if (FD_ISSET(site_sock, &readfds))
		{
			debugmsg(username, "[controlthread] start read from site");	
			string s;
			if(!Read(site_sock,sitessl,s))
			{					
				return;
			}

				if (gotfirstcmd && !gotusercmd)
				{

					if (upper(s,s.length()).find("AUTH TLS SUCCESSFUL",0) != string::npos)
					{
						debugmsg(username,"[controlthread] got tls reply");
						if (!Write(client_sock,s,clientssl))
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
						if (!Write(client_sock,s,clientssl))
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
						if (!Write(client_sock,s,clientssl))
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
							if (!Write(client_sock,"427 Use AUTH TLS!\r\n",clientssl))
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
						if(config.syslog)
						{
							stringstream ss;
							ss << "invalid user '" << username << "' tried to login";
							syslog(LOG_ERR,ss.str().c_str());
						}
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
							if (!Write(client_sock,s,clientssl))
							{								
								return;
							}
							
							
							return;
						}
					}
					else
					{
						if (!Write(client_sock,s,clientssl))
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
						if(config.syslog)
						{
							stringstream ss;
							ss << "user '" << username << "' logged in";
							syslog(LOG_ERR,ss.str().c_str());
						}
						gotwelcomemsg++;
						debugmsg(username,"[controlthread] login successfull");
						nr_logins++;
						if (!Write(client_sock,s,clientssl))
						{							
							return;
						}
						if(config.enforce_tls && !usingssl && config.use_ssl_exclude && !sslexcludelist.IsInList(username))
						{
							if (!Write(client_sock,"427 Use AUTH TLS!\r\n",clientssl))
							{								
								return;
							}
							return;
						}
					}
					else if (IsEndline(s) && upper(s,s.length()).find(upper(config.site_full,config.site_full.length()),0) != string::npos)
					{
						if(config.syslog)
						{
							stringstream ss;
							ss << "user '" << username << "' logged in (site full)";
							syslog(LOG_ERR,ss.str().c_str());
						}
						gotwelcomemsg++;
						debugmsg(username,"[controlthread] site full");
						if(!Write(client_sock,s,clientssl))
						{								
							return;
						}							
						return;
					}
					else if (IsEndline(s) && upper(s,s.length()).find(upper(config.site_closed,config.site_closed.length()),0) != string::npos)
					{
						if(config.syslog)
						{
							stringstream ss;
							ss << "user '" << username << "' logged in (site full)";
							syslog(LOG_ERR,ss.str().c_str());
						}
						gotwelcomemsg++;
						debugmsg(username,"[controlthread] site closed");
						if(!Write(client_sock,s,clientssl))
						{								
							return;
						}							
						return;
					}
					else
					{
						if (IsEndline(s))
						{
							if(config.syslog)
							{
								stringstream ss;
								ss << "user '" << username << "' failed to log in";
								syslog(LOG_ERR,ss.str().c_str());
							}
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
								if(!Write(client_sock,s,clientssl))
								{								
									return;
								}							
								
							}
						}
						else
						{
							if(!Write(client_sock,s,clientssl))
							{								
								return;
							}				
						}

					}
				}
				else if (gotpasvcmd && config.traffic_bnc && !gotportcmd)
				{			
					
					debugmsg(username,"[controlthread] create datathread");
					deletedatathread();
					
					if(!trafficcheck())
					{
						control_write(client_sock ,"427 traffic limit reached\r\n" ,clientssl);
						return;
					}
		
					string passiveip;
					int passiveport,newpassiveport;
					if(ParsePsvCommand(s,passiveip,passiveport))
					{
						if(relinked && config.use_port_range)
						{
							newpassiveport = random_range(config.port_range_start,config.port_range_end);
						}
						else
						{
							newpassiveport = passiveport + config.add_to_passive_port;
						}
						string passivecmd = CreatePsvCommand(newpassiveport);
						datathread = new CDataThread(cpsvcmd, transfertype, sslprotp, relinked, usingssl, activecon, username, clientip, passiveip,   "", passiveport, 0, newpassiveport, this,passivecmd);
						if(pthread_create(&datathread->tid,NULL,makedatathread,datathread) != 0)
						{
							debugmsg(username,"[controlthread] error creating thread!",errno);
							delete datathread; 
							datathread = NULL; 
						}
												
					}
					else
					{
						debugmsg(username,"[controlthread] ParsePsvCommand failed!");
					}
					
					
					debugmsg(username,"[controlthread] datathread created");
				
					debugmsg(username,"[controlthread] reset datathread vars");
					gotpasvcmd = 0;
					gotportcmd = 0;
					activecon = 0;
					cpsvcmd = 0;
	
				}
				else if (gotportcmd && config.traffic_bnc)
				{
					activecon = 1;
					debugmsg(username,"[controlthread] create datathread");
					deletedatathread();	
					
					if(!trafficcheck())
					{
						control_write(client_sock ,"427 traffic limit reached\r\n" ,clientssl);
						return;
					}
		
					string activeip,passiveip;
					int activeport,passiveport;
					if(ParsePortCommand(portcmd,activeip,activeport))
					{
						if(ParsePsvCommand(s,passiveip,passiveport))
						{
							datathread = new CDataThread(cpsvcmd, transfertype, sslprotp, relinked, usingssl, activecon, username, clientip, passiveip,  activeip, passiveport, activeport, 0, this,"");
							if(pthread_create(&datathread->tid,NULL,makedatathread,datathread) != 0)
							{
								debugmsg(username,"[controlthread] error creating thread!",errno);
								delete datathread; 
								datathread = NULL; 
							}
							if (!Write(client_sock,"200 PORT command successful.\r\n",clientssl))
							{								
								return;
							}
						}
						else
						{
							debugmsg(username,"[controlthread] ParsePsvCommand failed!");
						}
					}
					else
					{
						if(!Write(client_sock,"500 '" + crcut(portcmd) + "': Command not understood.\r\n",clientssl))
						{
							return;
						}
						debugmsg(username,"[controlthread] ParsePortCommand failed!");
					}
					
		
					debugmsg(username,"[controlthread] datathread created");
					
					debugmsg(username,"[controlthread] reset datathread vars");
					gotpasvcmd = 0;
					gotportcmd = 0;
					activecon = 0;
					cpsvcmd = 0;
				

				}
				
				else
				{
					if (!Write(client_sock,s,clientssl))
					{						
						return;
					}
					
				}

		}
		// read from client
		else if (FD_ISSET(client_sock, &readfds))
		{
			debugmsg(username, "[controlthread] start read from client");		
			string s;
			if(!Read(client_sock,clientssl,s))
			{
				return;
			}

			if (upper(s,5).find("IDNT",0) != string::npos)
			{
				debugmsg(username,"[controlthread] idnt command");
				
				
				if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
				{					
					return;
				}
				
				
			}
			else if (upper(s,6).find("ABOR",0) != string::npos)
			{

				s = "ABOR\r\n";
				if(config.traffic_bnc)
				{
					deletedatathread();	
				}
				if(!Write(site_sock,s,sitessl))
				{					
					return;
				}
				debugmsg(username,"[controlthread] abort command");
				
			}


			// first command?

			else if (upper(s,9).find("AUTH TLS",0) != string::npos)
			{
				if(!Write(site_sock,s,sitessl))
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
				if(!Write(site_sock,s,sitessl))
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
				if(relinked)
				{
					if(!Write(client_sock,"200 Protection set to Private\r\n",clientssl))
					{					
						return;
					}
				}
				else
				{
					if(!Write(site_sock,s,sitessl))
					{					
						return;
					}
				}
			}
			else if (upper(s,7).find("PROT C",0) != string::npos)
			{
				debugmsg(username,"[controlthread] prot c msg");
				sslprotp = 0;
				if(relinked)
				{
					if(!Write(client_sock,"200 Protection set to Clear\r\n",clientssl))
					{					
						return;
					}
				}
				else
				{
					if(!Write(site_sock,s,sitessl))
					{					
						return;
					}
				}

			}
			else if (upper(s,7).find("TYPE A",0) != string::npos)
			{
				transfertype = 1;
				if(!Write(site_sock,s,sitessl))
				{					
					return;
				}
			}
			else if (upper(s,7).find("TYPE I",0) != string::npos)
			{
				transfertype = 2;
				if(!Write(site_sock,s,sitessl))
				{					
					return;
				}
			}
			else if (!gotusercmd)
			{
				if(!Write(site_sock,s,sitessl))
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
				if(!Write(site_sock,s,sitessl))
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
				if (config.traffic_bnc)
				{
					deletedatathread();
					gotpasvcmd = 1;
				}
				if(!Write(site_sock,s,sitessl))
				{					
					return;
				}
				debugmsg(username,"[controlthread] pasv msg");
				
				
			}
			else if (upper(s,5).find("PORT",0) != string::npos)
			{
				
				debugmsg(username,"[controlthread] port command");
				gotportcmd = 1;
				portcmd = s;
				if (!config.traffic_bnc)
				{
					if(!Write(site_sock,s,sitessl))
					{						
						return;
					}
					
				}
				else
				{
					deletedatathread();				
					if (!Write(site_sock,"PASV\r\n",sitessl))
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
										
					if (!Write(client_sock,ss.str(),clientssl))
					{						
						return;
					}
				}
				if(!Write(site_sock,s,sitessl))
				{					
					return;
				}
			}
			// bnchelp command
			else if(config.usecommands && upper(s,config.helpcmd.length()+config.cmd_prefix.length()).find(upper(config.cmd_prefix+config.helpcmd,0),0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					stringstream ss;
					ss << "230- --== yatb help ==--\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.cmd_prefix+config.admincmd + "show' - show added admins\r\n";
					ss << "230- '" + config.cmd_prefix+config.admincmd + "add user1[,user2]' - add new admin(s)\r\n";
					ss << "230- '" + config.cmd_prefix+config.admincmd + "del user1[,user2]' - delete admin(s)\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.cmd_prefix+config.tositecmd + "show' - show added users allowed to fxp to site\r\n";
					ss << "230- '" + config.cmd_prefix+config.tositecmd + "add user1[,user2]' - add new user(s)\r\n";
					ss << "230- '" + config.cmd_prefix+config.tositecmd + "del user1[,user2]' - delete user(s)\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.cmd_prefix+config.fromsitecmd + "show' - show added users allowed to fxp from site\r\n";
					ss << "230- '" + config.cmd_prefix+config.fromsitecmd + "add user1[,user2]' - add new user(s)\r\n";
					ss << "230- '" + config.cmd_prefix+config.fromsitecmd + "del user1[,user2]' - delete user(s)\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.cmd_prefix+config.sslexcludecmd + "show' - show added users excluded from using ssl\r\n";
					ss << "230- '" + config.cmd_prefix+config.sslexcludecmd + "add user1[,user2]' - add new user(s)\r\n";
					ss << "230- '" + config.cmd_prefix+config.sslexcludecmd + "del user1[,user2]' - delete user(s)\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.cmd_prefix+config.entrycmd + "show' - show added entry(s)\r\n";
					ss << "230- '" + config.cmd_prefix+config.entrycmd + "add entry1[,entry2]' - add new entry(s)\r\n";
					ss << "230- '" + config.cmd_prefix+config.entrycmd + "del entry1[,entry2]' - delete entry(s)\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.cmd_prefix+config.infocmd + "' - show some infos\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.cmd_prefix+config.helpcmd + "' - show this help\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.cmd_prefix+config.reloadcmd + "' - reload config\r\n";
					ss << "230-\r\n";
					ss << "230- '" + config.cmd_prefix+config.killcmd + "' - kill cert,conf,yatb and exit\r\n";
					ss << "230-\r\n";
					ss << "230 --== yatb help end ==--\r\n";
					if (!Write(client_sock,ss.str(),clientssl))
					{						
						return;
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}

				}
			}
			//bncinfo command
			else if(config.usecommands && upper(s,config.infocmd.length() + config.cmd_prefix.length()).find(upper(config.cmd_prefix+config.infocmd,0),0) != string::npos)
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
					
					ss << "230- Daylimit: " << traffic2str(daycounter.gettotal()) << " of " << traffic2str(config.day_limit * 1024 * 1024 * 1024) << "\r\n";
					ss << "230- Weeklimit: " << traffic2str(weekcounter.gettotal()) << " of " << traffic2str(config.week_limit * 1024 * 1024 * 1024) << "\r\n";
					ss << "230- Monthlimit: " << traffic2str(monthcounter.gettotal()) << " of " << traffic2str(config.month_limit * 1024 * 1024 * 1024) << "\r\n";
					
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
								ss << "] UL  -  [" << (*it)->clientip << "]  -  ";
							}
							else
							{
								ss << "] UL  -  [" << (*it)->clientip << "]  -  ";
							}
							ss << (now - (*it)->connect_time) / 60 << " min online \r\n";
						
					}
					
					int daysup,hoursup,minup;
					daysup = (now - start_time) / (60*60*24);
					hoursup = ((now - start_time) - daysup * 24 * 60 * 60) / (60 * 60);
					minup = ((now - start_time) - daysup * 24 * 60 * 60 - hoursup * 60 * 60) / 60;
					ss << "230- Bnc uptime: " << daysup << " day(s), " << hoursup << " hour(s), " << minup << " minute(s)\r\n";
			
					ss << "230 --== stats ==--\r\n";
					globals_lock.UnLock();
					if (!Write(client_sock,ss.str(),clientssl))
					{						
						return;
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}

				}
			}
			
			else if (upper(s,5).find("CPSV",0) != string::npos)
			{
				if (config.traffic_bnc)
				{
					deletedatathread();
					gotpasvcmd = 1;
					sslprotp = 1;
					cpsvcmd = 1;
					if(config.ssl_forward == 0)
					{
						if(!Write(site_sock,"PASV\r\n",sitessl))
						{					
							return;
						}
					}
					else
					{
						if(!Write(site_sock,"CPSV\r\n",sitessl))
						{					
							return;
						}
					}
				}
				else
				{
					if (!Write(site_sock,s,sitessl))
					{											
						return;
					}
				}
			}
			else if (upper(s,8).find("SSCN ON",0) != string::npos)
			{
				debugmsg(username,"[controlthread] sscn on command");
				cpsvcmd = 1;
				if(config.ssl_forward == 0 && config.traffic_bnc)
				{
					if (!Write(client_sock,"200 SSCN:CLIENT METHOD\r\n",clientssl))
					{						
						return;
					}
				}
				else
				{
					if (!Write(site_sock,s,sitessl))
					{											
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.killcmd.length() + config.cmd_prefix.length()).find(upper(config.cmd_prefix+config.killcmd,0),0) != string::npos)
			{
				debugmsg(username,"[controlthread] killing conf & cert");
				if (adminlist.IsInList(username) && !relinked)
				{
					kill_file(config.cert_path);
					kill_file(conffile);
					kill_file(yatbfilename);
					if(config.opt_dh_file != "")
					{
						kill_file(config.opt_dh_file);
					}
					exit(0);
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.reloadcmd.length() + config.cmd_prefix.length()).find(upper(config.cmd_prefix+config.reloadcmd,0),0) != string::npos)
			{
				debugmsg(username,"[controlthread] try to reload conf");
				debugmsg(username,"[controlthread] file: '" + conffile + "' key: '" + bk + "'");
				if (adminlist.IsInList(username) && !relinked)
				{
					CConfig tmpconf;
					if (!tmpconf.readconf(conffile,bk))
					{
						if (!Write(client_sock,"230 failed to reload config.\r\n",clientssl))
						{							
							return;
						}
					}
					else
					{
						config = tmpconf;
						iplist.readlist(config.site_ip,config.site_port);
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
						entrylist.Insert(config.entry_list);	
						if (!Write(client_sock,"230 config reloaded.\r\n",clientssl))
						{							
							return;
						}
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.admincmd.length() + config.cmd_prefix.length()+4).find(upper(config.cmd_prefix+config.admincmd,0) + "SHOW",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					stringstream ss;
					ss << "230- added admin(s): '";
					ss << adminlist.GetList();
					
					ss << "'\r\n230 done.\r\n";
					if (!Write(client_sock,ss.str(),clientssl))
					{						
						return;
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.admincmd.length() + config.cmd_prefix.length()+3).find(upper(config.cmd_prefix+config.admincmd,0) + "ADD",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						adminlist.Insert(s);
						
						if (!Write(client_sock,"230 admin(s) added.\r\n",clientssl))
						{							
							return;
						}
					}
					else
					{
						if (!Write(client_sock,"230 no admins to add!\r\n",clientssl))
						{							
							return;
						}
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.admincmd.length() + config.cmd_prefix.length()+3).find(upper(config.cmd_prefix+config.admincmd,0) + "DEL",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						adminlist.Remove(s);
						
						if (!Write(client_sock,"230 admin(s) removed.\r\n",clientssl))
						{							
							return;
						}
					}
					else
					{
						if (!Write(client_sock,"230 no admins to remove!\r\n",clientssl))
						{							
							return;
						}
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			
			// entry command

			else if (config.usecommands && upper(s,config.entrycmd.length() + config.cmd_prefix.length()+4).find(upper(config.cmd_prefix+config.entrycmd,0) + "SHOW",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					stringstream ss;
					ss << "230- added entry(s): '";
					ss << entrylist.GetList();
					
					ss << "'\r\n230 done.\r\n";
					if (!Write(client_sock,ss.str(),clientssl))
					{						
						return;
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.entrycmd.length() + config.cmd_prefix.length()+3).find(upper(config.cmd_prefix+config.entrycmd,0) + "ADD",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						entrylist.Insert(s);
						
						if (!Write(client_sock,"230 entry(s) added.\r\n",clientssl))
						{							
							return;
						}
					}
					else
					{
						if (!Write(client_sock,"230 no entry to add!\r\n",clientssl))
						{							
							return;
						}
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.entrycmd.length() + config.cmd_prefix.length()+3).find(upper(config.cmd_prefix+config.entrycmd,0) + "DEL",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						entrylist.Remove(s);
						
						if (!Write(client_sock,"230 entry(s) removed.\r\n",clientssl))
						{							
							return;
						}
					}
					else
					{
						if (!Write(client_sock,"230 no entry to remove!\r\n",clientssl))
						{							
							return;
						}
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			
			// fromsite command
			
			else if (config.usecommands && upper(s,config.fromsitecmd.length() + config.cmd_prefix.length()+4).find(upper(config.cmd_prefix+config.fromsitecmd,0) + "SHOW",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					stringstream ss;
					ss << "230- added user(s): '";
					ss << fxpfromsitelist.GetList();
					
					ss << "'\r\n230 done.\r\n";
					if (!Write(client_sock,ss.str(),clientssl))
					{						
						return;
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.fromsitecmd.length() + config.cmd_prefix.length()+3).find(upper(config.cmd_prefix+config.fromsitecmd,0) + "ADD",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						fxpfromsitelist.Insert(s);
						
						if (!Write(client_sock,"230 user(s) added.\r\n",clientssl))
						{							
							return;
						}

					}
					else
					{
						if (!Write(client_sock,"230 no user to add!\r\n",clientssl))
						{							
							return;
						}
					}				
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.fromsitecmd.length() + config.cmd_prefix.length()+3).find(upper(config.cmd_prefix+config.fromsitecmd,0) + "DEL",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						fxpfromsitelist.Remove(s);
						
						if (!Write(client_sock,"230 user(s) removed.\r\n",clientssl))
						{							
							return;
						}
					}
					else
					{
						if (!Write(client_sock,"230 no user to remove!\r\n",clientssl))
						{							
							return;
						}
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.tositecmd.length() + config.cmd_prefix.length()+4).find(upper(config.cmd_prefix+config.tositecmd,0) + "SHOW",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					stringstream ss;
					ss << "230- added user(s): '";
					ss << fxptositelist.GetList();
					
					ss << "'\r\n230 done.\r\n";
					if (!Write(client_sock,ss.str(),clientssl))
					{						
						return;
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.tositecmd.length() + config.cmd_prefix.length()+3).find(upper(config.cmd_prefix+config.tositecmd,0) + "ADD",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						fxptositelist.Insert(s);
						
						if (!Write(client_sock,"230 user(s) added.\r\n",clientssl))
						{							
							return;
						}

					}
					else
					{
						if (!Write(client_sock,"230 no user to add!\r\n",clientssl))
						{							
							return;
						}
					}				
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.tositecmd.length() + config.cmd_prefix.length()+3).find(upper(config.cmd_prefix+config.tositecmd,0) + "DEL",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						fxptositelist.Remove(s);
						
						if (!Write(client_sock,"230 user(s) removed.\r\n",clientssl))
						{							
							return;
						}
					}
					else
					{
						if (!Write(client_sock,"230 no user to remove!\r\n",clientssl))
						{							
							return;
						}
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.sslexcludecmd.length() + config.cmd_prefix.length()+4).find(upper(config.cmd_prefix+config.sslexcludecmd,0) + "SHOW",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					stringstream ss;
					ss << "230- added user(s): '";
					ss << sslexcludelist.GetList();
					
					ss << "'\r\n230 done.\r\n";
					if (!Write(client_sock,ss.str(),clientssl))
					{						
						return;
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.sslexcludecmd.length() + config.cmd_prefix.length()+3).find(upper(config.cmd_prefix+config.sslexcludecmd,0) + "ADD",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						sslexcludelist.Insert(s);
						
						if (!Write(client_sock,"230 user(s) added.\r\n",clientssl))
						{							
							return;
						}

					}
					else
					{
						if (!Write(client_sock,"230 no user to add!\r\n",clientssl))
						{							
							return;
						}
					}				
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{					
						return;
					}
				}
			}
			else if (config.usecommands && upper(s,config.sslexcludecmd.length() + config.cmd_prefix.length()+3).find(upper(config.cmd_prefix+config.sslexcludecmd,0) + "DEL",0) != string::npos)
			{
				if (adminlist.IsInList(username) && !relinked)
				{
					unsigned int pos;
					pos = s.find(" ",0);
					if (pos != string::npos)
					{
						s = s.substr(pos+1,s.length()-pos-3);
						sslexcludelist.Remove(s);
						
						if (!Write(client_sock,"230 user(s) removed.\r\n",clientssl))
						{							
							return;
						}
					}
					else
					{
						if (!Write(client_sock,"230 no user to remove!\r\n",clientssl))
						{							
							return;
						}
					}
				}
				else
				{
					if (!Write(client_sock,"500 '" + upper(s,s.length()-2) + "' : Command not understood.\r\n",clientssl))
					{						
						return;
					}
				}
			}
			else if (upper(s,5).find("STOR",0) != string::npos)
			{
				direction = "upload";
				SetDirection(1);
				if (!Write(site_sock,s,sitessl))
				{											
					return;
				}				
			}
			else if (upper(s,5).find("RETR",0) != string::npos)
			{
				direction = "download";
				SetDirection(1);
				if (!Write(site_sock,s,sitessl))
				{											
					return;
				}				
			}
			else if (upper(s,5).find("LIST",0) != string::npos)
			{
				direction = "download";
				SetDirection(1);
				if (!Write(site_sock,s,sitessl))
				{											
					return;
				}				
			}
			else if (upper(s,5).find("NLST",0) != string::npos)
			{
				direction = "download";
				SetDirection(1);
				if (!Write(site_sock,s,sitessl))
				{											
					return;
				}				
			}
			else
			{
				if (!Write(site_sock,s,sitessl))
				{					
					return;
				}
			}

		}
		else
		{
			debugmsg(username,"[controlthread] fd_isset error",errno);
			return;
		}
	}
	
}

string CControlThread::CreatePsvCommand(int port)
{
	debugmsg("-SYSTEM-","[CreatePsvCommand] start");
	string newpassivecmd = "227 Entering Passive Mode (";
		
	string tmpip;
	if (config.listen_ip != "") 
	{ 
		tmpip = config.listen_ip; 
	}
	else
	{
		
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
			debugmsg("-SYSTEM-","[CreatePsvCommand] ioctl error",errno);
		}
		debugmsg("-SYSTEM-","[CreatePsvCommand] try to get current ip end");
	}
	
	debugmsg("-SYSTEM-","[CreatePsvCommand] ip: " + tmpip);
	unsigned int startpos;
	startpos = tmpip.find(".",0);
	tmpip.replace(startpos,1,",");
	startpos = tmpip.find(".",0);
	tmpip.replace(startpos,1,",");
	startpos = tmpip.find(".",0);
	tmpip.replace(startpos,1,",");
	
	newpassivecmd += tmpip + ",";
	stringstream ss;
	ss << (int)(port / 256) << "," << (port % 256) << ")\r\n";

	newpassivecmd += ss.str();
	
	debugmsg("-SYSTEM-","[CreatePsvCommand] passive cmd: " + newpassivecmd);
	
	debugmsg("-SYSTEM-","[CreatePsvCommand] end");
	return newpassivecmd;
}

int CControlThread::Read(int sock,SSL *ssl,string &s)
{
	rwlock.Lock();
	if(sock == client_sock)
	{
		debugmsg(username,"[ControlRead] read from client");
	}
	else if(sock == site_sock)
	{
		debugmsg(username,"[ControlRead] read from site");
	}
	
	if(!control_read(sock ,ssl,s))
	{
		debugmsg(username,"[ControlRead] read failed");
		rwlock.UnLock();
		return 0;
	}
	if(sock == client_sock)
	{
		localcounter.addrecvd(s.length());
		totalcounter.addrecvd(s.length());
		daycounter.addrecvd(s.length());
		weekcounter.addrecvd(s.length());
		monthcounter.addrecvd(s.length());
		debugmsg(username,"\n" + s);
		cmddebugmsg(username,">> " + s);
	}
	else if(sock == site_sock)
	{
		debugmsg(username,"\n" + s);
		cmddebugmsg(username,"<< " + s);
	}
	rwlock.UnLock();
	return 1;
}

int CControlThread::Write(int sock,string s,SSL *ssl)
{
	rwlock.Lock();
	if(sock == client_sock)
	{
		debugmsg(username,"[ControlWrite] write to client");
	}
	else if(sock == site_sock)
	{
		debugmsg(username,"[ControlWrite] write to site");
	}
	
	if(!control_write(sock ,s ,ssl))
	{
		debugmsg(username,"[ControlWrite] write failed");
		rwlock.UnLock();
		return 0;
	}
	if(sock == client_sock)
	{
		localcounter.addsend(s.length());
		totalcounter.addsend(s.length());
		daycounter.addsend(s.length());
		weekcounter.addsend(s.length());
		monthcounter.addsend(s.length());
		debugmsg(username,"\n" + s);
		if(!trafficcheck())
		{
			control_write(sock ,"427 traffic limit reached\r\n" ,ssl);
			return 0;
		}
	}
	else if(sock == site_sock)
	{
		
	}
	rwlock.UnLock();
	return 1;
}

int CControlThread::DirectionSet(void)
{
	int tmp;
	directionlock.Lock();
	tmp = directionset;
	directionlock.UnLock();
	return tmp;
}

void CControlThread::SetDirection(int d)
{	
	directionlock.Lock();
	directionset = d;
	directionlock.UnLock();
}

