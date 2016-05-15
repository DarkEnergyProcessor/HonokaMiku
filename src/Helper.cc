/*
* Helper.cc
* Utilities helper function
*/

#include <exception>
#include <stdexcept>

#include "DecrypterContext.h"


HonokaMiku::DecrypterContext* HonokaMiku::FindSuitable(const char* filename, const void* header, char* game_type)
{
	char temp;
	DecrypterContext* dctx = NULL;

	if(game_type == NULL) game_type = &temp;

	try
	{
		dctx = new EN_Dctx(header, filename);
		*game_type = 1;
	}
	catch(std::runtime_error& )
	{
		try
		{
			dctx = new JP_Dctx(header, filename);
			*game_type = 2;
		}
		catch(std::runtime_error& )
		{
			try
			{
				dctx = new TW_Dctx(header, filename);
				*game_type = 3;
			}
			catch(std::runtime_error& )
			{
				try
				{
					dctx = new KR_Dctx(header, filename);
					*game_type = 4;
				}
				catch(std::runtime_error& )
				{
					try
					{
						dctx = new CN_Dctx(header, filename);
						*game_type = 5;
					}
					catch(std::runtime_error)
					{
					}
				}
			}
		}
	}

	return dctx;
}

HonokaMiku::DecrypterContext* HonokaMiku::CreateFrom(char game_id, const void* header, const char* filename)
{
	try
	{
		switch(game_id)
		{
			case 1:
				return new EN_Dctx(header, filename);
			case 2:
				return new JP_Dctx(header, filename);
			case 3:
				return new TW_Dctx(header, filename);
			case 4:
				return new KR_Dctx(header, filename);
			case 5:
				return new CN_Dctx(header, filename);
			default:
				return NULL;
		}
	}
	catch(std::runtime_error& )
	{
		return NULL;
	}
}

HonokaMiku::DecrypterContext* HonokaMiku::EncryptPrepare(char game_id, const char* filename, void* header_out)
{
	switch(game_id)
	{
		case 1:
			return EN_Dctx::encrypt_setup(filename, header_out);
		case 2:
			return JP_Dctx::encrypt_setup(filename, header_out);
		case 3:
			return TW_Dctx::encrypt_setup(filename, header_out);
		case 4:
			return KR_Dctx::encrypt_setup(filename, header_out);
		case 5:
			return CN_Dctx::encrypt_setup(filename, header_out);
		default:
			return NULL;
	}
}
