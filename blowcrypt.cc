#include <iostream>
#include <unistd.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <openssl/ssl.h>


// g++ blowcrypt.cc -o blowcrypt -lssl -lcrypto

using namespace std;

int filesize(string filename,int &s)
{
	ifstream ifile(filename.c_str(),ios::binary | ios::in);
	if (!ifile)
	{
		return 0;
	}
	int start,end;
	start = ifile.tellg();
	ifile.seekg(0,ios::end);
	end = ifile.tellg();
	ifile.seekg(0,ios::beg);
	s = end-start;
	return 1;
}

unsigned char *readfile(string filename,int s)
{
	ifstream ifile(filename.c_str(),ios::binary | ios::in);
	unsigned char *tmp;
	tmp = new unsigned char [s];
	ifile.read((char*)tmp,s);
	ifile.close();
	return tmp;
}

int writefile(string filename,unsigned char *data,int s)
{
	ofstream ofile(filename.c_str(),ios::binary | ios::out | ios::trunc);
	if (!ofile)
	{
		return 0;
	}
	ofile.write((char*)data,s);
	ofile.close();
	return 1;
}

int decrypt(string key,unsigned char *datain,unsigned char *dataout,int s)
{
	unsigned char ivec[8];
	memset(ivec,0, sizeof(ivec));
	int ipos = 0;
	int outlen = s;

	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);
        EVP_CipherInit_ex(&ctx, EVP_bf_cfb(), NULL, NULL, NULL,ipos );
        EVP_CIPHER_CTX_set_key_length(&ctx, key.length());
        EVP_CipherInit_ex(&ctx, NULL, NULL,(unsigned char*)key.c_str(), ivec,ipos );

	if(!EVP_CipherUpdate(&ctx, dataout, &outlen, datain, s))
	{
		return 0;
	}

 	EVP_CIPHER_CTX_cleanup(&ctx);
 	for (unsigned int i=0;i<key.length();i++) { key[i] = '0'; }
        return 1;
}

int encrypt(string key,unsigned char *datain,unsigned char *dataout,int s)
{
	unsigned char ivec[8];
	memset(ivec, 0,sizeof(ivec));
	int outlen = s;

	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);
        EVP_EncryptInit_ex(&ctx, EVP_bf_cfb(), NULL, NULL, NULL );
        EVP_CIPHER_CTX_set_key_length(&ctx, key.length());
        EVP_EncryptInit_ex(&ctx, NULL, NULL, (unsigned char*)key.c_str(), ivec );

	if(!EVP_EncryptUpdate(&ctx, dataout, &outlen, datain, s))
	{
		return 0;
	}

 	EVP_CIPHER_CTX_cleanup(&ctx);
 	for (unsigned int i=0;i<key.length();i++) { key[i] = '0'; }
        return 1;
}

int main(int argc,char *argv[])
{
	if (argc < 3 || argc > 4)
	{
		cout << "Blowcrypt 1.1 (c) Hawk/PPX\n";
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
			cout << "Blowcrypt 1.1 (c) Hawk/PPX\n";
			cout << "Usage: \n blowcrypt -e file to encrypt 'file' to 'file'\n";
			cout << "\tor blowcrypt -e file1 file2 to encrypt 'file1' to 'file2'\n";
			cout << " blowcrypt -d file to decrypt 'file' to 'file'\n";
			cout << "\tor blowcrypt -d file1 file2 to decrypt 'file1' to 'file2'\n";
			return 0;
		}
	}



}
