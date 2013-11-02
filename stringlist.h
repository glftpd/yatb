#ifndef __STRINGLIST_H
#define __STRINGLIST_H

#include "global.h"


class CStringlist
{
	public:
	
	CStringlist();
	
	~CStringlist();
	
	void Remove(string);
	
	void Insert(string);
	
	int IsInList(string);
	
	string GetList(void);
	
	private:
	
	vector<string> List;
	
	void ParseString(string,int);
	
	void Ins(string);
	
	void Del(string);
	
};

extern CStringlist adminlist,fxpfromsitelist,fxptositelist,sslexcludelist,entrylist;

#endif

