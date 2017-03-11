/*
* Helper.cc
* Utilities helper function
*/

#include <exception>
#include <stdexcept>

#include "DecrypterContext.h"

#define MAKE_FACTORY_FUNCTION(gametype) \
	static HonokaMiku::DecrypterContext* factory_##gametype(uint32_t dec, const char* filename, const void* header) \
	{ \
		HonokaMiku::DecrypterContext* x = NULL; \
		\
		switch(dec) \
		{ \
			case HONOKAMIKU_DECRYPT_V1: \
				x = new HonokaMiku::V1_Dctx(HonokaMiku::GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_##gametype), filename); break; \
			case HONOKAMIKU_DECRYPT_V2: \
				x = new HonokaMiku::gametype##2_Dctx(header, filename); break; \
			case HONOKAMIKU_DECRYPT_V3: \
			case HONOKAMIKU_DECRYPT_V4: \
			case HONOKAMIKU_DECRYPT_V5: \
			case HONOKAMIKU_DECRYPT_V6: \
			case HONOKAMIKU_DECRYPT_V7: \
				x = new HonokaMiku::gametype##3_Dctx(header, filename); break; \
		} \
		return x; \
	}

MAKE_FACTORY_FUNCTION(JP);
MAKE_FACTORY_FUNCTION(EN);
MAKE_FACTORY_FUNCTION(TW);
MAKE_FACTORY_FUNCTION(CN);

typedef HonokaMiku::DecrypterContext*(*FactoryFunc)(uint32_t , const char* , const void* );


FactoryFunc DecrypterConstructors[] = {
	&factory_JP,
	&factory_EN,
	&factory_TW,
	&factory_CN
};

HonokaMiku::DecrypterContext* HonokaMiku::FindSuitable(const char* filename, const void* header)
{
	DecrypterContext* dctx = NULL;

	try
	{
		dctx = new EN3_Dctx(header, filename);
	}
	catch(std::runtime_error& )
	{
		try
		{
			dctx = new JP3_Dctx(header, filename);
		}
		catch(std::runtime_error& )
		{
			try
			{
				dctx = new EN2_Dctx(header, filename);
			}
			catch(std::runtime_error& )
			{
				try
				{
					dctx = new JP2_Dctx(header, filename);
				}
				catch(std::runtime_error& )
				{
					try
					{
						dctx = new TW2_Dctx(header, filename);
					}
					catch(std::runtime_error)
					{
						try
						{
							dctx = new CN2_Dctx(header, filename);
						}
						catch(std::runtime_error)
						{
							try
							{
								dctx = new CN3_Dctx(header, filename);
							}
							catch(std::runtime_error)
							{
								try
								{
									dctx = new TW3_Dctx(header, filename);
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

HonokaMiku::DecrypterContext* HonokaMiku::RequestDecrypter(uint32_t game_prop, const void* header, const char* filename)
{
	try
	{
		if((game_prop & 0xFFFF) > HONOKAMIKU_GAMETYPE_CN)
			return NULL;

		return DecrypterConstructors[game_prop & 0xFFFF](game_prop & 0xFFFF0000U, filename, header);
	}
	catch(std::runtime_error& )
	{
		return NULL;
	}
}

HonokaMiku::DecrypterContext* HonokaMiku::RequestEncrypter(uint32_t game_prop, const char* filename, void* header_out)
{
	uint32_t dectype = game_prop & 0xFFFF0000U;
	uint32_t gt = game_prop & 0xFFFF;

	if(dectype == HONOKAMIKU_DECRYPT_V1)
		return new V1_Dctx(GetPrefixFromGameType(game_prop & 0xFFFF), filename);
	else if(dectype == HONOKAMIKU_DECRYPT_V2)
	{
		switch(gt)
		{
			case HONOKAMIKU_GAMETYPE_JP: return JP2_Dctx::encrypt_setup(filename, header_out);
			case HONOKAMIKU_GAMETYPE_EN: return EN2_Dctx::encrypt_setup(filename, header_out);
			case HONOKAMIKU_GAMETYPE_TW: return TW2_Dctx::encrypt_setup(filename, header_out);
			case HONOKAMIKU_GAMETYPE_CN: return CN2_Dctx::encrypt_setup(filename, header_out);
			default: return NULL;
		}
	}
	else if(dectype >= HONOKAMIKU_DECRYPT_V3)
	{
		int32_t fv = dectype >> 16;
		fv = fv == 0xFFFF ? 0 : fv;

		switch(gt)
		{
			case HONOKAMIKU_GAMETYPE_JP: return JP3_Dctx::encrypt_setup(filename, header_out, fv);
			case HONOKAMIKU_GAMETYPE_EN: return EN3_Dctx::encrypt_setup(filename, header_out, fv);
			case HONOKAMIKU_GAMETYPE_TW: return TW3_Dctx::encrypt_setup(filename, header_out, fv);
			case HONOKAMIKU_GAMETYPE_CN: return CN3_Dctx::encrypt_setup(filename, header_out, fv);
			default: return NULL;
		}
	}
	else
		return NULL;
}
