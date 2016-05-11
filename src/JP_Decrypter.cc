/**
* JP_Decrypter.cc
* Decrypts SIF JP game files
**/

#include <stdint.h>

#include <exception>
#include <stdexcept>
#include <cstring>

#include "DecrypterContext.h"
#include "md5.h"

static const unsigned int keyTables[64] = {
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

HonokaMiku::JP_Dctx::JP_Dctx(const void* header, const char* filename)
{
	MD5 mctx;
	const char* basename = __DctxGetBasename(filename);
	uint8_t digcopy[3];

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>("Hello"), 5);
	mctx.Update((unsigned char*)basename, strlen(basename));
	mctx.Final();

	memcpy(digcopy,mctx.digestRaw+4,3);
	
	digcopy[0] = ~digcopy[0];
	digcopy[1] = ~digcopy[1];
	digcopy[2] = ~digcopy[2];

	if(memcmp(digcopy,header,3))
		throw std::runtime_error(std::string("Header file doesn't match."));

	is_finalized = false;
	version = 3;
}

void HonokaMiku::JP_Dctx::final_setup(const char* filename, const void* block_rest)
{
	// Already assumed that the first 4 bytes already processed above.
	if (!is_finalized)
	{
		const char* basename = __DctxGetBasename(filename);
		const uint8_t* second_header = reinterpret_cast<const uint8_t*>(block_rest);
		uint32_t name_sum = 500;
		uint32_t expected_sum = second_header[7] | (second_header[6] << 8);

		for(; *basename != 0; name_sum += *basename, basename++) {}

		if (name_sum == expected_sum)
		{
			init_key = keyTables[name_sum & 0x3F];
			update_key = init_key;
			xor_key = init_key >> 24;
			pos = 0;
			is_finalized = true;

			return;
		}

		throw std::runtime_error(std::string("Header file doesn't match."));
	}
}

HonokaMiku::JP_Dctx* HonokaMiku::JP_Dctx::encrypt_setup(const char* filename,void* hdr_out)
{
	JP_Dctx* dctx=new JP_Dctx;
	MD5 mctx;
	const char* basename=__DctxGetBasename(filename);
	uint8_t hdr_create[16];
	uint8_t digcopy[3];
	uint16_t key_picker=500;

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>("Hello"),5);
	mctx.Update((unsigned char*)basename,strlen(basename));
	mctx.Final();

	memcpy(digcopy,mctx.digestRaw+4,3);
	memset(hdr_create,0,16);
	
	digcopy[0] = ~digcopy[0];
	digcopy[1] = ~digcopy[1];
	digcopy[2] = ~digcopy[2];

	hdr_create[3]=12;

	memcpy(hdr_create,&digcopy,3);

	for(;*basename != 0; key_picker += *basename, basename++) {}

	hdr_create[10] = key_picker >> 8;
	hdr_create[11] = key_picker & 0xFF;

	dctx->init_key = keyTables[hdr_create[11] & 0x3F];
	dctx->update_key = dctx->init_key;
	dctx->xor_key = dctx->init_key >> 24;
	dctx->pos = 0;
	dctx->version = 3;
	dctx->is_finalized = true;

	memcpy(hdr_out,hdr_create,16);
	return dctx;
}

void HonokaMiku::JP_Dctx::decrypt_block(void* b,uint32_t size)
{
	if(size == 0) return;

	if(is_finalized)
	{
		uint8_t* buffer = reinterpret_cast<uint8_t*>(b);
		
		pos+=size;

		for(; size != 0; buffer++, size--)
		{
			*buffer ^= uint8_t(xor_key);
			update();
		}

		return;
	}

	throw std::runtime_error(std::string("Decrypter is not fully initialized."));
}

void HonokaMiku::JP_Dctx::decrypt_block(void* _d, const void* _s, uint32_t size)
{
	if(size == 0) return;

	if(is_finalized)
	{
		uint8_t* out_buffer = reinterpret_cast<uint8_t*>(_d);
		const uint8_t* in_buffer = reinterpret_cast<const uint8_t*>(_s);
		
		pos+=size;

		for(; size != 0; out_buffer++, in_buffer++, size--)
		{
			*out_buffer = *in_buffer ^ uint8_t(xor_key);
			update();
		}

		return;
	}

	throw std::runtime_error(std::string("Decrypter is not fully initialized."));
}

void HonokaMiku::JP_Dctx::goto_offset(uint32_t offset)
{
	uint32_t loop_times;
	bool reset_dctx = false;

	if(!is_finalized) throw std::runtime_error(std::string("Decrypter is not fully initialized."));
	
	if (offset > pos)
		loop_times = offset - pos;
	else if (offset == pos) return;
	else
	{
		loop_times = offset;
		reset_dctx = true;
	}
	
	if (reset_dctx)
		xor_key = (update_key = init_key) >> 24;

	for(; loop_times != 0; loop_times--)
		update();

	pos = offset;
}

void HonokaMiku::JP_Dctx::goto_offset_relative(int32_t offset)
{
	if(offset == 0) return;

	int64_t x = pos + offset;
	if(x < 0) throw std::runtime_error(std::string("Position is negative."));

	goto_offset(x);
}

inline void HonokaMiku::JP_Dctx::update() {
	xor_key = (update_key = update_key * 214013 + 2531011) >> 24;
}
