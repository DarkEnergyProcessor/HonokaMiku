/**
* V3_Decrypter.cc
* Decrypts SIF version 3 files (JP and EN)
**/

#include <stdint.h>

#include <exception>
#include <stdexcept>
#include <cstring>

#include "DecrypterContext.h"
#include "md5.h"

HonokaMiku::V3_Dctx::V3_Dctx(const char* prefix, const unsigned int* key_tables, const void* header, const char* filename)
{
	MD5 mctx;
	const char* basename = __DctxGetBasename(filename);
	uint8_t digcopy[3];

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>(prefix), strlen(prefix));
	mctx.Update((unsigned char*)basename, strlen(basename));
	mctx.Final();

	memcpy(digcopy,mctx.digestRaw+4,3);
	
	digcopy[0] = ~digcopy[0];
	digcopy[1] = ~digcopy[1];
	digcopy[2] = ~digcopy[2];

	if(memcmp(digcopy, header, 3))
		throw std::runtime_error(std::string("Header file doesn't match."));

	is_finalized = false;
	version = 3;
	this->key_tables = key_tables;
}

void HonokaMiku::finalDecryptV3(V3_Dctx* dctx, unsigned int expected_sum_name, const char* filename, const void* block_rest)
{
	// Already assumed that the first 4 bytes already processed above.
	if (!dctx->is_finalized)
	{
		const char* basename = __DctxGetBasename(filename);
		const uint8_t* second_header = reinterpret_cast<const uint8_t*>(block_rest);
		uint32_t name_sum = expected_sum_name;
		uint32_t expected_sum = second_header[7] | (second_header[6] << 8);

		for(; *basename != 0; name_sum += *basename, basename++) {}

		if (name_sum == expected_sum)
		{
			dctx->init_key = dctx->key_tables[name_sum & 0x3F];
			dctx->update_key = dctx->init_key;
			dctx->xor_key = dctx->init_key >> 24;
			dctx->pos = 0;
			dctx->is_finalized = true;

			return;
		}

		throw std::runtime_error(std::string("Header file doesn't match."));
	}
}

void HonokaMiku::setupEncryptV3(HonokaMiku::V3_Dctx* dctx, const char* prefix, const unsigned int* key_tables, unsigned short name_sum_base, const char* filename, void* hdr_out)
{
	MD5 mctx;
	const char* basename=__DctxGetBasename(filename);
	uint8_t hdr_create[16];
	uint8_t digcopy[3];
	uint16_t key_picker=name_sum_base;

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>(prefix), strlen(prefix));
	mctx.Update((unsigned char*)basename, strlen(basename));
	mctx.Final();

	memcpy(digcopy, mctx.digestRaw + 4, 3);
	memset(hdr_create, 0, 16);
	
	digcopy[0] = ~digcopy[0];
	digcopy[1] = ~digcopy[1];
	digcopy[2] = ~digcopy[2];

	hdr_create[3]=12;

	memcpy(hdr_create,&digcopy,3);

	for(;*basename != 0; key_picker += *basename, basename++) {}

	hdr_create[10] = key_picker >> 8;
	hdr_create[11] = key_picker & 0xFF;

	dctx->init_key = key_tables[hdr_create[11] & 0x3F];
	dctx->update_key = dctx->init_key;
	dctx->xor_key = dctx->init_key >> 24;
	dctx->pos = 0;
	dctx->version = 3;
	dctx->is_finalized = true;
	dctx->key_tables = key_tables;

	memcpy(hdr_out, hdr_create, 16);
}

void HonokaMiku::V3_Dctx::decrypt_block(void* b,uint32_t size)
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

void HonokaMiku::V3_Dctx::decrypt_block(void* _d, const void* _s, uint32_t size)
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

void HonokaMiku::V3_Dctx::goto_offset(uint32_t offset)
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

void HonokaMiku::V3_Dctx::goto_offset_relative(int32_t offset)
{
	if(offset == 0) return;

	int64_t x = pos + offset;
	if(x < 0) throw std::runtime_error(std::string("Position is negative."));

	goto_offset(x);
}

inline void HonokaMiku::V3_Dctx::update() {
	xor_key = (update_key = update_key * 214013 + 2531011) >> 24;
}
