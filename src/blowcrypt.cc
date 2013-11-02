#include <iostream>
#include <unistd.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <openssl/ssl.h>
#include "tools.h"
#include "config.h"
#include "lock.h"
#include "counter.h"

using namespace std;

// some dummy objects to use tools.h
CConfig config;
CLock sock_lock,config_lock;
CCounter monthcounter,weekcounter,daycounter;
int use_blowconf;

int main(int argc,char *argv[])
{
	if (argc < 3 || argc > 4)
	{
		cout << "Blowcrypt 1.2 (c) Hawk/PPX\n";
		cout << "Usage: \n blowcrypt -e file to encrypt 'file' to 'file'\n";
		cout << "\tor blowcrypt -e file1 file2 to encrypt 'file1' to 'file2'\n";
		cout << " blowcrypt -d file to decrypt 'file' to 'file'\n";
		cout << "\tor blowcrypt -d file1 file2 to decrypt 'file1' to 'file2'\n";
		return 0;
	}
	else
	{
		string p1,p2,p3;
		p1 = argv[1];
		p2 = argv[2];
		if (argc == 3)
		{
			p3 = argv[2];
		}
		else
		{
			p3 = argv[3];
		}
		if (p1 == "-d")
		{
			char *key;
			key = getpass("Enter blowkey: ");
			string k1 = key;

			int size;
			if (!filesize(p2,size))
			{
				cout << "error opening file!\n";
				return 0;
			}

			unsigned char *in,*out;
			in = new unsigned char [size];
			out = new unsigned char [size];
			in = readfile(p2,size);
			if(!decrypt(k1,in,out,size))
			{
				cout << "Decrypt error!\n";
				delete in;
				delete out;
				return 0;
			}
			if (!writefile(p3,out,size))
			{
				delete in;
				delete out;
				cout << "Error opening file!\n";
				return 0;
			}
			delete in;
			delete out;
			return 0;


		}
		else if(p1 == "-e")
		{
			char *key;
			key = getpass("Enter blowkey: ");
			string k1 = key;
			key = getpass("Please reenter: ");
			string k2 = key;

			if (k1 != k2)
			{
				cout << "Passes do not match!\n";
				return 0;
			}

			int size;
			if (!filesize(p2,size))
			{
				cout << "error opening file!\n";
				return 0;
			}

			unsigned char *in,*out;
			in = new unsigned char [size];
			out = new unsigned char [size];
			in = readfile(p2,size);
			if (!encrypt(k2,in,out,size))
			{
				cout << "Encrypt error!\n";
				delete in;
				delete out;
				return 0;
			}
			if (!writefile(p3,out,size))
			{
				delete in;
				delete out;
				cout << "Error opening file!\n";
				return 0;
			}
			delete in;
			delete out;
			return 0;
		}
		else
		{
			cout << "Blowcrypt 1.2 (c) Hawk/PPX\n";
			cout << "Usage: \n blowcrypt -e file to encrypt 'file' to 'file'\n";
			cout << "\tor blowcrypt -e file1 file2 to encrypt 'file1' to 'file2'\n";
			cout << " blowcrypt -d file to decrypt 'file' to 'file'\n";
			cout << "\tor blowcrypt -d file1 file2 to decrypt 'file1' to 'file2'\n";
			return 0;
		}
	}



}
