#include "global.h"
#include "config.h"
#include "lock.h"

// print debug msg
void debugmsg(string un,string s,int err)
{
	
	if (config.debug)
	{
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
	if((flags = fcntl(socket, F_GETFL, 0)) < 0)
	{ 
		return 0;
	}
	flags |= O_NONBLOCK;
	if (fcntl(socket, F_SETFL, flags) < 0)
	{
		return 0;
	}
	return 1;
}

int setblocking(int socket)
{
	int flags;
	if((flags = fcntl(socket, F_GETFL, 0)) < 0)
	{ 
		return 0;
	}
	flags &= ~O_NONBLOCK;
	if (fcntl(socket, F_SETFL, flags) < 0)
	{
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

int Connect(int &sock,struct sockaddr_in &adr,int sec,int &shouldquit)
{
	if(connect(sock, (struct sockaddr *)&adr, sizeof(adr)) == -1)
	{
		if(errno != EINPROGRESS)
		{
			return 0;
		}
	}
	fd_set writefds;
	for(int i=0; i < sec * 2;i++)
	{
		FD_ZERO(&writefds);
		FD_SET(sock, &writefds);
		
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		int res = select(sock+1, NULL, &writefds, NULL, &tv);
		if (res < 0)
		{			
			return 0;
		}
		else if(res == 0)
		{
			if(shouldquit) return 0;
		}
		else
		{
			break;
		}
	}
	int err;
	socklen_t errlen = sizeof(err);
	if(getsockopt(sock,SOL_SOCKET,SO_ERROR,&err,&errlen) == -1)
	{
		return 0;
	}
	if(err != 0)
	{		
		return 0;
	}
	return 1;
}

int Accept(int &listensock,int &newsock,struct sockaddr_in &adr,int sec,int &shouldquit)
{
	fd_set readfds;
	
	
	for(int i=0; i < sec * 2;i++)
	{
		FD_ZERO(&readfds);
		FD_SET(listensock, &readfds);
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		
		int res =  select(listensock+1, &readfds, NULL, NULL, &tv);
		if (res < 0)
		{		
			return 0;
		}
		else if(res == 0)
		{
			if (shouldquit) return 0;
		}
		else
		{
			break;
		}
	}
	socklen_t size = sizeof(adr);
	if ((newsock = accept(listensock,(struct sockaddr *)&adr,&size)) == -1)
	{
		return 0;
	}
	return 1;
}

#if defined(__GNUC__) && __GNUC__ < 3
#define ios_base ios
#endif

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
		
		if(tmp[pos2+5] != '-')
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
