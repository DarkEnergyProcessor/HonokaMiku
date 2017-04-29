/**
* V3_Decrypter.cc
* Decrypts SIF version 3 files (JP and EN)
**/

#include <stdint.h>

#include <exception>
#include <stdexcept>
#include <cstring>

// Mac OS fix
#include <string>
#include <sstream>
#include <iostream>

#include "DecrypterContext.h"
#include "md5.h"

HonokaMiku::V3_Dctx::V3_Dctx(const char* prefix, const void* header, const char* filename):
is_finalized(false),
_decryptFunc(&decryptV3),
_jumpFunc(&jumpV3)
{
	MD5 mctx;
	const char* basename = __DctxGetBasename(filename);
	uint8_t digcopy[3];

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>(prefix), strlen(prefix));
	mctx.Update(reinterpret_cast<const uint8_t*>(basename), strlen(basename));
	mctx.Final();

	memcpy(digcopy, mctx.digestRaw + 4, 3);
	
	digcopy[0] = ~digcopy[0];
	digcopy[1] = ~digcopy[1];
	digcopy[2] = ~digcopy[2];

	if(memcmp(digcopy, header, 3))
		throw std::runtime_error(std::string("Header file doesn't match."));

	is_finalized = false;
	init_key = (mctx.digestRaw[8] << 24) |
			   (mctx.digestRaw[9] << 16) |
			   (mctx.digestRaw[10] << 8) | mctx.digestRaw[11];
	version = 3;
}

void HonokaMiku::finalDecryptV3(V3_Dctx* dctx, unsigned int expected_sum_name, const char* filename, const void* block_rest, int32_t force_version)
{
	// Already assumed that the first 4 bytes already processed above.
	if (!dctx->is_finalized)
	{
		const char* basename = __DctxGetBasename(filename);
		const uint8_t* second_header = reinterpret_cast<const uint8_t*>(block_rest);

		if(!force_version || force_version == 3)
		{
			uint32_t name_sum = expected_sum_name;
			uint32_t expected_sum = second_header[7] | (second_header[6] << 8);

			for(; *basename != 0; name_sum += *basename, basename++) {}

			if (name_sum == expected_sum)
			{
				dctx->init_key = dctx->_getKeyTables()[name_sum & 0x3F];
				dctx->xor_key = dctx->update_key = dctx->init_key;
				dctx->shift_val = 24;
				dctx->add_val = 2531011;
				dctx->mul_val = 214013;
				dctx->pos = 0;
				dctx->is_finalized = true;

				return;
			}

			if(force_version)
				throw std::runtime_error(std::string("Name sum counter doesn't match."));
		}

		if(!force_version || force_version >= 4)
		{
			if(second_header[3] >= 2)
			{
				// V4+ encryption
				const uint32_t* validx = dctx->_getLngKeyTables();

				if(validx)
				{
					// This Dctx supports V4+
					validx += 3 * (second_header[2] & 3);

					dctx->xor_key = dctx->update_key = dctx->init_key;
					dctx->mul_val = validx[0];
					dctx->shift_val = validx[2];
					dctx->add_val = validx[1];
					dctx->version = 4;
					dctx->pos = 0;
					dctx->is_finalized = true;

					return;
				}

				if(force_version)
					throw std::runtime_error(std::string("This decrypter context doesn't support V4+."));
			}

			throw std::runtime_error(std::string("Invalid V4+ encryption."));
		}

		throw std::runtime_error(std::string("No suitable V3+ encryption format detected."));
	}
}

