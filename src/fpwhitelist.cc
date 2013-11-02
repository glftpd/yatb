#include "fpwhitelist.h"


CFpWhitelist::CFpWhitelist()
{
	
}

CFpWhitelist::~CFpWhitelist()
{
	
}

int CFpWhitelist::WriteList(string filename, string key)
{
	string daten;
	for (unsigned int i=0;i < List.size();i++)
	{
		daten += List[i].fp + "," + List[i].comment + "," + List[i].user + "\r\n";
	}
	unsigned char *bufferin,*bufferout;
	int s = daten.length();
	bufferin = new unsigned char [s+1];
	bufferout = new unsigned char [s+1];
	
	memset(bufferout,'\0',s+1);
	for(int i=0; i < s;i++)
	{
		bufferin[i] = daten[i];
	}
	if(key == "")
	{
		for(int a=0;a < s+1;a++) bufferout[a] = bufferin[a];
	}
	else
	{
		encrypt(key,bufferin,bufferout,s);
	}
	if(!writefile(filename,bufferout,s))
	{
		memset(bufferin,'\0',s+1);
		memset(bufferout,'\0',s+1);
		delete [] bufferin;
 		delete [] bufferout;
		return 0;
	}
	else
	{
		memset(bufferin,'\0',s+1);
		memset(bufferout,'\0',s+1);
		delete [] bufferin;
 		delete [] bufferout;
		return 1;
	}
	
}

int CFpWhitelist::ReadList(string filename, string key)
{
	int s;
	if (!filesize(filename,s))
	{
		debugmsg("-READLIST-","Error reading fpwhitelist list");
		return 0;
	}
	else
	{	
		unsigned char *bufferin,*bufferout;
		
		bufferout = new unsigned char [s+1];
		
		memset(bufferout,'\0',s+1);
		readfile(filename,&bufferin,s);

		string tmp,daten;
		if(config.crypted_fpwhitelist)
		{
			decrypt(key,bufferin,bufferout,s);
			daten = (char*)bufferout;
		}
		else
		{
			daten = (char*)bufferin;
		}

		for(int i=0; i < s;i++)
		{
			if(daten[i] != '\r' && daten[i] != '\n') tmp += daten[i];
			if(daten[i] == '\n')
			{
				Insert(tmp);
				tmp = "";
			}
		}
		memset(bufferin,'\0',s+1);
		memset(bufferout,'\0',s+1);
		delete [] bufferin;
	 	delete [] bufferout;
		return 1;
	}
}

int CFpWhitelist::IsInList(string s)
{
	string tmp;
	tmp = s;
	for (unsigned int i=0;i < List.size();i++)
	{
		if (List[i].fp == tmp)
		{
			return 1;
		}
	}
	return 0;
}

string CFpWhitelist::GetComment(string s)
{
	string tmp;
	
	tmp = s;
	
	for (unsigned int i=0;i < List.size();i++)
	{
		if (List[i].fp == tmp)
		{
			return List[i].comment + " - " + List[i].user;
		}
	}
	return "";
}

void CFpWhitelist::Remove(string s)
{
	string tmp;
	
	tmp = s;
	
	vector<FpEntry>::iterator it;
	for( it = List.begin(); it != List.end(); it++ )
	{		
		if (((FpEntry)*it).fp == tmp)
		{
			List.erase(it);
			return;
		}
	}
}


int CFpWhitelist::Insert(string s)
{
	// format: fp,comment,user
	string fp,comment,user;
	if (s == "") { return 0; }	
	unsigned int pos = s.find(",",0);
	if (pos != string::npos)
	{
		if(IsInList(s.substr(0,pos))) { return 2; }
		
		fp = s.substr(0,pos);
		
		s = s.substr(pos + 1, s.length() - pos - 1);
		pos = s.find(",",0);
		if (pos != string::npos)
		{
			comment = s.substr(0,pos);
			s = s.substr(pos + 1, s.length() - pos - 1);
			user = s;
			FpEntry entry;
			entry.fp = upper(fp,0);
			entry.comment = comment;
			entry.user = user;
			List.push_back(entry);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	
}


string CFpWhitelist::GetList(void)
{
	string tmp = "200- FpWhitelist report start\r\n";
	for (unsigned int i=0; i < List.size(); i++)
	{
		tmp += "200- " + List[i].fp + " - " + List[i].comment + " - " + List[i].user + "\r\n";
	}
	tmp += "200 FpWhitelist report end\r\n";
	return tmp;
}
