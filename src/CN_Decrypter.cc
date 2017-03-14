/**
* CN_Decrypter.cc
* SIF CN game files related things
**/

#include "DecrypterContext.h"

static const uint32_t cn_key_tables[64] = {
	0x1b695658u, 0x0a43a213u, 0x0ead0863u, 0x1400056du,
	0xd470461du, 0xb6152300u, 0xfbe054bcu, 0x9ac9f112u,
	0x23d3cab6u, 0xcd8fe028u, 0x6905bd74u, 0x01a3a612u, 
	0x6e96a579u, 0x333d7ad1u, 0xb6688bffu, 0x29160495u, 
	0xd7743bcfu, 0x8ede97bbu, 0xcacb7e8du, 0x24d81c23u, 
	0xdbfc6947u, 0xb07521c8u, 0xf506e2aeu, 0x3f48df2fu, 
	0x52beb172u, 0x695935e8u, 0x13e2a0a9u, 0xe2edf409u, 
	0x96cba5c1u, 0xdbb1e890u, 0x4c2af968u, 0x17fd17c6u, 
	0x1b9af5a8u, 0x97c0bc25u, 0x8413c879u, 0xd9b13fe1u, 
	0x4066a948u, 0x9662023au, 0x74a4feeeu, 0x1f24b4f6u, 
	0x637688c8u, 0x7a7ccf70u, 0x91042eecu, 0x57edd02cu, 
	0x666da2ddu, 0x92839de9u, 0x43baa9edu, 0x024a8e2cu, 
	0xd4ee7b72u, 0x34c18b72u, 0x13b275c4u, 0xed506a6eu, 
	0xbc1c29b9u, 0xfa66a220u, 0xc2364de3u, 0x767e52b2u, 
	0xe2d32439u, 0xe6f0cef5u, 0xd18c8687u, 0x14bba295u, 
	0xcd84d15bu, 0xa0290f82u, 0xd3e95afcu, 0x9c6a97b4u
};

////////////////////
// Version 2 code //
////////////////////

HonokaMiku::CN2_Dctx::CN2_Dctx(const void* header, const char* filename): V2_Dctx(GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_CN), header, filename) {}

uint32_t HonokaMiku::CN2_Dctx::get_id()
{
	return HONOKAMIKU_GAMETYPE_CN | HONOKAMIKU_DECRYPT_V2;
}

////////////////////
// Version 3 code //
////////////////////

HonokaMiku::CN3_Dctx::CN3_Dctx(const void* header, const char* filename): V3_Dctx(GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_CN), header, filename) {}

const uint32_t* HonokaMiku::CN3_Dctx::_getKeyTables() { return cn_key_tables; }

uint32_t HonokaMiku::CN3_Dctx::get_id()
{
	return HONOKAMIKU_GAMETYPE_CN | HONOKAMIKU_DECRYPT_V3;
}

void HonokaMiku::CN3_Dctx::final_setup(const char* filename, const void* block_rest, int32_t force_version)
{
	finalDecryptV3(this, 1847, filename, block_rest, force_version);
}

HonokaMiku::CN3_Dctx* HonokaMiku::CN3_Dctx::encrypt_setup(const char* filename, void* hdr_out, int32_t fv)
{
	CN3_Dctx* dctx = new CN3_Dctx;
	setupEncryptV3(dctx, GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_CN), 1847, filename, hdr_out, fv);
	return dctx;
}
