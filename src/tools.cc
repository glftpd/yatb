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

   int idx = str.find_first_not_of(whitespace);
   if( idx != (int)string::npos )
       return str.substr(idx);
   
   return "";
}

string rtrim( const string &str, const string &whitespace)
{

   int idx = str.find_last_not_of(whitespace);
   if( idx != (int)string::npos )
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
  double r = ((double)rand() / ((double)(RAND_MAX)+1) );
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
	for(int i=0;i < (int)in.length();i++)
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

int Connect(int &sock,string host,int port,int sec,int &shouldquit,struct sockaddr_in &retadr)
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
				retadr = adr;
				//cout << "connect ip: " << inet_ntoa(adr.sin_addr) << "\n";
				//cout << "connect port: " << adr.sin_port << "\n";
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
		retadr = adr;
		//cout << "connect ip: " << inet_ntoa(adr.sin_addr) << "\n";
		//cout << "connect port: " << adr.sin_port << "\n";
		debugmsg("CONNECT","[Connect] end(1)");
		return 1;
	}
	
	return 0;
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
				
				//cout << "connect ip: " << inet_ntoa(adr.sin_addr) << "\n";
				//cout << "connect port: " << adr.sin_port << "\n";
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
		
		//cout << "connect ip: " << inet_ntoa(adr.sin_addr) << "\n";
		//cout << "connect port: " << adr.sin_port << "\n";
		debugmsg("CONNECT","[Connect] end(1)");
		return 1;
	}
	
	return 0;
}


