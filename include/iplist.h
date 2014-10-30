#ifndef __IPLIST_H
#define __IPLIST_H

#include "global.h"
#include "lock.h"

class CIp;


class CIplist
{
	public:
	
	int readlist(string,string);
	void getip(string &,int &);
	void setipdown(string, int);
	CIplist();
	~CIplist();
	
	private:
	
	void ParseString(string ,vector<string> &);
	vector<CIp> List;
	vector<string> ip_list;
	vector<string> port_list;
	CLock lock;
};


#endif
