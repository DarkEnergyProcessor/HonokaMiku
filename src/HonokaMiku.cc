/*
* HonokaMiku.cc
* Main program
*/

#include <fstream>
#include <exception>
#include <stdexcept>

#include <cerrno>
#include <cstdio>
#include <cstdlib>

#include "DecrypterContext.h"
#include "CompilerName.h"
#include "../VersionInfo.rc"

// Actually this is `strnicmp` implementation in MSVCRT v110
// Copyright Microsoft Corporation.
int msvcr110_strnicmp (const char * first, const char * last, size_t count)
{
    if(count)
    {
        int f=0;
        int l=0;

        do
        {
            if ( ((f = (unsigned char)(*(first++))) >= 'A') &&
                    (f <= 'Z') )
                f -= 'A' - 'a';

            if ( ((l = (unsigned char)(*(last++))) >= 'A') &&
                    (l <= 'Z') )
                l -= 'A' - 'a';
        }
        while ( --count && f && (f == l) );

        return ( f - l );
    }
    else
    {
        return 0;
    }
}

static char usage_string[] =
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

char g_DecryptGame = 0;			// 0 = not specificed; 1 = ww/en; 2 = jp; 3 = tw; 4 = kr; 5 = cn
bool g_Encrypt = false;			// Encrypt mode?
const char* g_Basename = NULL;	// Basename
int g_InPos = 0;				// Input filename argv position
int g_OutPos = 0;				// Output filename argv position
char g_XEncryptGame = 0;		// 0 = cross-encryption not specificed; 1 = ww/en; 2 = jp; 3 = tw; 4 = kr; 5 = cn
bool g_TestMode = false;		// Detect only.
const char* g_ProgramName;		// Executable name.

char get_gametype(const char* a)
{
	if(msvcr110_strnicmp(a,"w",2)==0 || msvcr110_strnicmp(a,"sif-ww",7)==0 || msvcr110_strnicmp(a,"sif-en",7)==0)
		return 1;
	else if(msvcr110_strnicmp(a,"j",2)==0 || msvcr110_strnicmp(a,"sif-jp",7)==0)
		return 2;
	else if(msvcr110_strnicmp(a,"t",2)==0 || msvcr110_strnicmp(a,"sif-tw",7)==0)
		return 3;
	else if(msvcr110_strnicmp(a,"k",2)==0 || msvcr110_strnicmp(a,"sif-kr",7)==0)
		return 4;
	else if(msvcr110_strnicmp(a,"c",2)==0 || msvcr110_strnicmp(a,"sif-cn",7)==0)
		return 5;
	return 0;
}

const char* gameid_to_string(char gameid)
{
	switch(gameid)
	{
		case 1:
			return "EN game file";
		case 2:
			return "JP game file";
		case 3:
			return "TW game file";
		case 4:
			return "KR game file";
		case 5:
			return "CN game file";
		default:
			return "Unknown";
	}
}

