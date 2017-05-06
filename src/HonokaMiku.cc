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

#ifdef WIN32
#include <io.h>
#endif

#include "DecrypterContext.h"
#include "../VersionInfo.rc"

const char* CompilerName()
{
	static char compilername[256];
	static bool already_loaded = false;
	static int v;

	if(already_loaded) return compilername;

	memset(compilername, 0, 256);
#if defined(__clang__) || defined(__ICC) || defined(__INTEL_COMPILER)
	/* Clang/LLVM or Intel Compiler. It support __VERSION__ ----- */
	strcpy(compilername, __VERSION__);
#elif defined(__GNUC__) || defined(__GNUG__)
	/* GCC. -----------------------------------------------------*/
	strcpy(compilername, "gcc-" __VERSION__);
#elif defined(__HP_cc) || defined(__HP_aCC)
	/* Hewlett-Packard C/aC++. ---------------------------------- */
#ifdef __cplusplus
	v=__HP_aCC;
#else
	v=__HP_cc;
#endif
	sprintf(compilername, "HP C/aC++ Version A.%02d.%02d.%02d", v/10000, (v/100)%100, v%100);
#elif defined(__IBMC__) || defined(__IBMCPP__)
	/* IBM XL C/C++. -------------------------------------------- */
	strcpy(compilername,"IBM XL C/C++ Version " __xlc__);
#elif defined(_MSC_VER)
	/* Microsoft Visual Studio. --------------------------------- */
	sprintf(compilername,"MSVC %02d.%02d.%05d.%02d", int(_MSC_VER/100), int(_MSC_VER)%100, int(_MSC_FULL_VER)%100000, _MSC_BUILD);
#elif defined(__PGI)
	/* Portland Group PGCC/PGCPP. ------------------------------- */
	sprintf(compilername, "PGCC/PGCPP Version %02d.%d.%d", __PGIC__, __PGIC_MINOR, __PGIC_PATCHLEVEL__);
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
	/* Oracle Solaris Studio. ----------------------------------- */
#ifdef __cplusplus
	v=__SUNPRO_CC;
#else
	v=__SUNPRO_C;
#endif
	sprintf(compilername, "Solaris Studio Version %x.%x.%x", v/0x1000, (v/0x10)%0x100, v%0x10);
#else
	strcpy(compilername, "Unknown compiler");
#endif
	
	already_loaded = true;
	return compilername;
}

// Copied from MSVC strnicmp.c
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

bool AssembleGameName(int game_prop, char* dest)
{
	const char* gamepref = NULL;
	int32_t ver = game_prop >> 16;

	if(ver > 255)
		return false;

	switch(game_prop & 0xFFFF)
	{
		case HONOKAMIKU_GAMETYPE_JP:
			gamepref = "JP";
			break;
		case HONOKAMIKU_GAMETYPE_EN:
			gamepref = "EN";
			break;
		case HONOKAMIKU_GAMETYPE_TW:
			gamepref = "TW";
			break;
		case HONOKAMIKU_GAMETYPE_CN:
			gamepref = "CN";
			break;
		case 0xFFFF:
			gamepref = "Unk";
		default:
			return false;
	}

	return sprintf(dest, "%s (Version %d) game file", gamepref, ver) > 0;
}

// Must be deleted with delete[]
char* AssembleGameName(int game_prop)
{
	char* temp = new char[32];

	if(AssembleGameName(game_prop, temp))
		return temp;

	delete[] temp;
	return NULL;
}

