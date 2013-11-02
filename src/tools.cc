#include "tools.h"
#include "global.h"
#include "config.h"
#include "counter.h"

#include "lock.h"

// print debug msg
void debugmsg(string un,string s,int err)
{
	if (config.debug)
	{
		#if defined(__linux__) && defined(__i386__)
		stringstream ss;
		ss << gettid();
		un = un + " - " + ss.str();
		#endif
		config_lock.Lock();
		time_t rawtime;
	  struct tm * timeinfo;

  	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
		string t = asctime (timeinfo);
		t = t.substr(0,t.length() - 1);
		if (!config.log_to_screen)
		{
			ofstream dbg_logfile(config.debug_logfile.c_str(),ios::out | ios::app);
			if (dbg_logfile)
			{
				dbg_logfile << "[" << un << "," << t << "] " << s << endl ;
				if (err != 0)
				{
					dbg_logfile << "[" << un << "," << t << "] " << strerror(err) << endl;
				}
				dbg_logfile.flush();
				dbg_logfile.close();
			}		
		}		
		else
		{
			cout << "[" << un << "," << t << "] " << s << endl ;
			if (err != 0)
			{
				cout << "[" << un << "," << t << "] " << strerror(err) << endl;
			}
		}
		
		config_lock.UnLock();
	}
}

void cmddebugmsg(string un,string s)
{
	if (config.debug && config.command_logfile != "")
	{		
		config_lock.Lock();
		time_t rawtime;
		struct tm * timeinfo;

	  	time ( &rawtime );
  		timeinfo = localtime ( &rawtime );
		string t = asctime (timeinfo);
		t = t.substr(0,t.length() - 1);
		
		ofstream dbg_logfile(config.command_logfile.c_str(),ios::out | ios::app);
		if (dbg_logfile)
		{
			dbg_logfile << "[" << un << "," << t << "] " << s << endl ;
			
			dbg_logfile.flush();
			dbg_logfile.close();
		}		
		
		
		config_lock.UnLock();
	}
}

string ltrim( const string &str, const string &whitespace)
{

   unsigned int idx = str.find_first_not_of(whitespace);
   if( idx != string::npos )
       return str.substr(idx);
   
   return "";
}

string rtrim( const string &str, const string &whitespace)
{

   unsigned int idx = str.find_last_not_of(whitespace);
   if( idx != string::npos )
       return str.substr(0,idx+1);

   return str;
}

string trim( const string &str, const string &whitespace)
{
    return rtrim(ltrim(str,whitespace),whitespace);
}

// cut off \r\n from string
string crcut(string s)
{
	string tmp;
	tmp = s;
	if(tmp[tmp.length()-1] == '\n')
	{
		tmp = tmp.substr(0,tmp.length() - 1);
	}
	if(tmp[tmp.length()-1] == '\r')
	{
		tmp = tmp.substr(0,tmp.length() - 1);
	}
	return tmp;
}

// return input string in upcase
string upper(string str,int length)
{
	if (length == 0) { length = str.length(); }
	string tmp;
	tmp = str;
	std::transform (tmp.begin(),tmp.end(), tmp.begin(), (int(*)(int)) toupper);
	return tmp.substr(0,length);
}

// get random port number from specified range
int random_range(int lowest_number, int highest_number)
{	
  if(lowest_number > highest_number){
      swap(lowest_number, highest_number);
  }

  int range = highest_number - lowest_number + 1;
  double r = ((double)rand() / (double)(RAND_MAX+1) );
  return (lowest_number + abs(int(range * r)));
}

int setnonblocking(int socket)
{
	int flags;
	if((flags = fcntl(socket, F_GETFL, 0)) == -1)
	{ 
		debugmsg("SETNONBLOCKING","get flags failed");
		return 0;
	}
	flags |= O_NONBLOCK;
	if (fcntl(socket, F_SETFL, flags) == -1)
	{
		debugmsg("SETNONBLOCKING","set flags failed");
		return 0;
	}
	return 1;
}

int setblocking(int socket)
{
	int flags;
	if((flags = fcntl(socket, F_GETFL, 0)) == -1)
	{ 
		debugmsg("SETBLOCKING","get flags failed");
		return 0;
	}
	flags &= ~O_NONBLOCK;
	if (fcntl(socket, F_SETFL, flags) == -1)
	{
		debugmsg("SETBLOCKING","set flags failed");
		return 0;
	}
	return 1;
}

void correctReply(string &in)
{
	string tmp;
	for(unsigned int i=0;i<in.length();i++)
	{
		if(in[i] != '\n')
		{
			tmp += in[i];
		}
		else
		{
			if(i>0)
			{
				if(in[i-1] != '\r')
				{
					tmp += '\r';
					tmp += in[i];
				}
				else
				{
					tmp += in[i];
				}
			}
			else if(i == 0)
			{
				tmp += '\r';
				tmp += in[i];
			}
		}
	}
	in = tmp;
}

