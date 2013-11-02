#include "stringlist.h"


CStringlist::CStringlist()
{
	
}

CStringlist::~CStringlist()
{
	
}

int CStringlist::IsInList(string s)
{
	for (int i=0;i < (int)List.size();i++)
	{
		if (List[i] == s)
		{
			return 1;
		}
	}
	return 0;
}

void CStringlist::Ins(string s)
{
	if (!IsInList(s))
	{
		List.push_back(s);
	}
}

void CStringlist::Del(string s)
{
	vector<string>::iterator it;
	for( it = List.begin(); it != List.end(); it++ )
	{
		if (*it == s)
		{
			List.erase(it);
			return;
		}
	}

}

void CStringlist::Insert(string s)
{
	ParseString(s,1);
}

void CStringlist::Remove(string s)
{
	ParseString(s,0);
}

void CStringlist::ParseString(string s,int mode)
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
			if (mode == 0)
			{
				Del(tmp);
			}
			else if (mode == 1)
			{
				Ins(tmp);
			}
			s = s.substr(pos + 1, s.length() - pos - 1);
		}
	}
	while (pos != (int)string::npos);
}

string CStringlist::GetList(void)
{
	string tmp = "";
	for (int i=0; i < (int)List.size(); i++)
	{
		tmp = tmp + List[i];
		if ( i + 1 < (int)List.size())
		{
			tmp = tmp + ",";
		}
	}
	return tmp;
}