int32_t GetGameProp(const char* str)
{
	uint32_t game_prop = (-1);

	// Single character check
	if(str[1] == 0 || str[2] == 0)
	{
		uint32_t defver = (str[1] == 0 || str[1] > 57 || str[1] < 49 ? HONOKAMIKU_DECRYPT_V3 : int32_t(str[1] - 48) << 16);

		if(str[0] == 'j')
			game_prop = HONOKAMIKU_GAMETYPE_JP | defver;
		else if(str[0] == 'w')
			game_prop = HONOKAMIKU_GAMETYPE_EN | defver;
		else if(str[0] == 't')
			game_prop = HONOKAMIKU_GAMETYPE_TW | defver;
		else if(str[0] == 'c')
			game_prop = HONOKAMIKU_GAMETYPE_CN | defver;
	}

	if(game_prop != 0xFFFFFFFF)
		return game_prop;

	// Multi-character-check
	uint32_t defver = (-1);

	if(strlen(str) == 9)
	{
		defver = int32_t(str[8] - 48);

		if(defver < 1 || defver > 9)
			return (-1);
		else
			defver <<= 16;
	}
	else
		defver = HONOKAMIKU_DECRYPT_V3;

	if(msvcr110_strnicmp(str, "sif-jp", 6) == 0)
		return HONOKAMIKU_GAMETYPE_JP | defver;
	else if(
		msvcr110_strnicmp(str, "sif-ww", 6) == 0 ||
		msvcr110_strnicmp(str, "sif-en", 6) == 0
	)
		return HONOKAMIKU_GAMETYPE_EN | defver;
	else if(msvcr110_strnicmp(str, "sif-tw", 6) == 0)
		return HONOKAMIKU_GAMETYPE_TW | defver;
	else if(msvcr110_strnicmp(str, "sif-cn", 6) == 0)
		return HONOKAMIKU_GAMETYPE_CN | defver;
	
	return (-1);
}

static char usage_string[] =
	"Usage: %s <input file> [output file=input] [options]\n"
	"<input file> and [output file] can be - for stdin and stdout.\n"
	"\nOptions:\n"
	" -b <name>                 Use basename <name> as decrypt/encrypt\n"
	" -basename <name>          key. Required if reading from stdin.\n"
	"\n"
	" -c[1|2|3]                 Assume <input file> is SIF CN game file.\n"
	" -sif-cn[-v1|v2|v3]        Defaults to version 3\n"
	"\n"
	" -d                        Detect game file type only. [output file]\n"
	" -detect                   and the other parameters is omitted.\n"
	"\n"
	" -e                        Encrypt <input file> instead of decrypting\n"
	" -encrypt                  it. If you use this, one of the game file\n"
	"                           flag must be specificed.\n"
	"\n"
	" -h                        Show this message\n"
	" -?\n"
	" -help\n"
	"\n"
	" -j[1|2|3|4]               Assume <input file> is SIF JP game file.\n"
	" -sif-jp[-v1|v2|v3|v4]     Defaults to version 3\n"
	"\n"
	" -t[1|2|3]                 Assume <input file> is SIF TW game file.\n"
	" -sif-tw[-v1|v2|v3]        Defaults to version 3\n"
	"\n"
	" -v                        Show version information.\n"
	" -version\n"
	"\n"
	" -w[1|2|3]                 Assume <input file> is SIF EN game file.\n"
	" -sif-en[-v1|v2|v3]        Defaults to version 3\n"
	" -sif-ww[-v1|v2|v3]\n"
	"\n"
	" -x <game>                 Cross-encrypt to another game file.\n"
	" -cross-encrypt <game>     <game> can be w, j, t, or c for short\n"
	"                           name (add 1, 2, or 3 to set version).\n"
	"                           For long name, see argument options \n"
	"                           above."
	"\n";

int32_t g_InPos = 0;						// Input filename argv position
int g_OutPos = 0;							// Output filename argv position
const char* g_Basename = NULL;				// Basename
const char* g_ProgramName;					// Executable name.
uint32_t g_DecryptGame = 0xFFFFFFFFU;		// Bitwise now
uint32_t g_XEncryptGame = 0xFFFFFFFFU;		// Bitwise now
bool g_Encrypt = false;						// Encrypt mode?
bool g_TestMode = false;					// Detect only?