int Bind(int &sock,string ip,int port)
{
	struct sockaddr_in adr;
	if (ip != "")
	{
		adr = GetIp(ip,port);
	}
	else
	{
		debugmsg("BIND","[Bind] bind to any adr");
		adr.sin_addr.s_addr = INADDR_ANY;
		adr.sin_port = htons(port);
		adr.sin_family = AF_INET;
		memset(&(adr.sin_zero), '\0', 8);
	}
	if(bind(sock,(struct sockaddr *)&adr, sizeof(struct sockaddr)) != 0)
	{
		debugmsg("BIND","[Bind] could not bind",errno);
		return 0;
	}
	return 1;
}

int Connect(int &sock,string host,int port,int sec,int &shouldquit)
{
	debugmsg("CONNECT","[Connect] start");
	
	struct sockaddr_in adr;
	adr = GetIp(host,port);
	if(!setnonblocking(sock))
	{
		debugmsg("CONNECT","[Connect] end(0-0)");
		return 0;
	}
	int rc;
	if((rc = connect(sock, (struct sockaddr *)&adr, sizeof(adr))) < 0)
	{
		if(errno != EINPROGRESS)
		{
			debugmsg("CONNECT","[Connect] end(0-1)");
			return 0;
		}
	}
	if (rc != 0) // ==0 -> connect completed immediately
	{
		fd_set writefds,readfds;
		for(int i=0; i < sec * 20;i++)
		{
			FD_ZERO(&writefds);
			FD_ZERO(&readfds);
			FD_SET(sock, &writefds);
			FD_SET(sock,&readfds);
			
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 50000;
			int res = select(sock+1, &readfds, &writefds, NULL, &tv);
			if (res < 0)
			{	
				debugmsg("CONNECT","[Connect] end(0-2)");		
				return 0;
			}
			else if(res == 0)
			{			
				if(shouldquit == 1) 
				{
					debugmsg("CONNECT","[Connect] end(0-3)");
					return 0;
				}
			}
			if(FD_ISSET(sock,&readfds) || FD_ISSET(sock,&writefds))
			{
				int err;
				socklen_t errlen = sizeof(err);
				if(getsockopt(sock,SOL_SOCKET,SO_ERROR,&err,&errlen) < 0)
				{
					debugmsg("CONNECT","[Connect] end(0-4)");
					return 0;
				}
				if(err)
				{
					debugmsg("CONNECT","[Connect] end(0-5)",err);
					return 0;
				}
				if(!SocketOption(sock,SO_KEEPALIVE))
				{
					debugmsg("CONNECT", "[Connect] client setsockopt error!",errno);
					return 0;
				}
				if(!setblocking(sock))
				{				
					return 0;
				}
				debugmsg("CONNECT","[Connect] end(1)");
				return 1;
			}
		}
	}
	else
	{
		int err;
		socklen_t errlen = sizeof(err);
		if(getsockopt(sock,SOL_SOCKET,SO_ERROR,&err,&errlen) < 0)
		{
			debugmsg("CONNECT","[Connect] end(0-4)");
			return 0;
		}
		if(err)
		{
			debugmsg("CONNECT","[Connect] end(0-5)");
			return 0;
		}
		if(!SocketOption(sock,SO_KEEPALIVE))
		{
			debugmsg("CONNECT", "[Connect] client setsockopt error!",errno);
			return 0;
		}
		if(!setblocking(sock))
		{				
			return 0;
		}
		debugmsg("CONNECT","[Connect] end(1)");
		return 1;
	}
	
	return 0;
}