void parse_args(int argc,char* argv[])
{
	int i = 1;
	for(; i < argc; i++)
	{
		if(argv[i][0]=='-' && argv[i][1] != '\0')
		{
			char s = tolower(argv[i][1]);
			if(s == '-')
			{
				// Long-name argument parse
				char* start_arg = argv[i] + 2;

				if(msvcr110_strnicmp(start_arg, "basename", 9) == 0)
				{
					i++;

					if(argv[i] == NULL)
						fprintf(stderr, "Ignoring --basename\n");
					else
						g_Basename = __DctxGetBasename(argv[i]);
				}

				else if(msvcr110_strnicmp(start_arg, "cross-encrypt", 14) == 0)
				{
					i++;

					if(argv[i] == NULL)
						fprintf(stderr, "Ignoring --cross-encrypt\n");
					else
						g_XEncryptGame = get_gametype(argv[i]);

					if(g_XEncryptGame == 0) fprintf(stderr, "Ignoring --cross-encrypt: Unknown game type %s\n", argv[i]);
				}

				else if(msvcr110_strnicmp(start_arg, "encrypt", 8) == 0)
					g_Encrypt=true;

				else if(msvcr110_strnicmp(start_arg, "detect", 7) == 0)
					g_TestMode=true;

				else if(msvcr110_strnicmp(start_arg, "help", 5) == 0)
				{
					fprintf(stderr, usage_string, g_ProgramName);
					exit(0);
				}

				else if(msvcr110_strnicmp(start_arg, "sif-cn", 7) == 0)
					g_DecryptGame = 5;

				else if(msvcr110_strnicmp(start_arg, "sif-en", 7) == 0 || msvcr110_strnicmp(start_arg, "sif-ww", 6) == 0)
					g_DecryptGame = 1;
				
				else if(msvcr110_strnicmp(start_arg, "sif-jp", 7) == 0)
					g_DecryptGame = 2;
				
				else if(msvcr110_strnicmp(start_arg, "sif-kr", 7) == 0)
					g_DecryptGame = 4;
				
				else if(msvcr110_strnicmp(start_arg, "sif-tw", 7) == 0)
					g_DecryptGame = 3;

				else if(msvcr110_strnicmp(start_arg, "version", 8) == 0)
				{
					fputs("Version " HONOKAMIKU_VERSION_STRING "\n", stderr);
					fputs("Build at " __DATE__ " " __TIME__ "\n", stderr);
					fprintf(stderr, "Compiled using %s\n\n", CompilerName());

					exit(0);
				}

				else
					fprintf(stderr, "Ignoring %s\n", argv[i]);
			}
			// Short argument name parse
			else if(s == 'b')
			{
				if(argv[i][2] == 0)
				{
					i++;
					if(argv[i] == NULL)
						fprintf(stderr, "Ignoring -b\n");
					else
						g_Basename = argv[i];
				}
				else
					g_Basename = argv[i] + 2;
			}
			else if(s == 'c')
				g_DecryptGame = 5;

			else if(s == 'd')
				g_TestMode = true;

			else if(s == 'e')
				g_Encrypt = true;

			else if(s == 'h' || s == '?')
			{
				fprintf(stderr, usage_string, g_ProgramName);
				exit(0);
			}

			else if(s == 'j')
				g_DecryptGame = 2;

			else if(s == 'k')
				g_DecryptGame = 4;
			
			else if(s == 't')
				g_DecryptGame = 3;

			else if(s == 'v')
			{
				fputs("Version " HONOKAMIKU_VERSION_STRING "\n", stderr);
				fputs("Build at " __DATE__ " " __TIME__ "\n", stderr);
				fprintf(stderr, "Compiled using %s\n\n", CompilerName());

				exit(0);
			}

			else if(s == 'w')
				g_DecryptGame=1;

			else if(s == 'x')
			{
				if(argv[i][2] == 0)
				{
					i++;

					if(argv[i] == NULL)
						fprintf(stderr, "Ignoring -x\n");
					else
						g_XEncryptGame = get_gametype(argv[i]);
				}
				else
					g_XEncryptGame = get_gametype(argv[i]+2);
				if(g_XEncryptGame == 0) fprintf(stderr, "Ignoring -x: Unknown game type %s\n", argv[i]+2);
			}

			else
				fprintf(stderr, "Ignoring %s: Unknown option\n", argv[i]);
		}
		else if(g_InPos == 0)
			g_InPos = i;
		else if(g_OutPos == 0)
			g_OutPos = i;
		else
			fprintf(stderr, "Ignoring %s\n", argv[i]);
	}
}

