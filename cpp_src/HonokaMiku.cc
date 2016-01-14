/**
* HonokaMiku.cc
* Main program
*
* Copyright © 2037 Dark Energy Processor Corporation
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

#include <stdint.h>

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include "DecrypterContext.h"
#include "md5.h"
#include "CompilerName.h"
#include "../VersionInfo.rc"

static char usage_string[]=
	"Usage: %s <input file> [output file=input] [options]\n"
	"<input file> and [output file] can be - for stdin and stdout.\n"
	"\nOptions:\n"
	" -b<name>                  Use basename <name> as decrypt/encrypt\n"
	"                           key.\n"
	"\n"
	" -c                        Assume <input file> is SIF CN game file.\n"
	"\n"
	" -e                        Encrypt <input file> instead of decrypting\n"
	"                           it. If you use this, one of the game file\n"
	"                           flag must be specificed.\n"
	"\n"
	" -h                        Show this message\n"
	" -?\n"
	"\n"
	" -j                        Assume <input file> is SIF JP game file.\n"
	"\n"
	" -k                        Assume <input file> is SIF KR game file.\n"
	"\n"
	" -t                        Assume <input file> is SIF TW game file.\n"
	"\n"
	" -v                        Show version information.\n"
	"\n"
	" -w                        Assume <input file> is SIF EN game file.\n";

/* globals */
char g_DecryptGame=0;	// 0 = not specificed; 1 = ww/en; 2 = jp; 3 = tw; 4 = kr; 5 = cn
bool g_Encrypt=false;
Dctx* g_Dctx=NULL;
const char* g_Basename=NULL;
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

			else if(s=='c')
				g_DecryptGame=5;

			else if(s=='e')
				g_Encrypt=true;

			else if(s=='h' || s=='?')
			{
				usage(argv[0]);
				exit(0);
			}

			else if(s=='j')
				g_DecryptGame=2;

			else if(s=='k')
				g_DecryptGame=4;
			
			else if(s=='t')
				g_DecryptGame=3;

			else if(s=='v')
			{
				fputs("Version " HONOKAMIKU_VERSION_STRING "\n",stdout);
				fputs("Build at " __DATE__ " " __TIME__ "\n",stderr);
				fprintf(stderr,"Compiled using %s\n\n",CompilerName());

				exit(0);
			}

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

	// check args
	if(g_InPos==0) usage(argv[0]);

	if(g_Encrypt && g_DecryptGame==0)
		failexit(argv[0],"-e requires -w, -j, or -t");
}

int main(int argc,char* argv[])
{
	fputs("HonokaMiku. Universal LL!SIF game files decrypter\n",stderr);
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
		file_out[argv1_len]=0;
	}

	char* buffer;
	size_t file_size;
	size_t out_size;
	size_t header_size;
	fseek(in,0,SEEK_END);
	file_size=ftell(in);
	fseek(in,0,SEEK_SET);
	buffer=new char[file_size+16];

	if(g_DecryptGame==0 && !g_Encrypt)
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
				fseek(in,4,SEEK_SET);
				try
				{
					g_Dctx=new TW_Dctx(header,g_Basename);
					std::cerr << "TW game file" << std::endl;
					g_DecryptGame=3;
				}
				catch(std::exception)
				{
					try
					{
						g_Dctx=new KR_Dctx(header,g_Basename);
						std::cerr << "KR game file" << std::endl;
						g_DecryptGame=4;
					}
					catch(std::exception)
					{
						try
						{
							g_Dctx=new CN_Dctx(header,g_Basename);
							std::cerr << "CN game file" << std::endl;
							g_DecryptGame=5;
						}
						catch(std::exception)
						{
							std::cerr << "cannot detect" << std::endl;
							failexit(argv[g_InPos],"decrypt","Cannot find suitable decryption method");
						}
					}
				}
			}
		}
	}
	else if(g_DecryptGame>0 && g_Encrypt)
	{
		char header[16];
		if(g_Basename==NULL) g_Basename=__DctxGetBasename(argv[g_InPos]);

		std::cerr << "Encrypt as: ";
		switch(g_DecryptGame) {
			case 1:
			{
				std::cerr << "EN game file" << std::endl;
				g_Dctx=EN_Dctx::encrypt_setup(g_Basename,header);
				memcpy(buffer,header,4);
				break;
			}
			case 2:
			{
				std::cerr << "JP game file" << std::endl;
				g_Dctx=JP_Dctx::encrypt_setup(g_Basename,header);
				memcpy(buffer,header,16);
				break;
			}
			case 3:
			{
				std::cerr << "TW game file" << std::endl;
				g_Dctx=TW_Dctx::encrypt_setup(g_Basename,header);
				memcpy(buffer,header,4);
				break;
			}
			case 4:
			{
				std::cerr << "KR game file" << std::endl;
				g_Dctx=KR_Dctx::encrypt_setup(g_Basename,header);
				memcpy(buffer,header,4);
				break;
			}
			case 5:
			{
				std::cerr << "CN game file" << std::endl;
				g_Dctx=CN_Dctx::encrypt_setup(g_Basename,header);
				memcpy(buffer,header,4);
				break;
			}
		}
	}
	else if(g_DecryptGame>0)
	{
		char header[16];
		std::cerr << "Decrypt as: ";
		try
		{
			switch(g_DecryptGame) {
				case 1:
				{
					std::cerr << "EN game file" << std::endl;
					fread(header,1,4,in);
					g_Dctx=new EN_Dctx(header,g_Basename);
					break;
				}
				case 2:
				{
					std::cerr << "JP game file" << std::endl;
					fread(header,1,16,in);
					g_Dctx=new JP_Dctx(header,g_Basename);
					break;
				}
				case 3:
				{
					std::cerr << "TW game file" << std::endl;
					fread(header,1,4,in);
					g_Dctx=new TW_Dctx(header,g_Basename);
					break;
				}
				case 4:
				{
					std::cerr << "KR game file" << std::endl;
					fread(header,1,4,in);
					g_Dctx=new KR_Dctx(header,g_Basename);
					break;
				}
				case 5:
				{
					std::cerr << "CN game file" << std::endl;
					fread(header,1,4,in);
					g_Dctx=new CN_Dctx(header,g_Basename);
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

	return 0;
}