int Accept(int listensock,int &newsock,string &clientip,int &clientport,int sec,int &shouldquit)
{
	debugmsg("ACCEPT","[Accept] start");
	fd_set readfds;
	struct sockaddr_in adr;
	
	if(sec > 0)
	{
		if(!setnonblocking(listensock))
		{
			return 0;
		}
		for(int i=0; i < sec * 20;i++)
		{
			FD_ZERO(&readfds);
			FD_SET(listensock, &readfds);
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 50000;
			int res;
			
			res =  select(listensock+1, &readfds, NULL, NULL, &tv);
			
			if (res < 0)
			{
				debugmsg("ACCEPT","[Accept] end(0-1)");		
				return 0;
			}
			else if(res == 0)
			{
				if (shouldquit == 1) 
				{
					debugmsg("ACCEPT","[Accept] end(0-2)");		
					return 0;
				}
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		// no timeout? block and wait
		if(!setblocking(listensock))
		{
			return 0;
		}
	}
	socklen_t size = sizeof(adr);
	if ((newsock = accept(listensock,(struct sockaddr *)&adr,&size)) == -1)
	{
		debugmsg("ACCEPT","[Accept] end(0-3)");		
		return 0;
	}
	clientip = inet_ntoa(adr.sin_addr);
	clientport = ntohs(adr.sin_port);
	if(!setblocking(newsock))
	{
		return 0;
	}
	if(!SocketOption(newsock,SO_KEEPALIVE))
	{
		debugmsg("ACCEPT", "[Accept] client setsockopt error!",errno);
		return 0;
	}
	debugmsg("ACCEPT","[Accept] end(1)");		
	return 1;
}

int SocketOption(int &sock,int option)
{
	int yes = 1;
	if (setsockopt(sock,SOL_SOCKET,option,&yes,sizeof(int)) != 0)
	{
		debugmsg("SOCKETOPTION","setsockopt error!");
		
		return 0;
	}
	return 1;
}




string traffic2str(double in)
{
	stringstream ss;
	//kb
	if((in / 1024) < 1024)
	{
		ss << setiosflags(ios_base::fixed) << setprecision(2) << (in / 1024) << " KB";
	}
	//mb
	else if ((in / 1024 / 1024) < 1024)
	{
		ss << setiosflags(ios_base::fixed) << setprecision(2) << (in / 1024 / 1024) << " MB";
	}
	//gb
	else if ((in / 1024 / 1024 / 1024) < 1024)
	{
		ss << setiosflags(ios_base::fixed) << setprecision(2) << (in / 1024 / 1024 / 1024) << " GB";
	}
	//tb
	else 
	{
		ss << setiosflags(ios_base::fixed) << setprecision(2) << (in / 1024 / 1024 / 1024 / 1024) << " TB";
	}
	return ss.str();
}

int IsEndline(string tmp)
{
	// check if it is last line
	unsigned int pos1,pos2;
	pos1 = tmp.rfind('\r',tmp.length());
	
	if (pos1 == string::npos)
	{
		// we never should get here
		return 0;
	}
	
						
	// check if message has only 1 line
	pos2 = tmp.rfind('\r',pos1-1);
	if (pos2 != string::npos)
	{
		//multiple lines
		
		if(tmp[pos2+5] != '-' && tmp[pos2+4] != ' ')
		{			
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		// only one line
		if(tmp[3] != '-')
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
}

struct sockaddr_in GetIp(string ip,int port)
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	
	if(!inet_aton(ip.c_str(),&addr.sin_addr))
	{
		struct hostent *he;
	
		if((he = gethostbyname(ip.c_str())) == NULL)
		{
			//cout << "Error resolving ip\n";
			
		}
		else
		{
			addr.sin_addr = *(struct in_addr*)he->h_addr;
		}
	}
	addr.sin_port = htons(port);
	memset(&(addr.sin_zero), '\0', 8);
	return addr;
}

int Ident(string ip, int clientport, int listenport, string connectip, string &reply,int timeout)
{
	int ident_sock = -1;
	int shouldquit = 0;
	
	string ident_reply;
	reply = "*";
	if((ident_sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		return 0;
	}
	
	if (connectip != "")
	{
		if(!Bind(ident_sock,connectip,0))
		{				
			debugmsg("IDENT","[Ident] could not bind",errno);
			Close(ident_sock,"ident_sock");
			return 0;
		}
	}
	
	if (Connect(ident_sock,ip,113,timeout,shouldquit) )
	{				
		debugmsg("IDENT","[Ident] try to read ident reply");
		
		
		stringstream ss;
		ss << clientport << " , " << listenport << "\r\n";
		debugmsg("IDENT","ident msg: '" + ss.str() + "'");
		if (!control_write(ident_sock,ss.str(),NULL))
		{	
			debugmsg("IDENT","ident write failed");
			Close(ident_sock,"ident_sock");				
			return 0;		
		}
				
		if(!control_read(ident_sock,NULL,ident_reply))
		{	
			debugmsg("IDENT","ident read failed");
			Close(ident_sock,"ident_sock");								
			return 0;					
		}
		debugmsg("IDENT","[Ident] ident: " + ident_reply);
			
		Close(ident_sock,"ident_sock");
	}
	else
	{
		debugmsg("IDENT","[Ident] could not connect to ident port @"+ip);
		Close(ident_sock,"ident_sock");		
		return 0;		
	}
	string idnt,ident_user;
	idnt=ident_reply.substr(ident_reply.rfind(":",ident_reply.length())+1);
	idnt=idnt.substr(0,idnt.find("\r"));
	ident_user = trim(idnt);
	if (ident_user[ident_user.length() - 1] == '\n') { ident_user = ident_user.substr(0,ident_user.length() - 1); }
	Close(ident_sock,"ident_sock");	
	reply = ident_user;
	return 1;
}

// send string
int control_write(int sock,string s,SSL *sslcon)
{	
		
	stringstream ss;
	ss << s.length();
		
	fd_set writefds;
	struct timeval tv;

	int maxsize = config.buffersize; // max size in bytes of packet
	int total = 0;
	int count = 0;
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
				if(count == config.retry_count) { debugmsg("CONTROLWRITE","retry count reached"); return 0; } // not more then x retries
				int err = SSL_get_error(sslcon,n);
				
				if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_X509_LOOKUP) 
				{
					debugmsg("CONTROLWRITE","want read/write error");
					fd_set data_writefds;
					FD_ZERO(&data_writefds);
					FD_SET(sock,&data_writefds);
					struct timeval tv;
					tv.tv_sec = config.read_write_timeout;
					tv.tv_usec = 0;
					if (select(sock+1, NULL, &data_writefds, NULL, &tv) < 1)
					{
						debugmsg("CONTROLWRITE"," write timeout",errno);
						return 0;
					}
					if (FD_ISSET(sock, &data_writefds))
					{
						count++; 
						continue; 
					}
					else
					{
						debugmsg("CONTROLWRITE"," fd isser error",errno);
						return 0;
					} 
				}
				else
				{
					debugmsg("CONTROLWRITE","SSL error",errno);
					return 0;
				}
				
				
			}
			
			debugmsg("CONTROLWRITE","write error",errno);
			return 0;
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
	
	if (bytesleft == 0)
	{		
		
		return 1;
	}
	else
	{		
		
		return 0;
	}
}

// read string
int control_read(int sock,SSL *sslcon,string &str)
{
	debugmsg("CONTROLREAD","start");
	string tmp = "";
	fd_set readfds;
	struct timeval tv;
	int rc;
	char *buffer;
	buffer = new char[config.buffersize];
	int count = 0;
	
	while(1)
	{	
		debugmsg("CONTROLREAD","loop start");	
		for (int i=0;i<config.buffersize;i++) { buffer[i] = 0; }
		tv.tv_sec = config.read_write_timeout;
		tv.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(sock,&readfds);
		if (select(sock+1,&readfds,NULL,NULL,&tv) == -1)
		{
			debugmsg("CONTROLREAD","select error");
			str = tmp;
			
			stringstream ss;
			ss << str.length();
			
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
				delete [] buffer;
				return 0;
			}
			else if (rc < 0)
			{
				if (sslcon != NULL)
				{
					if(count == config.retry_count) { debugmsg("CONTROLREAD","retry count reached"); return 0; } // not more then x retries
					int err = SSL_get_error(sslcon,rc);
					
					if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_X509_LOOKUP) 
					{ 
						debugmsg("CONTROLREAD","want read/write error");
						fd_set data_writefds;
						FD_ZERO(&data_writefds);
						FD_SET(sock,&data_writefds);
						struct timeval tv;
						tv.tv_sec = config.read_write_timeout;
						tv.tv_usec = 0;
						if (select(sock+1, NULL, &data_writefds, NULL, &tv) < 1)
						{
							debugmsg("CONTROLREAD"," write timeout",errno);
							delete [] buffer;
							return 0;
						}
						if (FD_ISSET(sock, &data_writefds))
						{
							count++; 
							continue; 
						}
						else
						{
							debugmsg("CONTROLREAD"," fd isser error",errno);
							delete [] buffer;
							return 0;
						} 
					}
					else
					{
						debugmsg("CONTROLWRITE","SSL error",errno);
						delete [] buffer;
						return 0;
					}
					
					
				}
				debugmsg("CONTROLWRITE","read error",errno);
				delete [] buffer;
				return 0;
			}
			else
			{	
				debugmsg("CONTROLREAD","else path");			
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
		else
		{
			debugmsg("CONTROLREAD","socket not ready - timeout");
			return 0;
		}
	}
}

int SslConnect(int &sock,SSL **ssl,SSL_CTX **sslctx)
{
	if(!setnonblocking(sock))
	{				
		return 0;
	}
	debugmsg("SSLCONNECT", "[SslConnect] start");
	*sslctx = SSL_CTX_new(TLSv1_client_method());
	
	if (*sslctx == NULL)
	{
		debugmsg("SSLCONNECT", "[SslConnect] sitesslctx failed!");
		return 0;
	}
	SSL_CTX_set_options(*sslctx,SSL_OP_ALL);
	SSL_CTX_set_mode(*sslctx,SSL_MODE_AUTO_RETRY);
	SSL_CTX_set_session_cache_mode(*sslctx,SSL_SESS_CACHE_OFF);
	
	*ssl = SSL_new(*sslctx);
	if (*ssl == NULL)
	{
		debugmsg("SSLCONNECT", "[SslConnect] site ssl failed!");
		return 0;
	}
	if(SSL_set_fd(*ssl,sock) == 0)
	{
		debugmsg("SSLCONNECT", "[SslConnect] ssl set fd failed!");
		debugmsg("SSLCONNECT","[SslConnect] " +  (string)ERR_error_string(ERR_get_error(), NULL));
		return 0;
	}
	debugmsg("SSLCONNECT","[SslConnect] try to connect...");
	// try for 10 seconds
	for(int i=0; i <200;i++)
	{
		int err = SSL_connect(*ssl);
		if(err == 1)
		{
			break;
		}
		else
		{
			int sslerr = SSL_get_error(*ssl, err);
			if( sslerr == SSL_ERROR_WANT_READ || sslerr == SSL_ERROR_WANT_WRITE || sslerr == SSL_ERROR_WANT_X509_LOOKUP)
			{
				
				usleep(50000);
			}
			else
			{
				debugmsg("SSLCONNECT", "[SslConnect] TLS Connection failed!");
				debugmsg("SSLCONNECT","[SslConnect] " +  (string)ERR_error_string(ERR_get_error(), NULL));
				return 0;
			}
		}
		
	}
	
	debugmsg("SSLCONNECT", "[SslConnect] end");
	if(!setblocking(sock))
	{				
		return 0;
	}
	return 1;
}

int SslAccept(int &sock,SSL **ssl,SSL_CTX **sslctx)
{	
	if(!setnonblocking(sock))
	{				
		return 0;
	}
	debugmsg("SSLACCEPT", "[SslAccept] start");
	*ssl = SSL_new(*sslctx);
	if (*ssl == NULL)
	{
		debugmsg("SSLACCEPT", "[SslAccept] clientssl failed!");
		return 0;
	}

	if (SSL_set_fd(*ssl,sock) == 0)
	{
		debugmsg("SSLACCEPT","[SslAccept] " +  (string)ERR_error_string(ERR_get_error(), NULL));
		return 0;
	}
	
	
	
	debugmsg("SSLACCEPT","[SslAccept] try ssl accept");
	for(int i=0; i <200;i++)
	{
		int err = SSL_accept(*ssl);
		if(err == 1)
		{
			break;
		}
		else
		{
			int sslerr = SSL_get_error(*ssl, err);
			if( sslerr == SSL_ERROR_WANT_READ || sslerr == SSL_ERROR_WANT_WRITE || sslerr == SSL_ERROR_WANT_X509_LOOKUP)
			{
				
				usleep(50000);
			}
			else
			{
				debugmsg("SSLACCEPT", "[SslAccept] TLS Connection failed!");
				debugmsg("SSLACCEPT","[SslAccept] " +  (string)ERR_error_string(ERR_get_error(), NULL));
				return 0;
			}
		}
		
	}
	if(!setblocking(sock))
	{				
		return 0;
	}	
	debugmsg("SSLACCEPT", "[SslAccept] end");
	return 1;
	
}

int ParsePortCommand(string portcmd,string &ip,int &port)
{
	debugmsg("-SYSTEM-","[ParsePortCommand] start");
	
	unsigned int startpos;
		
	startpos = portcmd.find(" ",0);
	if (startpos == string::npos) { return 0; }
	portcmd = portcmd.substr(startpos+1,portcmd.length());
	startpos = portcmd.find(",",0);
	if (startpos == string::npos) { return 0; }
	portcmd.replace(startpos,1,".");
	startpos = portcmd.find(",",0);
	if (startpos == string::npos) { return 0; }
	portcmd.replace(startpos,1,".");
	startpos = portcmd.find(",",0);
	if (startpos == string::npos) { return 0; }
	portcmd.replace(startpos,1,".");
	startpos = portcmd.find(",",0);
	if (startpos == string::npos) { return 0; }
	ip = portcmd.substr(0,startpos);
	
		
	string tmpport;
	tmpport = portcmd.substr(startpos+1,portcmd.length());
	startpos = tmpport.find(",",0);
	if (startpos == string::npos) { return 0; }
	string p1,p2;
	p1 = tmpport.substr(0,startpos);
	p2 = tmpport.substr(startpos+1,tmpport.length()-1);
	if(p1 == "") return 0;
	if(p2 == "") return 0;
	port = 256 * atoi(p1.c_str()) + atoi(p2.c_str());
	debugmsg("-SYSTEM-","[ParsePortCommand] end");
	return 1;
	
}

int ParsePsvCommand(string passivecmd,string &ip, int &port)
{

	debugmsg("-SYSTEM-","[ParsePsvCommand] start");
	debugmsg("-SYSTEM-","[ParsePsvCommand] " + passivecmd);
	unsigned int startpos,endpos;
	startpos = passivecmd.find("(",0);
	endpos = passivecmd.find(")",0);
	if (startpos == string::npos || endpos == string::npos)
	{		
		return 0;
	}
	
	// split passive mode string
	string tmp = passivecmd.substr(1+startpos,endpos-startpos-1);

	if (tmp == "") return 0;
	
	endpos = tmp.find(",",0);
	if(endpos == string::npos) return 0;
	string ip1 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	endpos = tmp.find(",",0);
	if(endpos == string::npos) return 0;
	string ip2 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	endpos = tmp.find(",",0);
	if(endpos == string::npos) return 0;
	string ip3 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	endpos = tmp.find(",",0);
	if(endpos == string::npos) return 0;
	string ip4 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	ip = ip1 + "." + ip2 + "." + ip3 + "." + ip4;

	endpos = tmp.find(",",0);
	if(endpos == string::npos) return 0;
	string port1 = tmp.substr(0,endpos);
	if(port1 == "") return 0;
	string port2 = tmp.substr(endpos+1,tmp.length());
	if(port2 == "") return 0;
	port = (atoi(port1.c_str()) * 256 + atoi(port2.c_str()));
	
	debugmsg("-SYSTEM-","[ParsePsvCommand] end");
	return 1;
}

int DataWrite(int sock,char *data,int nrbytes,SSL *ssl)
{
	
	int total = 0;
	int bytesleft = nrbytes;
	int rc,len;
	len = nrbytes;
	int count = 0;
	
	fd_set data_writefds;
	FD_ZERO(&data_writefds);
	FD_SET(sock,&data_writefds);
	struct timeval tv;
	tv.tv_sec = config.read_write_timeout;
	tv.tv_usec = 0;
	if (select(sock+1, NULL, &data_writefds, NULL, &tv) < 1)
	{
		debugmsg("DATAWRITE"," write timeout",errno);
		return 0;
	}
	if (FD_ISSET(sock, &data_writefds))
	{
				
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
					debugmsg("DATAWRITE","[data_write] connection closed",errno);
					return 0;
				}
				else
				{
					if(count == config.retry_count) { debugmsg("DATAWRITE","retry count reached"); return 0; } // not more then x retries
					if (ssl != NULL)
					{
						
						int err = SSL_get_error(ssl,rc);
						
						if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_X509_LOOKUP) 
						{ 
							debugmsg("DATAWRITE","want read/write error");
							fd_set data_writefds;
							FD_ZERO(&data_writefds);
							FD_SET(sock,&data_writefds);
							struct timeval tv;
							tv.tv_sec = config.read_write_timeout;
							tv.tv_usec = 0;
							if (select(sock+1, NULL, &data_writefds, NULL, &tv) < 1)
							{
								debugmsg("DATAWRITE"," write timeout",errno);
								return 0;
							}
							if (FD_ISSET(sock, &data_writefds))
							{
								count++; 
								continue; 
							}
							else
							{
								debugmsg("DATAWRITE"," fd isser error",errno);
								return 0;
							}
						}
						else
						{
							debugmsg("DATAWRITE","SSL error",errno);
							return 0;
						}
							
						
					}
					
					debugmsg("DATAWRITE","[data_write] error!",errno);  
					return 0; 
				}
			}
		
		
		
	
		return 1;	
	}
	else
	{
		debugmsg("DATAWRITE"," fdset error",errno);
		return 0;
	}
}

