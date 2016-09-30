/**
* JP_Decrypter.cc
* SIF JP game files related things
**/

#include "DecrypterContext.h"

static const unsigned int jp_key_tables[64] = {
	1210253353u	,1736710334u,1030507233u,1924017366u,
	1603299666u	,1844516425u,1102797553u,32188137u	,
	782633907u	,356258523u	,957120135u	,10030910u	,
	811467044u	,1226589197u,1303858438u,1423840583u,
	756169139u	,1304954701u,1723556931u,648430219u	,
	1560506399u	,1987934810u,305677577u	,505363237u	,
	450129501u	,1811702731u,2146795414u,842747461u	,
	638394899u	,51014537u	,198914076u	,120739502u	,
	1973027104u	,586031952u	,1484278592u,1560111926u,
	441007634u	,1006001970u,2038250142u,232546121u	,
	827280557u	,1307729428u,775964996u	,483398502u	,
	1724135019u	,2125939248u,742088754u	,1411519905u,
	136462070u	,1084053905u,2039157473u,1943671327u,
	650795184u	,151139993u	,1467120569u,1883837341u,
	1249929516u	,382015614u	,1020618905u,1082135529u,
	870997426u	,1221338057u,1623152467u,1020681319u
};

HonokaMiku::JP3_Dctx::JP3_Dctx(const void* header, const char* filename): HonokaMiku::V3_Dctx(HonokaMiku::GetPrefixFromGameId(2), jp_key_tables, header, filename) {}

void HonokaMiku::JP3_Dctx::final_setup(const char* filename, const void* block_rest)
{
	finalDecryptV3(this, 500, filename, block_rest);
}

HonokaMiku::JP3_Dctx* HonokaMiku::JP3_Dctx::encrypt_setup(const char* filename,void* hdr_out)
{
	JP3_Dctx* dctx = new JP3_Dctx;
	setupEncryptV3(dctx, HonokaMiku::GetPrefixFromGameId(2), jp_key_tables, 500, filename, hdr_out);
	return dctx;
}
