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
	int working_ip_idx = -1;
	for (unsigned int i=0; i<List.size(); i++)
	{
		if (List[i].working)
		{
			working_ip_idx=i;
			break;
		}
	}
	// No working IP ? Reset and retry
	if (working_ip_idx == -1)
	{
		debugmsg("IPLIST", "resetting status");
		for (unsigned int i=0; i<List.size(); i++)
			List[i].working=1;
		working_ip_idx=0;
	}

	CIp tmp;
	tmp = List[working_ip_idx];
	cc << working_ip_idx << ",";
	ip = tmp.ip;
	port = tmp.port;
	cc << ip << "," << port;
	lock.UnLock();
	debugmsg("IPLIST",cc.str());
	debugmsg("IPLIST","get ip end");
}

void CIplist::setipdown(string ip, int port)
{
	debugmsg("IPLIST","set ip down start");
	lock.Lock();
	for (unsigned int i=0; i<List.size(); i++)
	{
		if (!strcmp(ip.c_str(), List[i].ip.c_str()) && port == List[i].port)
		{
			debugmsg("IPLIST","disabling " + ip);
			List[i].working = 0;
		}
	}

	lock.UnLock();
	debugmsg("IPLIST","set ip down end");
}
