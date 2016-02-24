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

#ifdef __GNUC__
#define DEPCASEISTRCMP strncasecmp
#elif defined(_MSC_VER)
#define DEPCASEISTRCMP strnicmp
#endif

static char usage_string[]=
	"Usage: %s <input file> [output file=input] [options]\n"
	"<input file> and [output file] can be - for stdin and stdout.\n"
	"\nOptions:\n"
	" -b<name>                  Use basename <name> as decrypt/encrypt\n"
	" --basename<name>          key.\n"
	"\n"
	" -c                        Assume <input file> is SIF CN game file.\n"
	" --sif-cn\n"
	"\n"
	" -d                        Detect game file type only. [output file]\n"
	" --detect                  and the other parameters is omitted.\n"
	"\n"
	" -e                        Encrypt <input file> instead of decrypting\n"
	" --encrypt                 it. If you use this, one of the game file\n"
	"                           flag must be specificed.\n"
	"\n"
	" -h                        Show this message\n"
	" -?\n"
	" --help\n"
	"\n"
	" -j                        Assume <input file> is SIF JP game file.\n"
	" --sif-jp\n"
	"\n"
	" -k                        Assume <input file> is SIF KR game file.\n"
	" --sif-kr\n"
	"\n"
	" -t                        Assume <input file> is SIF TW game file.\n"
	" --sif-tw\n"
	"\n"
	" -v                        Show version information.\n"
	" --version\n"
	"\n"
	" -w                        Assume <input file> is SIF EN game file.\n"
	" --sif-en\n"
	" --sif-ww\n"
	"\n"
	" -x<game>                  Cross-encrypt to another game file.\n"
	" --cross-encrypt<game>     <game> can be w, j, t, k, or c\n"
	"\n";

/* globals */
char g_DecryptGame=0;		// 0 = not specificed; 1 = ww/en; 2 = jp; 3 = tw; 4 = kr; 5 = cn
bool g_Encrypt=false;		// Encrypt mode?
Dctx* g_Dctx=NULL;			// Pointer to the DecrypterContext
const char* g_Basename=NULL;// Basename
int g_InPos=0;				// Input filename argv position
int g_OutPos=0;				// Output filename argv position
char g_XEncryptGame=0;		// 0 = cross-encryption not specificed; 1 = ww/en; 2 = jp; 3 = tw; 4 = kr; 5 = cn
bool g_TestMode=false;		// Detect only.

char get_gametype(const char* a)
{
	if(DEPCASEISTRCMP(a,"w",2)==0 || DEPCASEISTRCMP(a,"sif-ww",7)==0 || DEPCASEISTRCMP(a,"sif-en",10)==0)
		return 1;
	else if(DEPCASEISTRCMP(a,"j",2)==0 || DEPCASEISTRCMP(a,"sif-jp",7)==0)
		return 2;
	else if(DEPCASEISTRCMP(a,"t",2)==0 || DEPCASEISTRCMP(a,"sif-tw",7)==0)
		return 3;
	else if(DEPCASEISTRCMP(a,"k",2)==0 || DEPCASEISTRCMP(a,"sif-kr",7)==0)
		return 4;
	else if(DEPCASEISTRCMP(a,"c",2)==0 || DEPCASEISTRCMP(a,"sif-cn",7)==0)
		return 5;
	return 0;
}

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
	std::cerr << "Cannot " << what << " " << file << ": " << msg << std::endl;
	exit(-1);
}

