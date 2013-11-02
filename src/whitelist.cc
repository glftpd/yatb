#include "whitelist.h"


CWhitelist::CWhitelist()
{
	
}

CWhitelist::~CWhitelist()
{
	
}

int CWhitelist::WriteList(string filename, string key)
{
	string daten;
	for (unsigned int i=0;i < List.size();i++)
	{
		daten += List[i].ip + "," + List[i].comment + "," + List[i].user + "\r\n";
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

int CWhitelist::ReadList(string filename, string key)
{
	int s;
	if (!filesize(filename,s))
	{
		debugmsg("-READLIST-","Error reading ip list");
		return 0;
	}
	else
	{	
		unsigned char *bufferin,*bufferout;
		
		bufferout = new unsigned char [s+1];
		
		memset(bufferout,'\0',s+1);
		readfile(filename,&bufferin,s);

		string tmp,daten;
		if(config.crypted_iplist)
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
				Insert_nokey(tmp);
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

int CWhitelist::IsInList(string s)
{
	string tmp;
	if(config.use_fxpiphash)
	{		
		tmp = hash(bk+s,config.hash_algo);
	}
	else
	{
		tmp = s;
	}
	for (unsigned int i=0;i < List.size();i++)
	{
		if (List[i].ip == tmp)
		{
			return 1;
		}
	}
	return 0;
}

string CWhitelist::GetComment(string s)
{
	string tmp;
	if(config.use_fxpiphash)
	{		
		tmp = hash(bk+s,config.hash_algo);
	}
	else
	{
		tmp = s;
	}
	for (unsigned int i=0;i < List.size();i++)
	{
		if (List[i].ip == tmp)
		{
			return List[i].comment + " - " + List[i].user;
		}
	}
	return "";
}

void CWhitelist::Remove(string s)
{
	string tmp;
	if(config.use_fxpiphash)
	{	
		unsigned int pos;
		pos = s.find(".",0);
		if(pos == string::npos)
		{
			tmp = s;
		}
		else
		{
			tmp = hash(bk+s,config.hash_algo);
		}
	}
	else
	{
		tmp = s;
	}
	vector<FxpEntry>::iterator it;
	for( it = List.begin(); it != List.end(); it++ )
	{
		
		if (((FxpEntry)*it).ip == tmp)
		{
			List.erase(it);
			return;
		}
	}

}


int CWhitelist::Insert(string s)
{
	// format: ip,comment,user
	string ip,comment,user;
	if (s == "") { return 0; }	
	unsigned int pos = s.find(",",0);
	if (pos != string::npos)
	{
		if(IsInList(s.substr(0,pos))) { return 2; }
		if(config.use_fxpiphash)
		{			
			ip = hash(bk+s.substr(0,pos),config.hash_algo);			
		}
		else
		{
			ip = s.substr(0,pos);
		}
		s = s.substr(pos + 1, s.length() - pos - 1);
		pos = s.find(",",0);
		if (pos != string::npos)
		{
			comment = s.substr(0,pos);
			s = s.substr(pos + 1, s.length() - pos - 1);
			user = s;
			FxpEntry entry;
			entry.ip = ip;
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

void CWhitelist::Insert_nokey(string s)
{
	// format: ip,comment,user
	string ip,comment,user;
	if (s == "") { return; }
	if(IsInList(s)) { return; }
	unsigned int pos = s.find(",",0);
	if (pos != string::npos)
	{		
		ip = s.substr(0,pos);		
		s = s.substr(pos + 1, s.length() - pos - 1);
		pos = s.find(",",0);
		if (pos != string::npos)
		{
			comment = s.substr(0,pos);
			s = s.substr(pos + 1, s.length() - pos - 1);
			user = s;
			FxpEntry entry;
			entry.ip = ip;
			entry.comment = comment;
			entry.user = user;
			List.push_back(entry);
		}
	}

	
}

string CWhitelist::GetList(void)
{
	string tmp = "200- FxpIp report start\r\n";
	for (unsigned int i=0; i < List.size(); i++)
	{
		tmp += "200- " + List[i].ip + " - " + List[i].comment + " - " + List[i].user + "\r\n";
	}
	tmp += "200 FxpIp report end\r\n";
	return tmp;
}
