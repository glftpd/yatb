#ifndef __WHITELIST_H
#define __WHITELIST_H

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

class CWhitelist
{
	public:
	
	CWhitelist();
	
	~CWhitelist();
	
	void Remove(string);
	
	int Insert(string);
	
	int IsInList(string);
	
	int ReadList(string,string);
	
	int WriteList(string,string);
	
	string GetComment(string);

	string GetList(void);
	
	private:
	
	void Insert_nokey(string);

	vector<FxpEntry> List;
	
			
};

extern CWhitelist whitelist;
extern string bk; // blowkey for hashing
#endif