int DataRead(int sock ,char *buffer,int &nrbytes,SSL *ssl,int tt,int ussl)
{
	int count = 0;
	while(1)
	{
			int rc;		
			if (ssl == NULL)
			{
				if((tt == 1) && config.ssl_ascii_cache && ussl)
				{
					int sslasciiread = 0;
		      int rc = 1;
		      while (rc > 0 && (sslasciiread < config.buffersize))
		      {  
		      	debugmsg("DATAREAD","[data_read] ssl_ascii_cache mode");    	
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
				debugmsg("DATAREAD","[data_read] connection closed",errno);
				nrbytes=0; 
				return 0; 
			}
			else
			{	
				if(count == config.retry_count) { debugmsg("-SYSTEM-","retry count reached"); return 0; } // not more then x retries
				if (ssl != NULL)
				{
					int err = SSL_get_error(ssl,rc);
					
					if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_X509_LOOKUP) 
					{
						debugmsg("DATAREAD","want read/write error");
						fd_set data_readfds;
						FD_ZERO(&data_readfds);
						FD_SET(sock,&data_readfds);
						struct timeval tv;
						tv.tv_sec = config.read_write_timeout;
						tv.tv_usec = 0;
						if (select(sock+1, &data_readfds, NULL, NULL, &tv) < 1)
						{
							debugmsg("DATAREAD"," write timeout",errno);
							return 0;
						}
						if (FD_ISSET(sock, &data_readfds))
						{
							count++; 
							continue; 
						}
						else
						{
							debugmsg("DATAREAD"," fd isser error",errno);
							return 0;
						}
					}
					else
					{
						debugmsg("DATAREAD","SSL error",errno);
						return 0;
					}
					
				}
				
				debugmsg("-SYSTEM-","[data_read] error!"); 
				nrbytes=0; 
				return 0; 
			}
		}
  	
		
	return 0;
}

int printsockopt(int sock,string name)
{
	debugmsg("SOCKOPT","----- options for " + name + "  -----------");
	int optval;
	socklen_t optlen = sizeof(int);
	optval = 0;
	getsockopt(sock,SOL_SOCKET,SO_KEEPALIVE,(char *) &optval, &optlen);
	if(optval)
	{
		debugmsg("SOCKOPT","socket is set to keepalive");
	}
	else
	{
		debugmsg("SOCKOPT","socket is not set to keepalive");
	}
	
	getsockopt(sock,SOL_SOCKET,SO_LINGER,(char *) &optval, &optlen);
	if(optval)
	{
		debugmsg("SOCKOPT","socket is set to linger");
	}
	else
	{
		debugmsg("SOCKOPT","socket is not set to linger");
	}
	
	getsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char *) &optval, &optlen);
	if(optval)
	{
		debugmsg("SOCKOPT","socket is set to reuse adr");
	}
	else
	{
		debugmsg("SOCKOPT","socket is not set to reuse adr");
	}
	int flags;
	if((flags = fcntl(sock, F_GETFL, 0)) == -1)
	{ 
		return 0;
	}
	if(flags & O_NONBLOCK)
	{
		debugmsg("SOCKOPT","sock set to non blocking");
	}
	else
	{
		debugmsg("SOCKOPT","sock set to blocking");
	}
	return 1;
}

