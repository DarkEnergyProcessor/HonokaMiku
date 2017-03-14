/**
* EN_Decrypter.cc
* SIF EN game files related things
**/

#include "DecrypterContext.h"

static const uint32_t en_key_tables[64] = {
	2861607190u	,3623207331u,3775582911u,3285432773u,
	2211141973u	,3078448744u,464780620u	,714479011u,
	439907422u	,421011207u	,2997499268u,630739911u,
	1488792645u	,1334839443u,3136567329u,796841981u,
	2604917769u	,4035806207u,693592067u	,1142167757u,
	1158290436u	,568289681u	,3621754479u,3645263650u,
	4125133444u	,3226430103u,3090611485u,1144327221u,
	879762852u	,2932733487u,1916506591u,2754493440u,
	1489123288u	,3555253860u,2353824933u,1682542640u,
	635743937u	,3455367432u,532501229u	,4106615561u,
	2081902950u	,143042908u	,2637612210u,1140910436u,
	3402665631u	,334620177u	,1874530657u,863688911u,
	1651916050u	,1216533340u,2730854202u,1488870464u,
	2778406960u	,3973978011u,1602100650u,2877224961u,
	1406289939u	,1442089725u,2196364928u,2599396125u,
	2963448367u	,3316646782u,322755307u	,3531653795u
};

////////////////////
// Version 2 code //
////////////////////

HonokaMiku::EN2_Dctx::EN2_Dctx(const void* header, const char* filename): V2_Dctx(GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_EN), header, filename) {}

uint32_t HonokaMiku::EN2_Dctx::get_id()
{
	return HONOKAMIKU_GAMETYPE_EN | HONOKAMIKU_DECRYPT_V2;
}

////////////////////
// Version 3 code //
////////////////////

HonokaMiku::EN3_Dctx::EN3_Dctx(const void* header, const char* filename): V3_Dctx(GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_EN), header, filename) {}

const uint32_t* HonokaMiku::EN3_Dctx::_getKeyTables() { return en_key_tables; }

uint32_t HonokaMiku::EN3_Dctx::get_id()
{
	return HONOKAMIKU_GAMETYPE_EN | HONOKAMIKU_DECRYPT_V3;
}

void HonokaMiku::EN3_Dctx::final_setup(const char* filename, const void* block_rest, int force_version)
{
	finalDecryptV3(this, 844, filename, block_rest, force_version);
}

HonokaMiku::EN3_Dctx* HonokaMiku::EN3_Dctx::encrypt_setup(const char* filename, void* hdr_out, int force_version)
{
	EN3_Dctx* dctx = new EN3_Dctx;
	setupEncryptV3(dctx, GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_EN), 844, filename, hdr_out, force_version);
	return dctx;
}
