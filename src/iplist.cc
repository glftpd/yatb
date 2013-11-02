#include "iplist.h"
#include "tools.h"

class CIp
{
	public:
	string ip;
	int port;
	int working;
	
	CIp()
	{
		ip = "";
		port = 0;
		working = 1;
	}
};

void CIplist::ParseString(string s,vector<string> &list)
{
	if (s == "") { return; }
	s = s + ",";
	int pos;
	do
	{
		pos = s.find(",",0);
		if (pos != (int)string::npos)
		{
			string tmp;
			tmp = s.substr(0,pos);
			
			list.push_back(tmp);
			
			s = s.substr(pos + 1, s.length() - pos - 1);
		}
	}
	while (pos != (int)string::npos);
}

CIplist::CIplist()
{
}

CIplist::~CIplist()
{
}

int CIplist::readlist(string iplist,string portlist)
{
	debugmsg("IPLIST","readlist start");
	lock.Lock();
	List.clear();
	ip_list.clear();
	port_list.clear();
	ParseString(iplist,ip_list);
	if(portlist != "0")
	{
		ParseString(portlist,port_list);
		if(ip_list.size() != port_list.size())
		{
			lock.UnLock();
			debugmsg("IPLIST","list size doesn't match");
			 return 0;
		}
	}
	
	if(ip_list.size() == 0)
	{
		lock.UnLock();
		debugmsg("IPLIST","empty list!");
		return 0;
	}
	counter = 0;
	debugmsg("IPLIST","adding ips to list");
	for(int i=0; i < (int)ip_list.size();i++)
	{
		CIp ip;
		ip.ip = ip_list[i];
		if(portlist != "0")
		{
			ip.port = atoi(port_list[i].c_str());
		}
		else
		{
			ip.port = 0;
		}

		List.push_back(ip);
	}
	lock.UnLock();
	stringstream ss;
	ss << "found " << List.size() << " ip(s)";
	debugmsg("IPLIST",ss.str());
	debugmsg("IPLIST","readlist end");
	return 1;
}

void CIplist::getip(string &ip,int &port)
{
	debugmsg("IPLIST","get ip start");
	stringstream cc;
	lock.Lock();
	CIp tmp;
	tmp = List[counter];
	cc << counter << ",";
	counter++;
	if (counter == List.size()) counter = 0;
	ip = tmp.ip;
	port = tmp.port;
	cc << ip << "," << port;
	lock.UnLock();
	debugmsg("IPLIST",cc.str());
	debugmsg("IPLIST","get ip end");
}