void PrintSock(int sock,string desc)
{
	stringstream ss;
	ss << "-----SOCKET---- " << desc << " : " << sock;
	debugmsg("",ss.str());
}

int GetSock(int &sock)
{
	sock_lock.Lock();
	if((sock = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		sock = -1;
		sock_lock.UnLock();
		return 0;
	}
	sock_lock.UnLock();
	return 1;
}

int Close(int &sock,string desc)
{
	sock_lock.Lock();
	stringstream ss;
	ss << "-----SOCKET---- closing " << desc << " : " << sock;
	debugmsg("",ss.str());
	if(sock > 0)
	{		
		if(close(sock) != -1)
		{
			sock = -1;
			sock_lock.UnLock();
			return 1;
		}
		else
		{
			sock = -1;
			sock_lock.UnLock();
			return 0;
		}
	}
	else
	{
		sock = -1;
		sock_lock.UnLock();
		return 1;
	}
}

int filesize(string filename,int &s)
{
	ifstream ifile(filename.c_str(),ios::binary | ios::in);
	if (!ifile)
	{
		return 0;
	}
	int start,end;
	start = ifile.tellg();
	ifile.seekg(0,ios::end);
	end = ifile.tellg();
	ifile.seekg(0,ios::beg);
	s = end-start;
	return 1;
}

unsigned char *readfile(string filename,int s)
{
	ifstream ifile(filename.c_str(),ios::binary | ios::in);
	unsigned char *tmp;
	tmp = new unsigned char [s];
	ifile.read((char*)tmp,s);
	ifile.close();
	return tmp;
}

int writefile(string filename,unsigned char *data,int s)
{
	ofstream ofile(filename.c_str(),ios::binary | ios::out | ios::trunc);
	if (!ofile)
	{
		return 0;
	}
	ofile.write((char*)data,s);
	ofile.close();
	return 1;
}

int decrypt(string key,unsigned char *datain,unsigned char *dataout,int s)
{
	unsigned char ivec[8];
	memset(ivec,0, 8);
	int ipos = 0;
	int outlen = s;

	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);
    EVP_CipherInit_ex(&ctx, EVP_bf_cfb(), NULL, NULL, NULL,ipos );
    EVP_CIPHER_CTX_set_key_length(&ctx, key.length());
    EVP_CipherInit_ex(&ctx, NULL, NULL,(unsigned char*)key.c_str(), ivec,ipos );

	if(!EVP_CipherUpdate(&ctx, dataout, &outlen, datain, s))
	{
		return 0;
	}

 	EVP_CIPHER_CTX_cleanup(&ctx);
 	for (unsigned int i=0;i<key.length();i++) { key[i] = '0'; }
        return 1;
}