void parse_args(int argc,char* argv[])
{
	using namespace std;

	int i=1;
	for(;i<argc;i++)
	{
		bool is_slash=argv[i][0]=='/';
		if((argv[i][0]=='-' || argv[i][0]=='/') && argv[i][1]!='\0')
		{
			char s=tolower(argv[i][1]);
			if((s=='-' && !is_slash) || (s=='/' && is_slash))
			{
				// Long-name argument parse
				char* start_arg=argv[i]+2;
				if(DEPCASEISTRCMP(start_arg,"basename",8)==0)
				{
					if(start_arg[8]==0)
					{
						i++;
						if(argv[i]==NULL)
							cerr << "Ignoring --basename" << endl;
						else
							g_Basename=argv[i];
					}
					else
						g_Basename=start_arg+8;
				}

				else if(DEPCASEISTRCMP(start_arg,"cross-encrypt",13)==0)
				{
					if(start_arg[13]==0)
					{
						i++;
						if(argv[i]==NULL)
							cerr << "Ignoring --cross-encrypt" << endl;
						else
							g_XEncryptGame=get_gametype(argv[i]);
					}
					else
						g_XEncryptGame=get_gametype(start_arg+13);
					if(g_XEncryptGame==0) cerr << "Ignoring --cross-encrypt: Unknown game type" << endl;
				}

				else if(DEPCASEISTRCMP(start_arg,"encrypt",8)==0 && start_arg[7]==0)
					g_Encrypt=true;

				else if(DEPCASEISTRCMP(start_arg,"detect",7)==0 && start_arg[6]==0)
					g_TestMode=true;

				else if(DEPCASEISTRCMP(start_arg,"help",5)==0 && start_arg[4]==0)
				{
					usage_noexit(argv[0]);
					exit(0);
				}

				else if(DEPCASEISTRCMP(start_arg,"sif-cn",7)==0 && start_arg[6]==0)
					g_DecryptGame=5;

				else if((DEPCASEISTRCMP(start_arg,"sif-en",7)==0 || DEPCASEISTRCMP(start_arg,"sif-ww",6)==0) && start_arg[6]==0)
					g_DecryptGame=1;
				
				else if(DEPCASEISTRCMP(start_arg,"sif-jp",7)==0 && start_arg[6]==0)
					g_DecryptGame=2;
				
				else if(DEPCASEISTRCMP(start_arg,"sif-kr",7)==0 && start_arg[6]==0)
					g_DecryptGame=4;
				
				else if(DEPCASEISTRCMP(start_arg,"sif-tw",7)==0 && start_arg[6]==0)
					g_DecryptGame=3;

				else if(DEPCASEISTRCMP(start_arg,"version",8)==0 && start_arg[7]==0)
				{
					fputs("Version " HONOKAMIKU_VERSION_STRING "\n",stdout);
					fputs("Build at " __DATE__ " " __TIME__ "\n",stderr);
					fprintf(stderr,"Compiled using %s\n\n",CompilerName());

					exit(0);
				}

				else if(g_InPos==0 && (*start_arg=='-' || *start_arg=='/'))
					g_InPos=-i;
				else if(g_OutPos==0 && (*start_arg=='-' || *start_arg=='/'))
					g_OutPos=-i;
				else
					cerr << "Ignoring " << argv[i] << endl;
			}
			// Short argument name parse
			else if(s=='b')
			{
				if(argv[i][2]==0)
				{
					i++;
					if(argv[i]==NULL)
						cerr << "Ignoring -b" << endl;
					else
						g_Basename=argv[i];
				}
				else
					g_Basename=argv[i]+2;
			}
			else if(s=='c')
				g_DecryptGame=5;

			else if(s=='d')
				g_TestMode=true;

			else if(s=='e')
				g_Encrypt=true;

			else if(s=='h' || s=='?')
			{
				usage_noexit(argv[0]);
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
				fputs("Version " HONOKAMIKU_VERSION_STRING "\n",stderr);
				fputs("Build at " __DATE__ " " __TIME__ "\n",stderr);
				fprintf(stderr,"Compiled using %s\n\n",CompilerName());

				exit(0);
			}

			else if(s=='w')
				g_DecryptGame=1;

			else if(s=='x')
			{
				if(argv[i][2]==0)
				{
					i++;
					if(argv[i]==NULL)
						cerr << "Ignoring -x" << endl;
					else
						g_XEncryptGame=get_gametype(argv[i]);
				}
				else
					g_XEncryptGame=get_gametype(argv[i]+2);
				if(g_XEncryptGame==0) cerr << "Ignoring -x: Unknown game type" << endl;
			}

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
		failexit(argv[0],"encrypt mode requires game file switch");
	else if(g_Encrypt && g_XEncryptGame)
		failexit(argv[0],"cross-encrypt mode can't be used with encrypt mode");
	else if(g_DecryptGame>0 && g_XEncryptGame==g_DecryptGame)
		exit(0);
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
	
	if(g_InPos>0)
	{
		if(strcmp(argv[g_InPos],"-"))
			in=fopen(argv[g_InPos],"rb");
		else
			in=stdin;
		
		if(in==NULL)
			failexit(errno,argv[g_InPos]);
	}
	else
	{
		const char* s=argv[-g_InPos]+2;
		in=fopen(s,"rb");
		if(in==NULL)
			failexit(errno,s);
	}

	if(g_OutPos>0)
		file_out=argv[g_OutPos];
	else if(g_OutPos<0)
		file_out=argv[-g_OutPos]+2;
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

	if((g_DecryptGame==0 && !g_Encrypt) || g_TestMode)
	{
		char header[16];
		std::ostream& conout=g_TestMode?std::cout:std::cerr;

		if(g_TestMode)
			std::cout << "Detecting: ";
		else
			std::cerr << "Auto detecting: ";
		try
		{
			fread(header,1,4,in);
			g_Dctx=new EN_Dctx(header,g_Basename);
			conout << "EN game file" << std::endl;
			g_DecryptGame=1;
		}
		catch(std::exception)
		{
			fseek(in,0,SEEK_SET);
			try
			{
				fread(header,1,16,in);
				g_Dctx=new JP_Dctx(header,g_Basename);
				conout << "JP game file" << std::endl;
				g_DecryptGame=2;
			}
			catch(std::exception)
			{
				fseek(in,4,SEEK_SET);
				try
				{
					g_Dctx=new TW_Dctx(header,g_Basename);
					conout << "TW game file" << std::endl;
					g_DecryptGame=3;
				}
				catch(std::exception)
				{
					try
					{
						g_Dctx=new KR_Dctx(header,g_Basename);
						conout << "KR game file" << std::endl;
						g_DecryptGame=4;
					}
					catch(std::exception)
					{
						try
						{
							g_Dctx=new CN_Dctx(header,g_Basename);
							conout << "CN game file" << std::endl;
							g_DecryptGame=5;
						}
						catch(std::exception)
						{
							conout << "Unknown" << std::endl;
							if(!g_TestMode)
								failexit(argv[g_InPos],"decrypt","Cannot find suitable decryption method");
						}
					}
				}
			}
		}
		if(g_TestMode)
		{
			if(g_Dctx) delete g_Dctx;
			fclose(in);
			return 0;
		}
		if(g_DecryptGame==g_XEncryptGame) exit(0);
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
		fread(buffer,1,file_size,in);
		g_Dctx->decrypt_block(buffer,file_size);
	}
	else
	{
		out_size-=header_size;
		fread(buffer,1,out_size,in);
		g_Dctx->decrypt_block(buffer,out_size);
	}
	fclose(in);

	out=strcmp(file_out,"-")==0?stdout:fopen(file_out,"wb");
	if(out==NULL)
	{
		std::cerr << "Cannot open " << file_out << ": " << strerror(errno) << std::endl << "Writing to stdout instead" << std::endl;
		out=stdout;
	}

	if(g_Encrypt)
		buffer-=header_size;
	else if(g_XEncryptGame)
	{
		char temp_hdr[16];
		temp_hdr[4]='X';
		delete g_Dctx;

		std::cerr << "Cross-encrypt as: ";

		if(g_XEncryptGame==1)
		{
			g_Dctx=EN_Dctx::encrypt_setup(g_Basename,temp_hdr);
			std::cerr << "EN game file" << std::endl;
		}
		else if(g_XEncryptGame==2)
		{
			g_Dctx=JP_Dctx::encrypt_setup(g_Basename,temp_hdr);
			std::cerr << "JP game file" << std::endl;
		}
		else if(g_XEncryptGame==3)
		{
			g_Dctx=TW_Dctx::encrypt_setup(g_Basename,temp_hdr);
			std::cerr << "TW game file" << std::endl;
		}
		else if(g_XEncryptGame==4)
		{
			g_Dctx=KR_Dctx::encrypt_setup(g_Basename,temp_hdr);
			std::cerr << "KR game file" << std::endl;
		}
		else if(g_XEncryptGame==5)
		{
			g_Dctx=CN_Dctx::encrypt_setup(g_Basename,temp_hdr);
			std::cerr << "CN game file" << std::endl;
		}

		g_Dctx->decrypt_block(buffer,out_size);

		if(temp_hdr[4]=='X')
			fwrite(temp_hdr,1,4,out);
		else
			fwrite(temp_hdr,4,4,out);

	}

	fwrite(buffer,1,out_size,out);
	fclose(out);
	
	delete g_Dctx;
	return 0;
}
