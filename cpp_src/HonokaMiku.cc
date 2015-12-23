/**
* HonokaMiku.cc
* Main program
*
* Copyright © 2036 Dark Energy Processor Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
* associated documentation files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute,
* sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial
* portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
* NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**/

#include <iostream>
#include <string>

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cerrno>
#include <cstring>

#include "DecrypterContext.h"
#include "md5.h"

static char usage_string[]=
	"HonokaMiku. Universal LL!SIF game files decrypter\n"
	"Usage: %s <input file> [output file] [options]\n"
	"<input file> and [output file] can be - for stdin and stdout.\n"
	"\nOptions:\n"
	" -b<name>                  Use basename <name> as decrypt/encrypt\n"
	"                           key.\n"
	"\n"
	" -e<data>                  Encrypt <input file> instead of decrypting\n"
	"                           it. <data> is game-specific. If you use\n"
	"                           this, -w, -j, or -t must be specificed.\n"
	"\n"
	" -h                        Show this message\n"
	" -?\n"
	"\n"
	" -j                        Decrypt: Assume <input file> is SIF JP\n"
	"                                    game file.\n"
	"                           Encrypt: <data> is unknown 2-byte composed\n"
	"                                    this way: <00><11>. It's unknown\n"
	"                                    how does it work, but those bytes\n"
	"                                    can be fetched from the existing\n"
	"                                    encrypted file(at index 10-11).\n"
	"                                    The data needs to be passed as\n"
	"                                    hexadecimal\n"
	"\n"
	" -n<data>                  Decrypt headless file. In that case, -b\n"
	"                           must be specificed and either -w, -j, or\n"
	"                           -t must be specificed too.\n"
	"                           -w or -t: <data> is ignored\n"
	"                           -j: check -j Encrypt information.\n"
	"\n"
	" -t                        Decrypt: Assume <input file> is SIF TW\n"
	"                           game file.\n"
	"                           Encrypt: <data> is unused.\n"
	"\n"
	" -w                        Decrypt: Assume <input file> is SIF EN\n"
	"                           game file.\n"
	"                           Encrypt: <data> is unused.\n";

/* globals */
char g_DecryptGame=0;	// 0 = not specificed; 1 = ww/en; 2 = jp; 3 = tw
bool g_Encrypt=false;
char* g_MoreData=nullptr;
Dctx* g_Dctx=nullptr;
const char* g_Basename=nullptr;
bool g_Headless=false;
int g_InPos=0;
int g_OutPos=0;

inline void usage_noexit(const char* p)
{
	fprintf(stderr,usage_string,p);
}

void usage(const char* p)
{
	usage_noexit(p);
	exit(1);
}

void failexit(const char* p,const char* msg)
{
	std::cerr << msg << std::endl;
	usage_noexit(p);
	exit(-1);
}

void failexit(int errcode,const char* file)
{
	std::cerr << "Cannot open " << file << ": " << strerror(errcode) << std::endl;
	exit(-1);
}

void failexit(const char* file,const char* what,const char* msg)
{
	std::cerr << "Cannot " << what << " " << file << ": " << msg;
	exit(-1);
}

void parse_args(int argc,char* argv[])
{
	using namespace std;

	for(int i=1;i<argc;i++)
	{
		if((argv[i][0]=='-' || argv[i][0]=='/') && argv[i][1]!='\0')
		{
			char s=argv[i][1];
			if(s=='b')
				g_Basename=argv[i]+2;

			else if(s=='e')
			{
				char* data=argv[i]+2;
				size_t len=strlen(data);
				g_Encrypt=true;

				if(len==4) g_MoreData=data;
				else if(len>0) cerr << "Ignoring " << argv[i]+2 << " in " << argv[i] << ": additional data cannot be parsed" << endl;
			}

			else if(s=='h' || s=='?')
			{
				usage(argv[0]);
				exit(0);
			}

			else if(s=='j')
				g_DecryptGame=2;

			else if(s=='n')
			{
				char* data=argv[i]+2;
				size_t len=strlen(data);
				g_Headless=true;

				if(len==4) g_MoreData=data;
				else if(len>0) cerr << "Ignoring " << argv[i]+2 << " in " << argv[i] << ": additional data cannot be parsed" << endl;
			}

			else if(s=='t')
				g_DecryptGame=3;

			else if(s=='w')
				g_DecryptGame=1;

			else
				cerr << "Ignoring " << argv[i] << ": unknown option" << std::endl;
		}
		else if(g_InPos==0)
			g_InPos=i;
		else if(g_OutPos==0)
			g_OutPos=i;
		else
			cerr << "Ignoring " << argv[i] << endl;
	}
}

void check_args(char* argv[])
{
	// pre-check
	if((g_Basename!=NULL && strlen(g_Basename)==0) || g_Basename==NULL)
		g_Basename=argv[g_InPos];

	if(g_MoreData!=NULL && strlen(g_MoreData)==0)
		g_MoreData=NULL;	// discard

	// check args
	if(g_InPos==0) usage(argv[0]);

	if(g_Headless && g_Encrypt)
		failexit(argv[0],"only -n or -e are allowed");

	if(g_Encrypt || g_Headless)
	{
		if(g_DecryptGame==0)
			failexit(argv[0],"-e or -n requires -w, -j, or -t");
		else if(g_DecryptGame==2)
		{
			if(g_MoreData==NULL)
				failexit(argv[0],"-e or -n with -j requires <data>");
			else
			{
				char* string_position;
				short val=strtol(g_MoreData,&string_position,16);
				if(g_MoreData==string_position || *string_position!=0)
					failexit(argv[0],"<data> passed in -e or -n is not a hexadecimal number");

				val=val>>8|val<<8;
				g_MoreData=new char[2];
				memcpy(g_MoreData,&val,2);
			}
		}
	}
}

