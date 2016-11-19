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
		dctx = new EN3_Dctx(header, filename);
		*game_type = 6;
	}
	catch(std::runtime_error& )
	{
		try
		{
			dctx = new JP3_Dctx(header, filename);
			*game_type = 2;
		}
		catch(std::runtime_error& )
		{
			try
			{
				dctx = new EN2_Dctx(header, filename);
				*game_type = 1;
			}
			catch(std::runtime_error& )
			{
				try
				{
					dctx = new JP2_Dctx(header, filename);
					*game_type = 4;
				}
				catch(std::runtime_error& )
				{
					try
					{
						dctx = new TW2_Dctx(header, filename);
						*game_type = 3;
					}
					catch(std::runtime_error)
					{
						try
						{
							dctx = new CN2_Dctx(header, filename);
							*game_type = 5;
						}
						catch(std::runtime_error)
						{
							try
							{
								dctx = new CN3_Dctx(header, filename);
								*game_type = 7;
							}
							catch(std::runtime_error)
							{
								try
								{
									dctx = new TW3_Dctx(header, filename);
									*game_type = 8;
								}
								catch(std::runtime_error)
								{}
							}
						}
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
				return new EN2_Dctx(header, filename);
			case 2:
				return new JP3_Dctx(header, filename);
			case 3:
				return new TW2_Dctx(header, filename);
			case 4:
				return new JP2_Dctx(header, filename);
			case 5:
				return new CN2_Dctx(header, filename);
			case 6:
				return new EN3_Dctx(header, filename);
			case 7:
				return new CN3_Dctx(header, filename);
			case 8:
				return new TW3_Dctx(header, filename);
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
			return EN2_Dctx::encrypt_setup(filename, header_out);
		case 2:
			return JP3_Dctx::encrypt_setup(filename, header_out);
		case 3:
			return TW2_Dctx::encrypt_setup(filename, header_out);
		case 4:
			return JP2_Dctx::encrypt_setup(filename, header_out);
		case 5:
			return CN2_Dctx::encrypt_setup(filename, header_out);
		case 6:
			return EN3_Dctx::encrypt_setup(filename, header_out);
		case 7:
			return CN3_Dctx::encrypt_setup(filename, header_out);
		case 8:
			return TW3_Dctx::encrypt_setup(filename, header_out);
		default:
			return NULL;
	}
}