int encrypt(string key,unsigned char *datain,unsigned char *dataout,int s)
{
	unsigned char ivec[8];
	memset(ivec, 0,8);
	int outlen = s;

	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);
        EVP_EncryptInit_ex(&ctx, EVP_bf_cfb(), NULL, NULL, NULL );
        EVP_CIPHER_CTX_set_key_length(&ctx, key.length());
        EVP_EncryptInit_ex(&ctx, NULL, NULL, (unsigned char*)key.c_str(), ivec );

	if(!EVP_EncryptUpdate(&ctx, dataout, &outlen, datain, s))
	{
		return 0;
	}

 	EVP_CIPHER_CTX_cleanup(&ctx);
 	for (unsigned int i=0;i<key.length();i++) { key[i] = '0'; }
        return 1;
}

int GetLine(int sock,SSL **ssl,string &reply)
{
	string rep="";
	while(!IsEndline(rep))
	{
		string tmp;
		
		if(!control_read(sock,*ssl,tmp))
		{
		   debugmsg("GETLINE","read error");
		   return 0;
		}
		rep += tmp;
				
	}	
	reply = rep;
	return 1;
}

int Login(int &sock,string ip,int port,string user,string pass,int usessl,SSL **ssl,SSL_CTX **sslctx,string &message)
{
	unsigned int pos;
	
	int shouldquit = 0;
	if (!Connect(sock,ip,port,5,shouldquit))
	{		
		debugmsg("LOGIN","can't connect");
		message = "could not connect";
		return 0;
	}
	debugmsg("LOGIN","connected");
	string reply = "";
	if(!GetLine(sock,ssl,reply))
	{
		message = "read error";
		return 0;
	}
	
	if(usessl)
	{
		if(!control_write(sock,"AUTH TLS\r\n",*ssl))
		{
		   debugmsg("LOGIN","write error");
		   message = "write error";
			return 0;
		}
		reply = "";
		
		if(!GetLine(sock,ssl,reply))
		{
			message = "read error";
			return 0;
		}
		
		pos = reply.find("AUTH TLS successful",0);
		if(pos == string::npos)
		{
		    message = "AUTH TLS failed";	
			return 0;
		}	
		if(!SslConnect(sock,ssl,sslctx))
		{
		   
			return 0;
		}
	}
	if(!control_write(sock,"user " + user + "\r\n",*ssl))
	{
	   debugmsg("LOGIN","write error");
	   message = "write error";
		return 0;
	}
	reply = "";
	if(!GetLine(sock,ssl,reply))
	{
		message = "read error";
		return 0;
	}
	// standard gl message
	pos = reply.find("331 Password required for",0);
	if(pos == string::npos)
	{
		// standard anonymous ftp message
		pos = reply.find("login ok",0);
		if(pos == string::npos)
		{
			pos = reply.find("specify the password",0);
			if(pos == string::npos)
			{ 	
				message = "login failed";
				return 0;      
			}
		}
	}
	if(!control_write(sock,"pass " + pass + "\r\n",*ssl))
	{
		debugmsg("LOGIN","write error");
		message = "write error";
		return 0;
	}
	reply = "";
	if(!GetLine(sock,ssl,reply))
	{
		message = "read error";
		return 0;
	}
	//standard gl message	
	pos = reply.find("logged in.",0);
	if(pos == string::npos)
	{		
		// standard anonymous ftp message
		pos = reply.find("access granted",0);
		if(pos == string::npos)
		{
			pos = reply.find("Login successful",0);
			if(pos == string::npos)
			{
				message = "login failed";  	
				return 0;
			}
		}
	}
   if(usessl)
   {
   		message = "SSL login successfull";
   }
   else
   {
   		message = "socket not successfull";
   }
	return 1;
}

