#include "fxpiplist.h"


CFxpiplist::CFxpiplist()
{
	
}

CFxpiplist::~CFxpiplist()
{
	
}

int CFxpiplist::IsInList(string s)
{
	for (unsigned int i=0;i < List.size();i++)
	{
		if (List[i].ip == s)
		{
			return 1;
		}
	}
	return 0;
}



void CFxpiplist::Remove(string s)
{
	string tmp;
	if(config.use_fxpiphash)
	{		
		tmp = hash(s,config.hash_algo);
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


void CFxpiplist::Insert(string s)
{
	// format: ip,comment,user
	string ip,comment,user;
	if (s == "") { return; }
	unsigned int pos = s.find(",",0);
	if (pos != string::npos)
	{
		if(config.use_fxpiphash)
		{			
			ip = hash(s.substr(0,pos),config.hash_algo);			
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
		}
	}

	
}

string CFxpiplist::GetList(void)
{
	string tmp = "200- FxpIp report start\r\n";
	for (unsigned int i=0; i < List.size(); i++)
	{
		tmp += "200- " + List[i].ip + " - " + List[i].comment + " - " + List[i].user + "\r\n";
	}
	tmp += "200 FxpIp report end\r\n";
	return tmp;
}
