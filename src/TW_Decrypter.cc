/**
* TW_Decrypter.cc
* SIF TW game files related things
**/

#include "DecrypterContext.h"

static const uint32_t tw_key_tables[64] = {
	0xA925E518U, 0x5AB9C4A4U, 0x01950558U, 0xACFF7182U,
	0xE8183331U, 0x9D1B6963U, 0x0B8E9D15U, 0x96DAD0BBU,
	0x0F941E35U, 0xC968E363U, 0x2058A6AAU, 0x7176BB02U,
	0x4A4B2403U, 0xED7A4E23U, 0x3BB41EE6U, 0x71634C06U,
	0x7E0DD1DAU, 0x343325C9U, 0xE97B42F6U, 0xF68F3C8FU,
	0x1587DED8U, 0x09935F9BU, 0x3273309BU, 0xEFBC3178U,
	0x94C01BDDU, 0x40CEA3BBU, 0xD5785C8AU, 0x0EC1B98EU,
	0xC8D2D2B6U, 0xEF7D77B1U, 0x71814AAFU, 0x2E838EABU,
	0x6B187F58U, 0xA9BC924EU, 0x6EAB5BA6U, 0x738F6D2FU,
	0xC1B49AA4U, 0xAB6A5D53U, 0xF958F728U, 0x5A0CDB5BU,
	0xB8133931U, 0x923336C3U, 0xB5A41DE0U, 0x5F819B33U,
	0x1F3A76AFU, 0x56FB7A7CU, 0x64AE7167U, 0xF39C00F2U,
	0x8F6F61C4U, 0x6A79B9B9U, 0x5B0AB1A6U, 0xB7F07A0AU,
	0x223035FFU, 0x1AA8664CU, 0x553EDB16U, 0x379230C6U,
	0xA2AEEB8AU, 0xF647D0EAU, 0xA91CB2F6U, 0xBB70F817U,
	0x94D63581U, 0x49A7FAD6U, 0x7BEDDD15U, 0xC6913CEDU,
};

////////////////////
// Version 2 code //
////////////////////

HonokaMiku::TW2_Dctx::TW2_Dctx(const void* header, const char* filename): V2_Dctx(GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_TW), header, filename) {}

uint32_t HonokaMiku::TW2_Dctx::get_id()
{
	return HONOKAMIKU_GAMETYPE_TW | HONOKAMIKU_DECRYPT_V2;
}

////////////////////
// Version 3 code //
////////////////////

HonokaMiku::TW3_Dctx::TW3_Dctx(const void* header, const char* filename): V3_Dctx(GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_TW), header, filename) {}

const uint32_t* HonokaMiku::TW3_Dctx::_getKeyTables() { return tw_key_tables; }

uint32_t HonokaMiku::TW3_Dctx::get_id()
{
	return HONOKAMIKU_GAMETYPE_TW | HONOKAMIKU_DECRYPT_V3;
}

void HonokaMiku::TW3_Dctx::final_setup(const char* filename, const void* block_rest, int force_version)
{
	finalDecryptV3(this, 1051, filename, block_rest, force_version);
}

HonokaMiku::TW3_Dctx* HonokaMiku::TW3_Dctx::encrypt_setup(const char* filename, void* hdr_out, int force_version)
{
	TW3_Dctx* dctx = new TW3_Dctx;
	setupEncryptV3(dctx, GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_TW), 1051, filename, hdr_out, force_version);
	return dctx;
}