int trafficcheck(void)
{
	if((config.day_limit == 0) && (config.week_limit == 0) && (config.month_limit == 0))
	{
		debugmsg("-SYSTEM-","using no traffic limit");
		return 1;
	}
	else
	{
		if(config.day_limit != 0)
		{
			if(daycounter.gettotal() >= config.day_limit * 1024 * 1024 * 1024)
			{
				debugmsg("-SYSTEM-","day traffic limit reached");
				return 0;
			}
		}
		if(config.week_limit != 0)
		{
			if(weekcounter.gettotal() >= config.week_limit * 1024 * 1024 * 1024)
			{
				debugmsg("-SYSTEM-","week traffic limit reached");
				return 0;
			}
		}
		if(config.month_limit != 0)
		{
			if(monthcounter.gettotal() >= config.month_limit * 1024 * 1024 * 1024)
			{
				debugmsg("-SYSTEM-","month traffic limit reached");
				return 0;
			}
		}
		debugmsg("-SYSTEM-","no traffic limit reached");
		return 1;
	}
}



#if defined(__linux__) && defined(__i386__)
pid_t gettid(void)
{
    pid_t ret;
    __asm__("int $0x80" : "=a" (ret) : "0" (224) /* SYS_gettid */);
    if (ret < 0) ret = -1;
    return ret;

}

#endif