int Connect5(int &sock,string host,int port, string socksip, int socksport, string socksuser, string sockspass, int sec,int &shouldquit)
{
	debugmsg("CONNECT5","[Connect] start");
	fd_set writefds,readfds;
	struct sockaddr_in adr;
	adr = GetIp(socksip,socksport);
	if(!setnonblocking(sock))
	{
		debugmsg("CONNECT5","[Connect] end(0-0)");
		return 0;
	}
	int rc;
	if((rc = connect(sock, (struct sockaddr *)&adr, sizeof(adr))) < 0)
	{
		if(errno != EINPROGRESS)
		{
			debugmsg("CONNECT5","[Connect] end(0-1)");
			return 0;
		}
	}
	if (rc != 0) // ==0 -> connect completed immediately
	{
		
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
				debugmsg("CONNECT5","[Connect] end(0-2)");		
				return 0;
			}
			else if(res == 0)
			{			
				if(shouldquit == 1) 
				{
					debugmsg("CONNECT5","[Connect] end(0-3)");
					return 0;
				}
			}
			if(FD_ISSET(sock,&readfds) || FD_ISSET(sock,&writefds))
			{
				int err;
				socklen_t errlen = sizeof(err);
				if(getsockopt(sock,SOL_SOCKET,SO_ERROR,&err,&errlen) < 0)
				{
					debugmsg("CONNECT5","[Connect] end(0-4)");
					return 0;
				}
				if(err)
				{
					debugmsg("CONNECT5","[Connect] end(0-5)",err);
					return 0;
				}
				if(!SocketOption(sock,SO_KEEPALIVE))
				{
					debugmsg("CONNECT5", "[Connect] client setsockopt error!",errno);
					return 0;
				}
				if(!setblocking(sock))
				{				
					return 0;
				}
				// here socks5 connect starts
				char *buffer;
				buffer = new char[config.buffersize];
				buffer[0] = 5;
				buffer[1] = 1;
				buffer[2] = 2;
				
				if(!DataWrite(sock,buffer,3,NULL))
				{
					debugmsg("-CONNECT5-","error writing to client");
					delete [] buffer;
					return 0;
				}

				for(int i=0; i < config.read_write_timeout * 2;i++)
				{
					FD_ZERO(&readfds);
					FD_SET(sock,&readfds);
					
					struct timeval tv;
					tv.tv_sec = 0;
					tv.tv_usec = 500000;
					
					if (select(sock+1, &readfds, NULL, NULL, &tv) > 1)
					{					
						break;
					}
					
				}
				
				if (FD_ISSET(sock, &readfds))
				{
					if(!DataRead(sock,buffer,rc,NULL,0,0))
					{					
						debugmsg("-CONNECT5-","data read failed");
						Close(sock,"");
						delete [] buffer;
						return 0;
					}
					if(rc != 2)
					{
						debugmsg("-CONNECT5-","illegal response");
						Close(sock,"");
						delete [] buffer;
						return 0;
					}
					if(buffer[0] != 5 || buffer[1] != 2)
					{
						debugmsg("-CONNECT5-","illegal response");
						Close(sock,"");
						delete [] buffer;
						return 0;
					}
					if(socksuser.length() > 255 || sockspass.length() > 255)
					{
						debugmsg("-CONNECT5-","illegal user/pass length");
						Close(sock,"");
						delete [] buffer;
						return 0;
					}
					buffer[0] = 1;
					buffer[1] = socksuser.length();
					for(int i=0;i < (int)socksuser.length();i++)
					{
						buffer[i+2] = socksuser[i];
					}
					buffer[socksuser.length() + 2] = sockspass.length();
					for(int i=0;i < (int)sockspass.length();i++)
					{
						buffer[i+3+socksuser.length()] = sockspass[i];
					}
					if(!DataWrite(sock,buffer,3 + socksuser.length() + sockspass.length(),NULL))
					{
						debugmsg("-CONNECT5-","error writing to client");
						Close(sock,"");
						delete [] buffer;
						return 0;
					}
					for(int i=0; i < config.read_write_timeout * 2;i++)
					{
						FD_ZERO(&readfds);
						FD_SET(sock,&readfds);
						
						struct timeval tv;
						tv.tv_sec = 0;
						tv.tv_usec = 500000;
						
						if (select(sock+1, &readfds, NULL, NULL, &tv) > 1)
						{					
							break;
						}
						
					}
					if (FD_ISSET(sock, &readfds))
					{
						if(!DataRead(sock,buffer,rc,NULL,0,0))
						{					
							debugmsg("-CONNECT5-","data read failed");
							Close(sock,"");
							delete [] buffer;
							return 0;
						}
						if(rc != 2)
						{
							debugmsg("-CONNECT5-","data read failed");
							Close(sock,"");
							delete [] buffer;
							return 0;
						}
						if(buffer[0] != 1 || buffer[1] != 0)
						{
							debugmsg("-CONNECT5-","auth failed");
							Close(sock,"");
							delete [] buffer;
							return 0;
						}
						// now send connect request
						buffer[0] = 5;
						buffer[1] = 1;
						buffer[2] = 0;
						buffer[3] = 1;

						struct sockaddr_in newadr;
						newadr = GetIp(host,port);

						memcpy(buffer + 4, &newadr.sin_addr,4);
						memcpy(buffer + 8, &newadr.sin_port,2);

						if(!DataWrite(sock,buffer,10 + socksuser.length() + sockspass.length(),NULL))
						{
							debugmsg("-CONNECT5-","error writing to client");
							Close(sock,"");
							delete [] buffer;
							return 0;
						}

						for(int i=0; i < config.read_write_timeout * 2;i++)
						{
							FD_ZERO(&readfds);
							FD_SET(sock,&readfds);
							
							struct timeval tv;
							tv.tv_sec = 0;
							tv.tv_usec = 500000;
							
							if (select(sock+1, &readfds, NULL, NULL, &tv) > 1)
							{					
								break;
							}
							
						}
						if (FD_ISSET(sock, &readfds))
						{
							if(!DataRead(sock,buffer,rc,NULL,0,0))
							{					
								debugmsg("-CONNECT5-","data read failed");
								Close(sock,"");
								delete [] buffer;
								return 0;
							}
							if(rc != 10)
							{
								debugmsg("-CONNECT5-","data read failed");
								Close(sock,"");
								delete [] buffer;
								return 0;
							}
						}
					}
					else
					{	
						debugmsg("-CONNECT5-","data read failed");
						Close(sock,"");
						delete [] buffer;
						return 0;				
					}

				}
				else
				{
					debugmsg("-CONNECT5-","data read failed");
					Close(sock,"");
					delete [] buffer;
					return 0;
				}
				// here socks5 connect ends
				debugmsg("CONNECT5","[Connect] end(1)");
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
			debugmsg("CONNECT5","[Connect] end(0-4)");
			return 0;
		}
		if(err)
		{
			debugmsg("CONNECT5","[Connect] end(0-5)");
			return 0;
		}
		if(!SocketOption(sock,SO_KEEPALIVE))
		{
			debugmsg("CONNECT5", "[Connect] client setsockopt error!",errno);
			return 0;
		}
		if(!setblocking(sock))
		{				
			return 0;
		}
		// here socks5 connect starts
		char *buffer;
		buffer = new char[config.buffersize];
		buffer[0] = 5;
		buffer[1] = 1;
		buffer[2] = 2;
		
		if(!DataWrite(sock,buffer,3,NULL))
		{
			debugmsg("-CONNECT5-","error writing to client");
			delete [] buffer;
			return 0;
		}

		for(int i=0; i < config.read_write_timeout * 2;i++)
		{
			FD_ZERO(&readfds);
			FD_SET(sock,&readfds);
			
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 500000;
			
			if (select(sock+1, &readfds, NULL, NULL, &tv) > 1)
			{					
				break;
			}
			
		}
		
		if (FD_ISSET(sock, &readfds))
		{
			if(!DataRead(sock,buffer,rc,NULL,0,0))
			{					
				debugmsg("-CONNECT5-","data read failed");
				Close(sock,"");
				delete [] buffer;
				return 0;
			}
			if(rc != 2)
			{
				debugmsg("-CONNECT5-","illegal response");
				Close(sock,"");
				delete [] buffer;
				return 0;
			}
			if(buffer[0] != 5 || buffer[1] != 2)
			{
				debugmsg("-CONNECT5-","illegal response");
				Close(sock,"");
				delete [] buffer;
				return 0;
			}
			if(socksuser.length() > 255 || sockspass.length() > 255)
			{
				debugmsg("-CONNECT5-","illegal user/pass length");
				Close(sock,"");
				delete [] buffer;
				return 0;
			}
			buffer[0] = 1;
			buffer[1] = socksuser.length();
			for(int i=0;i < (int)socksuser.length();i++)
			{
				buffer[i+2] = socksuser[i];
			}
			buffer[socksuser.length() + 2] = sockspass.length();
			for(int i=0;i < (int)sockspass.length();i++)
			{
				buffer[i+3+socksuser.length()] = sockspass[i];
			}
			if(!DataWrite(sock,buffer,3 + socksuser.length() + sockspass.length(),NULL))
			{
				debugmsg("-CONNECT5-","error writing to client");
				Close(sock,"");
				delete [] buffer;
				return 0;
			}
			for(int i=0; i < config.read_write_timeout * 2;i++)
			{
				FD_ZERO(&readfds);
				FD_SET(sock,&readfds);
				
				struct timeval tv;
				tv.tv_sec = 0;
				tv.tv_usec = 500000;
				
				if (select(sock+1, &readfds, NULL, NULL, &tv) > 1)
				{					
					break;
				}
				
			}
			if (FD_ISSET(sock, &readfds))
			{
				if(!DataRead(sock,buffer,rc,NULL,0,0))
				{					
					debugmsg("-CONNECT5-","data read failed");
					Close(sock,"");
					delete [] buffer;
					return 0;
				}
				if(rc != 2)
				{
					debugmsg("-CONNECT5-","data read failed");
					Close(sock,"");
					delete [] buffer;
					return 0;
				}
				if(buffer[0] != 1 || buffer[1] != 0)
				{
					debugmsg("-CONNECT5-","auth failed");
					Close(sock,"");
					delete [] buffer;
					return 0;
				}
				// now send connect request
				buffer[0] = 5;
				buffer[1] = 1;
				buffer[2] = 0;
				buffer[3] = 1;

				struct sockaddr_in newadr;
				newadr = GetIp(host,port);

				memcpy(buffer + 4, &newadr.sin_addr,4);
				memcpy(buffer + 8, &newadr.sin_port,2);

				if(!DataWrite(sock,buffer,10 + socksuser.length() + sockspass.length(),NULL))
				{
					debugmsg("-CONNECT5-","error writing to client");
					Close(sock,"");
					delete [] buffer;
					return 0;
				}

				for(int i=0; i < config.read_write_timeout * 2;i++)
				{
					FD_ZERO(&readfds);
					FD_SET(sock,&readfds);
					
					struct timeval tv;
					tv.tv_sec = 0;
					tv.tv_usec = 500000;
					
					if (select(sock+1, &readfds, NULL, NULL, &tv) > 1)
					{					
						break;
					}
					
				}
				if (FD_ISSET(sock, &readfds))
				{
					if(!DataRead(sock,buffer,rc,NULL,0,0))
					{					
						debugmsg("-CONNECT5-","data read failed");
						Close(sock,"");
						delete [] buffer;
						return 0;
					}
					if(rc != 10)
					{
						debugmsg("-CONNECT5-","data read failed");
						Close(sock,"");
						delete [] buffer;
						return 0;
					}
				}
			}
			else
			{	
				debugmsg("-CONNECT5-","data read failed");
				Close(sock,"");
				delete [] buffer;
				return 0;				
			}

		}
		else
		{
			debugmsg("-CONNECT5-","data read failed");
			Close(sock,"");
			delete [] buffer;
			return 0;
		}
		// here socks5 connect ends
		
		debugmsg("CONNECT5","[Connect] end(1)");
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

int IsNumeric(char s)
{
	if(s == '1') return 1;
	if(s == '2') return 1;
	if(s == '3') return 1;
	if(s == '4') return 1;
	if(s == '5') return 1;
	if(s == '6') return 1;
	if(s == '7') return 1;
	if(s == '8') return 1;
	if(s == '9') return 1;
	if(s == '0') return 1;
	return 0;	
}

int FtpCode(string tmp,int &res)
{
	// check if it is last line
	int pos1,pos2;
	pos1 = tmp.rfind('\r',tmp.length());
	
	if (pos1 == (int)string::npos)
	{
		// we never should get here
		return 0;
	}
	
						
	// check if message has only 1 line
	pos2 = tmp.rfind('\r',pos1-1);
	if (pos2 != (int)string::npos)
	{
		//multiple lines
		
		if(tmp[pos2+5] != '-' && IsNumeric(tmp[pos2+4]) && IsNumeric(tmp[pos2+3]) && IsNumeric(tmp[pos2+2]))
		{
			string t;
			t +=	tmp[pos2+2];
			t +=	tmp[pos2+3];
			t +=	tmp[pos2+4];
			res = atoi(t.c_str());
			
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
		if(tmp[3] != '-' && IsNumeric(tmp[2]) && IsNumeric(tmp[1]) && IsNumeric(tmp[0]))
		{
			string t;
			t +=	tmp[0];
			t +=	tmp[1];
			t +=	tmp[2];
			res = atoi(t.c_str());
			
			return 1;
		}
		else
		{
			return 0;
		}
	}
}

int IsEndline(string tmp)
{
	// check if it is last line
	int pos1,pos2;
	pos1 = tmp.rfind('\r',tmp.length());
	
	if (pos1 == (int)string::npos)
	{
		// we never should get here
		return 0;
	}
	
						
	// check if message has only 1 line
	pos2 = tmp.rfind('\r',pos1-1);
	if (pos2 != (int)string::npos)
	{
		//multiple lines
		
		if(tmp[pos2+5] != '-' && IsNumeric(tmp[pos2+4]) && IsNumeric(tmp[pos2+3]) && IsNumeric(tmp[pos2+2]))
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
		if(tmp[3] != '-' && IsNumeric(tmp[2]) && IsNumeric(tmp[1]) && IsNumeric(tmp[0]))
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
	debugmsg("-GETIP-","try to get ip for: " + ip);
	if(!inet_aton(ip.c_str(),&addr.sin_addr))
	{
		struct hostent *he;
	
		if((he = gethostbyname(ip.c_str())) == NULL)
		{
			debugmsg("-GETIP-","error resolving ip: " + ip);			
			inet_aton("0.0.0.0", &addr.sin_addr); 
		}
		else
		{
			addr.sin_addr = *(struct in_addr*)he->h_addr;
			string tmp;
			tmp = inet_ntoa(addr.sin_addr);
			debugmsg("-GETIP-","resolved ip: " + tmp);
		}
	}
	else
	{
		//debugmsg("-GETIP-","error resolving ip: " + ip + " using inet_addr - check your conf");
		///addr.sin_addr.s_addr = inet_addr(ip.c_str());
		//debugmsg("-GETIP-","error resolving ip: " + ip);
		//inet_aton("0.0.0.0", &addr.sin_addr); 
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
		n = 0;
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
						debugmsg("CONTROLWRITE"," fd isset error",errno);
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
//						if (trying) {
							delete [] buffer;
							return 2; 
//						}
/*						debugmsg("CONTROLREAD","want read/write error");
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
*/					}
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

static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
	if(preverify_ok == 2)
	{
		// do nothing
	}
	if(ctx == NULL)
	{
		// do nothing
	}
	return 1;
}

int SslConnect(int &sock,SSL **ssl,SSL_CTX **sslctx,int &shouldquit,string cipher)
{
	if(!setnonblocking(sock))
	{				
		return 0;
	}
	debugmsg("SSLCONNECT", "[SslConnect] start");
	if(*sslctx == NULL)
	{
		debugmsg("SSLCONNECT", "[SslConnect] ctx == NULL - make new");
		*sslctx = SSL_CTX_new(SSLv23_client_method());
		
		if (*sslctx == NULL)
		{
			debugmsg("SSLCONNECT", "[SslConnect] sitesslctx failed!");
			return 0;
		}
		SSL_CTX_set_default_verify_paths(*sslctx);
		SSL_CTX_set_options(*sslctx,SSL_OP_ALL);
		SSL_CTX_clear_mode(*sslctx,SSL_MODE_AUTO_RETRY);
		SSL_CTX_set_session_cache_mode(*sslctx,SSL_SESS_CACHE_OFF);			
	}
	*ssl = SSL_new(*sslctx);
	if (*ssl == NULL)
	{
		debugmsg("SSLCONNECT", "[SslConnect] sslnew failed!");
		return 0;
	}
	if(cipher != "")
	{
		SSL_set_cipher_list(*ssl,cipher.c_str());
	}
	
	if(SSL_set_fd(*ssl,sock) == 0)
	{
		debugmsg("SSLCONNECT", "[SslConnect] ssl set fd failed!");
		debugmsg("SSLCONNECT","[SslConnect] " +  (string)ERR_error_string(ERR_get_error(), NULL));
		return 0;
	}
	SSL_set_verify(*ssl,SSL_VERIFY_PEER,verify_callback);
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
				if (shouldquit == 1) 
				{
					debugmsg("SSLCONNECT","[SslConnect] end(0-2)");		
					return 0;
				}
				debugmsg("SSLCONNECT", "[SslConnect] want read");
				fd_set data_writefds, data_readfds;
				FD_ZERO(&data_readfds);
				FD_SET(sock,&data_readfds);
				FD_ZERO(&data_writefds);
				FD_SET(sock,&data_writefds);
				struct timeval tv;
				tv.tv_sec = 50;
				tv.tv_usec = 0;
				if (select(sock+1, &data_readfds, &data_writefds, NULL, &tv) < 1)
				{
					continue;
				}
				if (FD_ISSET(sock, &data_writefds))
				{
					continue; 
				} else
				if (FD_ISSET(sock, &data_readfds))
				{
					continue; 
				}
				else
				{
					debugmsg("SSLCONNECT"," fd isset error",errno);
					return 0;
				}
			}
			else
			{
			        stringstream ss;
			        ss<<"[SslConnect] TLS Connection failed! SSL Error : "<< sslerr << " Error : " << err << " Description :" << ERR_error_string(sslerr, NULL);
				debugmsg("SSLCONNECT", ss.str());
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

int SslAccept(int &sock,SSL **ssl,SSL_CTX **sslctx,int &shouldquit, string cipher)
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
	if(cipher != "")
	{
		SSL_set_cipher_list(*ssl,cipher.c_str());
	}
	if (SSL_set_fd(*ssl,sock) == 0)
	{
		debugmsg("SSLACCEPT","[SslAccept] " +  (string)ERR_error_string(ERR_get_error(), NULL));
		return 0;
	}
	
	SSL_set_verify(*ssl,SSL_VERIFY_PEER,verify_callback);
	
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
				if (shouldquit == 1) 
				{
					debugmsg("SSLACCEPT","[SslAccept] end(0-2)");		
					return 0;
				}
				debugmsg("SSLCONNECT", "[SslAccept] want read/write");
				fd_set data_writefds, data_readfds;
				FD_ZERO(&data_readfds);
				FD_SET(sock,&data_readfds);
				FD_ZERO(&data_writefds);
				FD_SET(sock,&data_writefds);
				struct timeval tv;
				tv.tv_sec = 50;
				tv.tv_usec = 0;
				if (select(sock+1, &data_readfds, &data_writefds, NULL, &tv) < 1)
				{
					continue;
				}
				if (FD_ISSET(sock, &data_writefds))
				{
					continue; 
				} else
				if (FD_ISSET(sock, &data_readfds))
				{
					continue; 
				}
				else
				{
					debugmsg("SSLACCEPT"," fd isset error",errno);
					return 0;
				}
			}
			else
			{
			        stringstream ss;
			        ss<<"[SslAccept] TLS Connection failed! SSL Error : "<< sslerr << " Error : " << err << " Description : " << ERR_error_string(sslerr, NULL);
				debugmsg("SSLACCEPT", ss.str());
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
	
	int startpos;
		
	startpos = portcmd.find(" ",0);
	if (startpos == (int)string::npos) { return 0; }
	portcmd = portcmd.substr(startpos+1,portcmd.length());
	startpos = portcmd.find(",",0);
	if (startpos == (int)string::npos) { return 0; }
	portcmd.replace(startpos,1,".");
	startpos = portcmd.find(",",0);
	if (startpos == (int)string::npos) { return 0; }
	portcmd.replace(startpos,1,".");
	startpos = portcmd.find(",",0);
	if (startpos == (int)string::npos) { return 0; }
	portcmd.replace(startpos,1,".");
	startpos = portcmd.find(",",0);
	if (startpos == (int)string::npos) { return 0; }
	ip = portcmd.substr(0,startpos);
	
		
	string tmpport;
	tmpport = portcmd.substr(startpos+1,portcmd.length());
	startpos = tmpport.find(",",0);
	if (startpos == (int)string::npos) { return 0; }
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
	int startpos,endpos;
	startpos = passivecmd.find("(",0);
	endpos = passivecmd.find(")",0);
	if (startpos == (int)string::npos || endpos == (int)string::npos)
	{		
		return 0;
	}
	
	// split passive mode string
	string tmp = passivecmd.substr(1+startpos,endpos-startpos-1);

	if (tmp == "") return 0;
	
	endpos = tmp.find(",",0);
	if(endpos == (int)string::npos) return 0;
	string ip1 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	endpos = tmp.find(",",0);
	if(endpos == (int)string::npos) return 0;
	string ip2 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	endpos = tmp.find(",",0);
	if(endpos == (int)string::npos) return 0;
	string ip3 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	endpos = tmp.find(",",0);
	if(endpos == (int)string::npos) return 0;
	string ip4 = tmp.substr(0,endpos);
	tmp = tmp.substr(endpos+1,tmp.length());

	ip = ip1 + "." + ip2 + "." + ip3 + "." + ip4;

	endpos = tmp.find(",",0);
	if(endpos == (int)string::npos) return 0;
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
	int rc;
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
		shutdown(sock,2);
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

int readfile(string filename,unsigned char **data,int s)
{
	ifstream ifile(filename.c_str(),ios::binary | ios::in);
	*data = new unsigned char [s+1];
	ifile.read((char *)*data,s);
	ifile.close();
	data[0][s] = '\0';
	return 1;
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

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_CipherInit_ex(ctx, EVP_bf_cfb(), NULL, NULL, NULL,ipos );
    EVP_CIPHER_CTX_set_key_length(ctx, key.length());
    EVP_CipherInit_ex(ctx, NULL, NULL,(unsigned char*)key.c_str(), ivec,ipos );

	if(!EVP_CipherUpdate(ctx, dataout, &outlen, datain, s))
	{
		return 0;
	}

 	EVP_CIPHER_CTX_free(ctx);
        
        for (int i=0;i < (int)key.length();i++) { key[i] = '0'; }
 	return 1;
}

int encrypt(string key,unsigned char *datain,unsigned char *dataout,int s)
{
	unsigned char ivec[8];
	memset(ivec, 0,8);
	int outlen = s;

	EVP_CIPHER_CTX* ctx =  EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(ctx, EVP_bf_cfb(), NULL, NULL, NULL );
        EVP_CIPHER_CTX_set_key_length(ctx, key.length());
        EVP_EncryptInit_ex(ctx, NULL, NULL, (unsigned char*)key.c_str(), ivec );

	if(!EVP_EncryptUpdate(ctx, dataout, &outlen, datain, s))
	{
		return 0;
	}

 	EVP_CIPHER_CTX_free(ctx);
 	
 	for (int i=0;i < (int)key.length();i++) { key[i] = '0'; }
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
	int pos;
	
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
		if(pos == (int)string::npos)
		{
		    message = "AUTH TLS failed";	
			return 0;
		}	
		if(!SslConnect(sock,ssl,sslctx,shouldquit,config.control_cipher))
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
	debugmsg("LOGIN",reply);
	int code = 0;
	if(!FtpCode(reply,code))
	{
		message = "ftpcode error";
		return 0;
	}
	// standard gl message
	pos = reply.find("331 Password required for",0);
	if(pos == (int)string::npos)
	{
		// standard anonymous ftp message
		pos = reply.find("login ok",0);
		if(pos == (int)string::npos)
		{
			pos = reply.find("specify the password",0);
			if(pos == (int)string::npos)
			{ 	
				if(code != 331 && code != 230)
				{
					debugmsg("LOGIN","login failed");
					message = "login failed";
					return 0;
				}
			}
		}
	}
	if(code != 230)
	{
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
		debugmsg("LOGIN",reply);
		if(!FtpCode(reply,code))
		{
			message = "ftpcode error";
			return 0;
		}
		//standard gl message	
		pos = reply.find("logged in.",0);
		if(pos == (int)string::npos)
		{		
			// standard anonymous ftp message
			pos = reply.find("access granted",0);
			if(pos == (int)string::npos)
			{
				pos = reply.find("Login successful",0);
				if(pos == (int)string::npos)
				{
					if(code != 230)
					{
						debugmsg("LOGIN","login failed");
						message = "login failed";  	
						return 0;
					}
				}
			}
		}
	}
   if(usessl)
   {
	   debugmsg("LOGIN","ssl login ok");
   		message = "SSL login successfull";
   }
   else
   {
	   debugmsg("LOGIN","normal login ok");
   		message = "Login successfull";
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

string hash(string text,string algo)
{
	stringstream res;
	EVP_MD_CTX* mdctx = EVP_MD_CTX_create();
    const EVP_MD *md;    
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len, k;
	md = EVP_get_digestbyname(algo.c_str());
	if(md == NULL)
	{
		// return original text if getting algo fails
		debugmsg("-HASH-","Error getting hash algo");
		return text;
	}
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, text.c_str(), text.length());    
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_destroy(mdctx);
	for(k = 0; k < md_len; k++)
	{
		res << hex << (int)md_value[k];		
	}
	return res.str();
}

int filehash(string filename,string algo,string &result)
{
	int size;
	if(!filesize(filename,size)) return 0;
	unsigned char *data;
	readfile(filename,&data,size);
	stringstream res;
	EVP_MD_CTX* mdctx = EVP_MD_CTX_create();
    const EVP_MD *md;    
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len, k;
	md = EVP_get_digestbyname(algo.c_str());
	if(md == NULL)
	{
		delete [] data;		
		return 0;
	}
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, data, size);    
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_destroy(mdctx);
	for(k = 0; k < md_len; k++)
	{
		res << hex << (int)md_value[k];		
	}
	delete [] data;	
	result = res.str();
	return 1;
}

string fingerprint(SSL *ssl)
{
	X509 *cert;
	cert = SSL_get_peer_certificate(ssl);
	
	if(cert == NULL)
	{
		debugmsg("-SYSTEM-", "[datathread] failed to get cert",errno);
		return "NO-FINGERPRINT";
	}
	else
	{
		unsigned char keyid[EVP_MAX_MD_SIZE];
		unsigned int keyidlen;
		if(!X509_digest(cert,EVP_md5(),keyid, &keyidlen))
		{
			debugmsg("-SYSTEM-", "[datathread] failed to get digest",errno);
			X509_free(cert);
			return "NO-FINGERPRINT";
		}
		else
		{
			X509_free(cert);
			stringstream res;
			for(unsigned int k = 0; k < keyidlen; k++)
			{
				res << hex << setfill('0') << setw(2) << (int)keyid[k];
				if(k != (keyidlen -1)) res << ":";
			}
			return upper(res.str(),0);			
		}
	}
}


int Split(const string& input, const string& delimiter, vector<string>& results, bool includeEmpties)
{
    int iPos = 0;
    int newPos = -1;
    int delim_size = (int)delimiter.size();
    int input_size = (int)input.size();

    if(( input_size == 0 ) || ( delim_size == 0 ))
    {
        return 0;
    }
	
    vector<int> positions;

    newPos = input.find(delimiter, 0);

    if( newPos < 0 )
    { 
		results.push_back(input);        
    }

    int numFound = 0;

    while( newPos >= iPos )
    {
        numFound++;
        positions.push_back(newPos);
        iPos = newPos;
        newPos = input.find (delimiter, iPos + delim_size);
    }

    if( numFound == 0 )
    {
        return 0;
    }

    for( int i = 0; i <= (int)positions.size(); ++i )
    {
        string s = "";
        if( i == 0 ) 
        { 
            s = input.substr(i, positions[i]); 
        }
		else
		{
			int offset = positions[i-1] + delim_size;
			if( offset < input_size )
			{
				if( i == (int)positions.size() )
				{
					s = input.substr(offset);
				}
				else if( i > 0 )
				{
					s = input.substr( positions[i-1] + delim_size, 
						  positions[i] - positions[i-1] - delim_size );
				}
			}
		}
        if( includeEmpties || ( s.size() > 0 ) )
        {
            results.push_back(s);
        }
    }
    return (int)results.size();
}


int MatchIp(const string& ip1, const string& ip2)
{
	// einfachster fall
	if(ip1 == ip2) return 1;
	vector<string> vip1;
	vector<string> vip2;
	if(Split(ip1,".",vip1,false) != 4) return 0; 
	if(Split(ip2,".",vip2,false) != 4) return 0; 
	// alle 4 blöcke vergleichen
	for(int i=0; i < 4;i++)
	{
		string s1,s2;
		s1 = vip1[i];
		s2 = vip2[i];		
		int pos = 0;
		int lpos = 0;
		int rpos = 0;
		
		// den längeren block raussuchen
		int max_size = 0;
		max_size = (int)s1.length();
		if((int)s2.length() > max_size) max_size = (int)s2.length();
	
		while(pos < max_size)
		{
			// stimmen die längen der laufvariablen noch?
			if(s1[lpos] == '*') break; // sonderfall *
			if(lpos >= (int)s1.length()) return 0;
			if(rpos >= (int)s2.length()) return 0;
			
			// bei ? kein vergleich - nur zähler erhöhen
			if(s1[lpos] == '?')
			{
				pos++;
				lpos++;
				rpos++;
			}
			// alles nach * ist ok (192.168.1.1*1 usinnig)
			else if(s1[lpos] == '*')
			{
				break;
			}
			// normalfall - beide stellen vergleichen
			else
			{
				if(s1[lpos] != s2[rpos]) return 0;
				lpos++;
				rpos++;
				pos++;
			}
		}
	}
	return 1;
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