void parse_args(int argc, char* argv[])
{
	int i = 1;

	for(char* arg = argv[i]; i < argc; arg = argv[++i])
	{
		if(*arg == '-' && *++arg != 0)
		{
			bool arg_f = false;

			// Arguments that need 1 parameter
			if(argv[i + 1] != NULL)
			{
				if(
					msvcr110_strnicmp("b", arg, 2) == 0 ||
					msvcr110_strnicmp("basename", arg, 9) == 0
				)
				{
					g_Basename = argv[++i];

					arg_f = true;
				}
				else if(
					msvcr110_strnicmp("x", arg, 2) == 0 ||
					msvcr110_strnicmp("cross-encrypt", arg, 14) == 0
				)
				{
					if((g_XEncryptGame = GetGameProp(argv[i++])) == (-1))
						fprintf(stderr, "Cross-encrypt: Invalid game '%s'\n", argv[i]);

					arg_f = true;
				}
			}

			if(arg_f == false)
			{
				// No parameter argument
				if(
					msvcr110_strnicmp("d", arg, 2) == 0 ||
					msvcr110_strnicmp("detect", arg, 7) == 0
				)
					g_TestMode = true;
				else if(
					msvcr110_strnicmp("h", arg, 2) == 0 ||
					msvcr110_strnicmp("?", arg, 2) == 0 ||
					msvcr110_strnicmp("help", arg, 5) == 0
				)
				{
					fprintf(stderr, usage_string, g_ProgramName);
					exit(0);
				}
				else if(
					msvcr110_strnicmp("v", arg, 2) == 0 ||
					msvcr110_strnicmp("version", arg, 8) == 0
				)
				{
					fprintf(stderr,
						"Version %s (%03d%03d%05d)\n"
						"Build at " __DATE__ " " __TIME__ "\n"
						"Compiled with %s\n"
						, HONOKAMIKU_VERSION_STRING, HONOKAMIKU_VERSION_MAJOR, HONOKAMIKU_VERSION_MINOR, HONOKAMIKU_VERSION_PATCH,
						CompilerName()
					);
					exit(0);
				}
				else if(
					msvcr110_strnicmp("e", arg, 2) == 0 ||
					msvcr110_strnicmp("encrypt", arg, 8) == 0
				)
					g_Encrypt = true;
				else if((g_DecryptGame = GetGameProp(arg)) == (-1))
					fprintf(stderr, "Unknown argument: %s\n", arg);
			}
		}
		else if(g_InPos == 0)
			g_InPos = i;
		else if(g_OutPos == 0)
			g_OutPos = i;
		else
			fprintf(stderr, "Ignored: %s\n", argv[i]);
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
	{
		if(memcmp(argv[g_InPos], "-", 2) == 0)
		{
			fputs("Error: basename must be specificed when reading from stdin\n\n", stderr);
			fprintf(stderr, usage_string, g_ProgramName);

			exit(EINVAL);
		}
		g_Basename = __DctxGetBasename(argv[g_InPos]);
	}

	else if(g_Encrypt && g_DecryptGame == (-1))
	{
		fputs("Error: encrypt mode requires game file switch\n\n", stderr);
		fprintf(stderr, usage_string, g_ProgramName);
		
		exit(EINVAL);
	}
	else if(g_Encrypt && g_XEncryptGame != (0xFFFFFFFFU))
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
	char* _reserved_memory = new char[2*1024*1024];	// 2 MB of memory. Used for gamefile string and OOM handling mechanism
	size_t file_contents_length = 0;		// This is the file size
	size_t file_contents_size = 4096;		// This is the memory size
	size_t header_size = 4;					// Used on encrypt mode
	int last_errno = 0;

	// Get program basename
	g_ProgramName = __DctxGetBasename(argv[0]);

	// Set mode to binary
#ifdef WIN32
	_setmode(0, 0x8000);	// stdin
	_setmode(1, 0x8000);	// stdout
#endif

	fputs("HonokaMiku. Universal LL!SIF game files decrypter\n", stderr);

	if(argc < 2)
	{
		fprintf(stderr, usage_string, g_ProgramName);

		return 1;
	}

	parse_args(argc, argv);
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
		uint32_t expected = g_DecryptGame;

		fputs("Detecting: ", stderr);

		if(fread(header_buffer, 1, 4, file_stream) != 4)
		{
			toosmallfilebuffer:

			delete[] _reserved_memory;
			fclose(file_stream);

			fputs("\nError: file is too small\n", stderr);
			return EBADF;
		}

		dctx = HonokaMiku::FindSuitable(g_Basename, header_buffer);

		if (dctx == NULL)
		{
			delete[] _reserved_memory;
			fputs("Unknown\n", stderr);

			if(expected > 0) return (-1);
		}
		else
		{
			if(dctx->version >= 3)
			{
				if(fread(header_buffer, 1, 12, file_stream) != 12)
					goto toosmallfilebuffer;

				try
				{
					dctx->final_setup(g_Basename, header_buffer);
				}
				catch(std::runtime_error& e)
				{
					fprintf(stderr, "\nError: %s\n", e.what());

					return EBADF;
				}
			}

			AssembleGameName(g_DecryptGame = dctx->get_id(), _reserved_memory);
			fprintf(stderr, "%s\n", _reserved_memory);

			if(expected != 0xFFFFFFFFU)
			{
				AssembleGameName(expected, _reserved_memory + 512);
				fprintf(stderr, "Expected: %s\n", _reserved_memory + 512);

				return expected == g_DecryptGame ? 0 : (-1);
			}
		}

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
		dctx = HonokaMiku::RequestEncrypter(g_DecryptGame, g_Basename, header_buffer);

		AssembleGameName(dctx->get_id(), _reserved_memory);
		fprintf(stderr, "Encrypt as: %s\n", _reserved_memory);
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
		else if(g_DecryptGame != 0xFFFFFFFF)
		{
			AssembleGameName(g_DecryptGame, _reserved_memory);
			fprintf(stderr, "Decrypt as: %s\n", _reserved_memory);

			dctx = HonokaMiku::RequestDecrypter(g_DecryptGame, header_buffer, g_Basename);

			if(dctx == NULL)
			{
				delete[] _reserved_memory;
				free(file_contents);
				fclose(file_stream);

				fputs("Error: the specificed method cannot be used to decrypt this file\n", stderr);

				return EINVAL;
			}

			if(dctx->version >= 3)
			{
				if(fread(header_buffer, 1, 12, file_stream) != 12)
				{
					delete dctx;
					fputc('\n', stderr);

					goto file2small_byte_buffer;
				}

				try
				{
					dctx->final_setup(g_Basename, header_buffer, g_DecryptGame >> 16);
				}
				catch(std::runtime_error& e)
				{
					delete dctx;
					delete[] _reserved_memory;
					free(file_contents);
					fclose(file_stream);
					
					fprintf(stderr, "Error: %s\n", e.what());
					return EBADF;
				}
			}
		}
		else
		{
			fputs("Auto-detecting: ", stderr);

			dctx = HonokaMiku::FindSuitable(g_Basename, header_buffer);

			if (dctx == NULL)
			{
				delete[] _reserved_memory;
				free(file_contents);

				fputs("Unknown\nError: no known method to decrypt this file\n", stderr);

				return EINVAL;
			}

			if(dctx->version >= 3)
			{
				if(fread(header_buffer, 1, 12, file_stream) != 12)
				{
					fputc('\n', stderr);
					delete dctx;

					goto file2small_byte_buffer;
				}

				try
				{
					dctx->final_setup(g_Basename, header_buffer);
				}
				catch(std::runtime_error& e)
				{
					delete dctx;
					delete[] _reserved_memory;
					free(file_contents);
					fclose(file_stream);
					
					fprintf(stderr, "\nError: %s\n", e.what());
					return EBADF;
				}
			}

			AssembleGameName(g_DecryptGame = dctx->get_id(), _reserved_memory);
			fprintf(stderr, "%s\n", _reserved_memory);
		}
	}

	
	// Decrypt/encrypt routines
	{
		HonokaMiku::Dctx* cross_dctx = NULL;
		static const size_t chunk_size = 4096;		// Edit if necessary
		size_t version1_consideration = 0;
		unsigned char* byte_buffer;

		try
		{
			byte_buffer = new uint8_t[chunk_size];
		}
		catch(std::bad_alloc& )
		{
			free(file_contents);

			goto not_enough_memory;
		}

		if(dctx->version == 1 && g_Encrypt == false)
		{
			version1_consideration = 4;

			memcpy(byte_buffer, header_buffer, 4);
		}

		if(g_XEncryptGame != 0xFFFFFFFF)
		{
			AssembleGameName(g_XEncryptGame, _reserved_memory + 512);
			fprintf(stderr, "Cross-encrypt to: %s", _reserved_memory + 512);

			if(g_XEncryptGame == g_DecryptGame)
			{
				fputs(" (no operation)\n", stderr);
				goto cleanup;
			}

			fputc('\n', stderr);

			g_Encrypt = true;
			cross_dctx = HonokaMiku::RequestEncrypter(g_XEncryptGame, g_Basename, header_buffer);
		}

		while(size_t read_bytes = fread(byte_buffer + version1_consideration, 1, chunk_size - version1_consideration, file_stream) + version1_consideration)
		{
			version1_consideration = 0;

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
		header_size = HonokaMiku::GetHeaderSize(dctx->get_id());

		if(header_size > 0 && fwrite(header_buffer, 1, header_size, file_stream) != header_size)
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