int main(int argc,char* argv[])
{
	if(argc<2) usage(argv[0]);
	
	FILE* in;
	FILE* out;
	char* file_out;

	parse_args(argc,argv);
	check_args(argv);
	
	if(strcmp(argv[g_InPos],"-"))
		in=fopen(argv[g_InPos],"rb");
	else
		in=stdin;
	if(in==NULL)
		failexit(errno,argv[1]);

	if(g_OutPos)
		file_out=argv[g_OutPos];
	else
	{
		size_t argv1_len=strlen(argv[g_InPos]);
		file_out=new char[argv1_len+2];
		memcpy(file_out,argv[g_InPos],argv1_len);
		file_out[argv1_len]='_';
		file_out[argv1_len+1]=0;
	}

	char* buffer;
	size_t file_size;
	size_t out_size;
	size_t header_size;
	fseek(in,0,SEEK_END);
	file_size=ftell(in);
	fseek(in,0,SEEK_SET);
	buffer=new char[file_size+16];

	if(g_DecryptGame==0 && !(g_Encrypt || g_Headless))
	{
		char header[16];
		std::cerr << "Auto detecting: ";
		try
		{
			fread(header,1,4,in);
			g_Dctx=new EN_Dctx(header,g_Basename);
			std::cerr << "EN game file" << std::endl;
			g_DecryptGame=1;
		}
		catch(std::exception)
		{
			fseek(in,0,SEEK_SET);
			try
			{
				fread(header,1,16,in);
				g_Dctx=new JP_Dctx(header,g_Basename);
				std::cerr << "JP game file" << std::endl;
				g_DecryptGame=2;
			}
			catch(std::exception)
			{
				fseek(in,0,SEEK_SET);
				try
				{
					fread(header,1,4,in);
					g_Dctx=new TW_Dctx(header,g_Basename);
					std::cerr << "TW game file" << std::endl;
					g_DecryptGame=3;
				}
				catch(std::exception)
				{
					std::cerr << "cannot detect" << std::endl;
					failexit(argv[g_InPos],"decrypt","Cannot find suitable decryption method");
				}
			}
		}
	}
	else if(g_DecryptGame>0 && (g_Encrypt || g_Headless))
	{
		char header[16];
		MD5_CTX* mctx;
		if(g_Basename==NULL) g_Basename=__DctxGetBasename(argv[g_InPos]);

		mctx=new MD5_CTX;
		MD5Init(mctx);

		switch(g_DecryptGame) {
			case 1:
			{
				MD5Update(mctx,(unsigned char*)"BFd3EnkcKa",10);
				break;
			}
			case 2:
			{
				MD5Update(mctx,(unsigned char*)"Hello",5);
				break;
			}
			case 3:
			{
				MD5Update(mctx,(unsigned char*)"M2o2B7i3M6o6N88",15);
				break;
			}
		}
		
		MD5Update(mctx,(unsigned char*)g_Basename,strlen(g_Basename));
		MD5Final(mctx);

		switch(g_DecryptGame) {
			case 1:
			{
				memcpy(header,mctx->digest+4,4);
				g_Dctx=new EN_Dctx(header,g_Basename);
				if(g_Encrypt) memcpy(buffer,header,4);
				break;
			}
			case 2:
			{
				unsigned int* digcopy=(unsigned int*)header;
				memset(header,0,16);
				memcpy(header,mctx->digest+4,3);
				memcpy(header+10,g_MoreData,2);
				*digcopy=~*digcopy;
				header[3]=12;
				g_Dctx=new JP_Dctx(header,g_Basename);
				if(g_Encrypt) memcpy(buffer,header,16);
				break;
			}
			case 3:
			{
				memcpy(header,mctx->digest+4,4);
				g_Dctx=new TW_Dctx(header,g_Basename);
				if(g_Encrypt) memcpy(buffer,header,4);
				break;
			}
		}

		delete mctx;
	}
	else if(g_DecryptGame>0)
	{
		char header[16];
		try
		{
			switch(g_DecryptGame) {
				case 1:
				{
					fread(header,1,4,in);
					g_Dctx=new EN_Dctx(header,g_Basename);
					break;
				}
				case 2:
				{
					fread(header,1,16,in);
					g_Dctx=new JP_Dctx(header,g_Basename);
					break;
				}
				case 3:
				{
					fread(header,1,4,in);
					g_Dctx=new TW_Dctx(header,g_Basename);
					break;
				}
			}
		}
		catch(std::exception)
		{
			failexit(argv[g_InPos],"decrypt","The specificed method cannot be used to decrypt this file");
		}
	}

	header_size=g_DecryptGame==2?16:4;
	out_size=file_size;

	if(g_Encrypt)
	{
		buffer+=header_size;
		out_size+=header_size;
	}
	else
		out_size-=header_size;

	fread(buffer,1,file_size,in);
	g_Dctx->decrypt_block(buffer,file_size);
	fclose(in);

	out=strcmp(file_out,"-")==0?stdout:fopen(file_out,"wb");
	if(out==NULL)
	{
		std::cerr << "Cannot open " << file_out << ": " << strerror(errno) << std::endl << "Writing to stdout instead" << std::endl;
		out=stdout;
	}

	if(g_Encrypt)
		buffer-=header_size;
	fwrite(buffer,1,out_size,out);
	fclose(out);
	
	delete g_Dctx;
	delete[] g_MoreData;

	return 0;
}
