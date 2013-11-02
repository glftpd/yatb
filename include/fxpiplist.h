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
	
	int ReadList(string,string);
	
	int WriteList(string,string);

	string GetList(void);
	
	private:
	
	void Insert_nokey(string);

	vector<FxpEntry> List;
	
			
};

extern CFxpiplist fxpiplist;
extern string bk; // blowkey for hashing
#endif

