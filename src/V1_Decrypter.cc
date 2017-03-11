/**
* V1_Decrypter.cc
* Routines to decrypt SIF v1.x game files or configuration files
**/

#include <stdint.h>

// Mac OS fix
#include <string>
#include <sstream>
#include <iostream>

#include "DecrypterContext.h"
#include "md5.h"

HonokaMiku::V1_Dctx::V1_Dctx(const char* prefix, const char* filename)
{
	MD5 mctx;
	const char* basename = __DctxGetBasename(filename);
	size_t basename_len = strlen(basename);

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>(prefix), strlen(prefix));
	mctx.Update(reinterpret_cast<const uint8_t*>(basename), basename_len);
	mctx.Final();

	pos = 0;
	update_key = basename_len + 1;
	xor_key = init_key =
		(mctx.digestRaw[0] << 24) |
		(mctx.digestRaw[1] << 16) |
		(mctx.digestRaw[2] << 8) |
		(mctx.digestRaw[3]);

	if(strcmp(prefix, GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_JP)) == 0)
		game_ver = HONOKAMIKU_GAMETYPE_JP;
	else if(strcmp(prefix, GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_EN)) == 0)
		game_ver = HONOKAMIKU_GAMETYPE_EN;
	else if(strcmp(prefix, GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_TW)) == 0)
		game_ver = HONOKAMIKU_GAMETYPE_TW;
	else if(strcmp(prefix, GetPrefixFromGameType(HONOKAMIKU_GAMETYPE_CN)) == 0)
		game_ver = HONOKAMIKU_GAMETYPE_CN;
	else
		game_ver = 0xFFFF;

	version = 1;
}

uint32_t HonokaMiku::V1_Dctx::get_id()
{
	return HONOKAMIKU_DECRYPT_V1 | game_ver;
}

void HonokaMiku::V1_Dctx::decrypt_block(void* b, uint32_t size)
{
	if (size == 0) return;
	
	char* file_buffer = reinterpret_cast<char*>(b);
	uint32_t last_pos = pos % 4;

	if(last_pos == 1)
	{
		*file_buffer++ ^= char(xor_key >> 16);
		size--;

		if(size == 0)
			goto decrypt_block_finish;
		else
			goto first_last_pos_mod2;
	}
	else if(last_pos == 2)
	{
first_last_pos_mod2:
		*file_buffer++ ^= char(xor_key >> 8);
		size--;

		if(size == 0)
			goto decrypt_block_finish;
		else
			goto first_last_pos_mod3;
	}
	else if(last_pos == 3)
	{
first_last_pos_mod3:
		*file_buffer++ ^= char(xor_key);
		size--;

		update();
	}
	
	for (size_t decrypt_size = size / 4; decrypt_size != 0; decrypt_size--, file_buffer += 4)
	{
		file_buffer[0] ^= xor_key >> 24;
		file_buffer[1] ^= char(xor_key >> 16);
		file_buffer[2] ^= char(xor_key >> 8);
		file_buffer[3] ^= char(xor_key);

		update();
	}
	
	if ((size & 0xFFFFFFFCU) != size)
	{
		last_pos = size % 4;
		
		if(last_pos >= 1)
			file_buffer[0] ^= char(xor_key >> 24);
		if(last_pos >= 2)
			file_buffer[1] ^= char(xor_key >> 16);
		if(last_pos >= 3)
			file_buffer[2] ^= char(xor_key >> 8);
	}

decrypt_block_finish:
	pos += size;
}

void HonokaMiku::V1_Dctx::decrypt_block(void* _d, const void* _s, uint32_t size)
{
	if (size == 0) return;
	
	const char* src_buffer = reinterpret_cast<const char*>(_s);
	char* file_buffer = reinterpret_cast<char*>(_d);
	uint32_t last_pos = pos % 4;

	if(last_pos == 1)
	{
		*file_buffer++ = *src_buffer++ ^ char(xor_key >> 16);
		size--;

		if(size == 0)
			goto decrypt_block2_finish;
		else
			goto first2_last_pos_mod2;
	}
	else if(last_pos == 2)
	{
first2_last_pos_mod2:
		*file_buffer++ = *src_buffer++ ^ char(xor_key >> 8);
		size--;

		if(size == 0)
			goto decrypt_block2_finish;
		else
			goto first2_last_pos_mod3;
	}
	else if(last_pos == 3)
	{
first2_last_pos_mod3:
		*file_buffer++ = *src_buffer++ ^ char(xor_key);
		size--;

		update();
	}
	
	for (size_t decrypt_size = size / 4; decrypt_size != 0; decrypt_size--, file_buffer += 4)
	{
		file_buffer[0] = *src_buffer++ ^ xor_key >> 24;
		file_buffer[1] = *src_buffer++ ^ char(xor_key >> 16);
		file_buffer[2] = *src_buffer++ ^ char(xor_key >> 8);
		file_buffer[3] = *src_buffer++ ^ char(xor_key);

		update();
	}
	
	if ((size & 0xFFFFFFFCU) != size)
	{
		last_pos = size % 4;
		
		if(last_pos >= 1)
			file_buffer[0] = src_buffer[0] ^ char(xor_key >> 24);
		if(last_pos >= 2)
			file_buffer[1] = src_buffer[1] ^ char(xor_key >> 16);
		if(last_pos >= 3)
			file_buffer[2] = src_buffer[2] ^ char(xor_key >> 8);
	}

decrypt_block2_finish:
	pos += size;
}

inline void HonokaMiku::V1_Dctx::update()
{
	xor_key += update_key;
}

void HonokaMiku::V1_Dctx::goto_offset(uint32_t offset)
{
	uint32_t current = pos - (pos % 4);
	uint32_t nearest = offset - (offset % 4);

	if(current > nearest)
		xor_key -= update_key;
	else if(nearest > current)
		xor_key += update_key;

	pos = offset;
}

void HonokaMiku::V1_Dctx::goto_offset_relative(int32_t offset)
{
	if(offset == 0) return;

	int64_t x = pos + offset;
	if(x < 0) throw std::runtime_error(std::string("Position is negative."));

	goto_offset(x);
}
