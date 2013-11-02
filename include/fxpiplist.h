#ifndef __FXPIPLIST_H
#define __FXPIPLIST_H

#include "global.h"
#include "config.h"
#include "tools.h"

class FxpEntry
{
	public:
	string ip;
	string comment;
	string user;
};

class CFxpiplist
{
	public:
	
	CFxpiplist();
	
	~CFxpiplist();
	
	void Remove(string);
	
	void Insert(string);
	
	int IsInList(string);
	
	string GetList(void);
	
	private:
	
	vector<FxpEntry> List;
	
			
};

extern CFxpiplist fxpiplist;

#endif

