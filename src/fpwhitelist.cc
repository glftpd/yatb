#include "fpwhitelist.h"


CFpWhitelist::CFpWhitelist()
{
	
}

CFpWhitelist::~CFpWhitelist()
{
	
}

// return 1 for new ip - 0 for already added ip
int CFpWhitelist::CheckIp(string fp, string ip,string &msg)
{
	string tmp = "";
	if(config.use_fxpiphash)
	{
		// use hash of ip?
		tmp = hash(bk+ip,config.hash_algo);
	}
	else
	{
		tmp = ip;
	}
	// get fp entry
	for(unsigned int i=0; i < List.size();i++)
	{
		if(List[i].fp == fp)
		{
			for(unsigned int k=0; k < List[i].iplist.size();k++)
			{
				if(List[i].iplist[k] == tmp)
				{
					// already added - do nothing
					return 0;
				}		
			}
			List[i].iplist.push_back(tmp);
			msg = " new ip: " + List[i].comment + " [" + fp + "] - " + ip;
			WriteList(config.fpwhitelist_file,fpwl_bk);
			return 1;
		}
	}	
	return -1; // fp not found
}

int CFpWhitelist::WriteList(string filename, string key)
{
	string daten;
	for (unsigned int i=0;i < List.size();i++)
	{
		daten += List[i].fp + "," + List[i].comment + "," + List[i].user + ",";
		for(unsigned int k=0; k < List[i].iplist.size();k++)
		{
			if(k == List[i].iplist.size() -1)
			{
				daten += List[i].iplist[k];
			}
			else
			{
				daten += List[i].iplist[k] + ",";
			}
		}
		daten += "\r\n";
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
	// format: fp,comment,user,ip1,ip2..
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
			pos = s.find(",",0);
			if(pos == string::npos)
			{
				user = s;
			}
			else
			{
				user = s.substr(0,pos);
			}
			FpEntry entry;
			entry.fp = upper(fp,0);
			entry.comment = comment;
			entry.user = user;
			pos = s.find(",",0);
			if (pos != string::npos)
			{
				s = s.substr(pos + 1, s.length() - pos - 1);				
				// add ips if possible
				if(s != "")
				{
					string tmp = "";
					for(unsigned int i=0; i < s.length();i++)
					{
						if(s[i] != ',' && s[i] != '\r' && s[i] != '\n') tmp += s[i];
						if(s[i] == ',')
						{
							if(tmp != "")
							{								
								entry.iplist.push_back(tmp);
								tmp = "";
							}
						}
					}
				}
			}
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
	stringstream ss;
	ss << "200- FpWhitelist report start\r\n";
	for (unsigned int i=0; i < List.size(); i++)
	{
		ss << "200- " + List[i].fp + " - " + List[i].comment + " - " + List[i].user +
			" #ips: " << List[i].iplist.size() << "\r\n";
	}
	ss << "200 FpWhitelist report end\r\n";
	return ss.str();
}