void check_args(char* argv[])
{
	// Check input
	if(g_InPos == 0)
	{
		fputs("Error: input file is missing\n\n", stderr);
		fprintf(stderr, usage_string, g_ProgramName);

		exit(EINVAL);
	}
	if(g_OutPos == 0) g_OutPos = g_InPos;

	// Check basename
	if((g_Basename != NULL && strlen(g_Basename)==0) || g_Basename == NULL)
		g_Basename = __DctxGetBasename(argv[g_InPos]);

	if(g_Encrypt && g_DecryptGame == 0)
	{
		fputs("Error: encrypt mode requires game file switch\n\n", stderr);
		fprintf(stderr, usage_string, g_ProgramName);
		
		exit(EINVAL);
	}
	else if(g_Encrypt && g_XEncryptGame)
	{
		fputs("Error: cross-encrypt mode can't be used with encrypt mode\n\n", stderr);
		fprintf(stderr, usage_string, g_ProgramName);
		
		exit(EINVAL);
	}
}

int main(int argc, char* argv[])
{
	FILE* file_stream = NULL;
	HonokaMiku::DecrypterContext* dctx = NULL;
	unsigned char* file_contents = NULL;
	unsigned char header_buffer[16];
	char* filename_input;
	char* filename_output;
	char* _reserved_memory = new char[2*1024*1024];	// 2 MB of memory. If we're running out of memory, delete this.
	size_t file_contents_length = 0;		// This is the file size
	size_t file_contents_size = 4096;		// This is the memory size
	size_t header_size = 4;					// Used on encrypt mode
	int last_errno = 0;

	// Get program basename
	g_ProgramName = __DctxGetBasename(argv[0]);

	fputs("HonokaMiku. Universal LL!SIF game files decrypter\n", stderr);

	if(argc < 2)
	{
		fprintf(stderr, usage_string, g_ProgramName);

		return 1;
	}

	parse_args(argc,argv);
	check_args(argv);

	filename_input = argv[g_InPos];
	filename_output = argv[g_OutPos];

	if(memcmp(filename_input, "-", 2))
	{
		file_stream = fopen(filename_input, "rb");
		last_errno = errno;
	}
	else
		file_stream = stdin;

	if(file_stream == NULL)
	{
		delete[] _reserved_memory;

		fprintf(stderr, "Error: cannot open '%s': %s\n", filename_input, strerror(last_errno));
		return last_errno;
	}

	if(g_TestMode)
	{
		fputs("Detecting: ", stderr);

		if(fread(header_buffer, 1, 4, file_stream) != 4)
		{
			delete[] _reserved_memory;
			fclose(file_stream);

			fputs("\nError: file is too small\n", stderr);
			return EBADF;
		}

		dctx = HonokaMiku::FindSuitable(g_Basename, header_buffer, &g_DecryptGame);

		if (dctx == NULL)
			fputs("Unknown\n", stderr);
		else
			fprintf(stderr, "%s\n", gameid_to_string(g_DecryptGame));

		delete[] _reserved_memory;

		return 0;
	}
	
	file_contents = reinterpret_cast<unsigned char*>(malloc(file_contents_size));
	
	if(file_contents == NULL)
	{
		not_enough_memory:

		delete[] _reserved_memory;
		fclose(file_stream);

		fputs("Error: not enough memory\n", stderr);

		return ENOMEM;
	}

	if(g_Encrypt)
	{
		fprintf(stderr, "Encrypt as: %s\n", gameid_to_string(g_DecryptGame));

		dctx = HonokaMiku::EncryptPrepare(g_DecryptGame, g_Basename, header_buffer);
	}
	else
	{
		if(fread(header_buffer, 1, 4, file_stream) != 4)
		{
			file2small_byte_buffer:

			delete[] _reserved_memory;
			free(file_contents);
			fclose(file_stream);

			fputs("Error: file is too small\n", stderr);

			return EBADF;
		}

		if(g_DecryptGame > 0)
		{
			fprintf(stderr, "Decrypt as: %s\n", gameid_to_string(g_DecryptGame));

			dctx = HonokaMiku::CreateFrom(g_DecryptGame, header_buffer, g_Basename);

			if(dctx == NULL)
			{
				delete[] _reserved_memory;
				free(file_contents);
				fclose(file_stream);

				fputs("Error: the specificed method cannot be used to decrypt this file\n", stderr);

				return EINVAL;
			}

			if(dctx->version == 3)
			{
				version3_finalize_setup:

				if(fread(header_buffer, 1, 12, file_stream) != 12)
				{
					delete dctx;

					goto file2small_byte_buffer;
				}

				dctx->final_setup(g_Basename, header_buffer);
			}
		}
		else
		{
			fputs("Auto-detecting: ", stderr);

			dctx = HonokaMiku::FindSuitable(g_Basename, header_buffer, &g_DecryptGame);

			if (dctx == NULL)
			{
				delete[] _reserved_memory;
				free(file_contents);

				fputs("Unknown\nError: no known method to decrypt this file\n", stderr);

				return EINVAL;
			}

			fprintf(stderr, "%s\n", gameid_to_string(g_DecryptGame));

			if(dctx->version == 3)
			{
				goto version3_finalize_setup;
			}
		}
	}

	
	// Decrypt/encrypt routines
	{
		HonokaMiku::Dctx* cross_dctx = NULL;
		static const size_t chunk_size = 4096;		// Edit if necessary
		unsigned char* byte_buffer;

		if(g_XEncryptGame > 0)
		{
			fprintf(stderr, "Cross-encrypt to: %s", gameid_to_string(g_XEncryptGame));

			if(g_XEncryptGame == g_DecryptGame)
			{
				fputs(" (no operation)\n", stderr);
				goto cleanup;
			}

			fputc('\n', stderr);

			g_Encrypt = true;
			cross_dctx = HonokaMiku::EncryptPrepare(g_XEncryptGame, g_Basename, header_buffer);

		}

		try
		{
			byte_buffer = new unsigned char[chunk_size];
		}
		catch(std::bad_alloc& )
		{
			free(file_contents);

			goto not_enough_memory;
		}
		while(size_t read_bytes = fread(byte_buffer, 1, chunk_size, file_stream))
		{
			for(size_t free_size = file_contents_size - file_contents_length; read_bytes > free_size; )
			{
				if(read_bytes > free_size)
				{
					unsigned char* temp = reinterpret_cast<unsigned char*>(realloc(file_contents, file_contents_size *= 2));

					if(temp == NULL)
					{
						free(file_contents);

						goto not_enough_memory;
					}

					file_contents = temp;
				}
				free_size = file_contents_size - file_contents_length;
			}

			if(cross_dctx)
				// XORing with cross-encrypted first.
				// When the real Dctx reaches, it will result in expected cross-encrypted game file
				cross_dctx->decrypt_block(byte_buffer, read_bytes);

			dctx->decrypt_block(file_contents + file_contents_length, byte_buffer, read_bytes);

			file_contents_length += read_bytes;
		}

		if(cross_dctx)
		{
			// Swap decrypter context
			delete dctx;
			dctx = cross_dctx;
		}
	}

	fclose(file_stream);

	// Open output
	if(memcmp(filename_output, "-", 2))
	{
		file_stream = fopen(filename_output, "wb");
		last_errno = errno;
	}
	else
		file_stream = stdout;

	if(file_stream == NULL)
	{
		fprintf(stderr, "Error: cannot open '%s': %s\nWriting to stdout instead\n", filename_output, strerror(last_errno));

		file_stream = stdout;
	}

	// If we're encrypting, write header first
	if(g_Encrypt)
	{

		if(dctx->version == 3)
			header_size = 16;

		if(fwrite(header_buffer, 1, header_size, file_stream) != header_size)
		{
			write_error:

			last_errno = errno;
			fprintf(stderr, "Error: cannot write to '%s': %s\n", filename_output, strerror(last_errno));

			return last_errno;
		}
	}

	if(fwrite(file_contents, 1, file_contents_length, file_stream) != file_contents_length)
	{
		goto write_error;
	}

	cleanup:

	fclose(file_stream);
	free(file_contents);

	delete[] _reserved_memory;
	delete dctx;

	return 0;
}