void HonokaMiku::setupEncryptV3(HonokaMiku::V3_Dctx* dctx, const char* prefix, unsigned short name_sum_base, const char* filename, void* hdr_out, int32_t fv)
{
	MD5 mctx;
	const char* basename = __DctxGetBasename(filename);
	const uint32_t* lcg_ktbl = dctx->_getLngKeyTables();
	uint8_t* hdr_create = reinterpret_cast<uint8_t*>(hdr_out);
	uint8_t digcopy[3];

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>(prefix), strlen(prefix));
	mctx.Update((unsigned char*)basename, strlen(basename));
	mctx.Final();

	memcpy(digcopy, mctx.digestRaw + 4, 3);
	memset(hdr_create, 0, 16);
	hdr_create[3] = 12;
	
	digcopy[0] = ~digcopy[0];
	digcopy[1] = ~digcopy[1];
	digcopy[2] = ~digcopy[2];
	memcpy(hdr_create, &digcopy, 3);

	if(fv == 0 || fv == 3)
	{
		uint16_t key_picker = name_sum_base;

		for(;*basename != 0; key_picker += *basename, basename++) {}

		hdr_create[10] = key_picker >> 8;
		hdr_create[11] = key_picker & 0xFF;
		
		dctx->init_key = dctx->_getKeyTables()[hdr_create[11] & 0x3F];
		dctx->xor_key = dctx->update_key = dctx->init_key;
		dctx->pos = 0;
		dctx->version = 3;
		dctx->shift_val = 24;
		dctx->add_val = 2531011;
		dctx->mul_val = 214013;
		dctx->is_finalized = true;
	}
	else if(fv == 4 && lcg_ktbl != NULL)
	{
		hdr_create[4] = 0x2C;
		hdr_create[7] = 2;

		dctx->init_key = (mctx.digestRaw[8] << 24) |
						 (mctx.digestRaw[9] << 16) |
						 (mctx.digestRaw[10] << 8) |
						 mctx.digestRaw[11];
		dctx->xor_key = dctx->update_key = dctx->init_key;
		dctx->pos = 0;
		dctx->version = 4;
		dctx->mul_val = lcg_ktbl[0];
		dctx->shift_val = lcg_ktbl[2];
		dctx->add_val = lcg_ktbl[1];
		dctx->is_finalized = true;
	}
	else
		throw std::runtime_error("No suitable or invalid encryption method.");
}

void HonokaMiku::V3_Dctx::decryptV3(V3_Dctx* dctx, void* b, uint32_t size)
{
	// Other checking is already done in decrypt_block
	uint8_t* buffer = reinterpret_cast<uint8_t*>(b);
	uint32_t i;

	for(i = dctx->xor_key; size; i = (dctx->update_key = dctx->mul_val * dctx->update_key + dctx->add_val), size--)
		*buffer++ ^= uint8_t(i >> dctx->shift_val);

	dctx->xor_key = i;
}

void HonokaMiku::V3_Dctx::jumpV3(V3_Dctx* dctx, uint32_t offset)
{
	uint32_t loop_times;
	bool reset_dctx = false;

	if (offset > dctx->pos)
		loop_times = offset - dctx->pos;
	else if (offset == dctx->pos) return;
	else
	{
		loop_times = offset;
		reset_dctx = true;
	}
	
	if (reset_dctx)
		dctx->xor_key = dctx->update_key = dctx->init_key;

	for(; loop_times != 0; loop_times--)
		dctx->xor_key = (dctx->update_key = (dctx->mul_val * dctx->update_key) + dctx->add_val);

	dctx->pos = offset;
}

void HonokaMiku::V3_Dctx::decrypt_block(void* b, uint32_t size)
{
	if(size == 0) return;

	if(is_finalized)
	{
		_decryptFunc(this, b, size);

		pos+=size;
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
		
		// Actually it doesn't efficient, but this is beta release anyway
		memcpy(_d, _s, size);
		_decryptFunc(this, out_buffer, size);

		pos += size;
		return;
	}

	throw std::runtime_error(std::string("Decrypter is not fully initialized."));
}

void HonokaMiku::V3_Dctx::goto_offset(uint32_t offset)
{
	if(!is_finalized) throw std::runtime_error(std::string("Decrypter is not fully initialized."));
	
	_jumpFunc(this, offset);
}

void HonokaMiku::V3_Dctx::goto_offset_relative(int32_t offset)
{
	if(offset == 0) return;

	int64_t x = pos + offset;
	if(x < 0) throw std::runtime_error(std::string("Position is negative."));

	_jumpFunc(this, uint32_t(x));
}

inline void HonokaMiku::V3_Dctx::update() {
	//xor_key = (update_key = update_key * 214013 + 2531011) >> 24;
}

const uint32_t* HonokaMiku::V3_Dctx::_getLngKeyTables() { return NULL; }
