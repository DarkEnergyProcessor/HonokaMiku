/**
* V2_Decrypter.cc
* Base class of Version2 decrypter
* Used in all SIF game files except SIF JP which has it's own class
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

HonokaMiku::V2_Dctx::V2_Dctx(const char* prefix, const void* _hdr, const char* filename)
{
	MD5 mctx;
	const uint8_t* header = reinterpret_cast<const uint8_t*>(_hdr);
	const char* basename = __DctxGetBasename(filename);

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>(prefix), strlen(prefix));
	mctx.Update(reinterpret_cast<const uint8_t*>(basename), strlen(basename));
	mctx.Final();

	if(memcmp(header,mctx.digestRaw + 4, 4))
		throw std::runtime_error(std::string("Header file doesn't match."));

	init_key = ((mctx.digestRaw[0] & 0x7F) << 24) |
			   (mctx.digestRaw[1] << 16) |
			   (mctx.digestRaw[2] << 8) |
			   mctx.digestRaw[3];
	update_key = init_key;
	xor_key = ((init_key>>23) & 0xFF) | ((init_key >> 7) & 0xFF00);
	pos = 0;
	version = 2;
}

void HonokaMiku::V2_Dctx::decrypt_block(void* b, uint32_t size)
{
	if (size == 0) return;
	
	char* file_buffer = reinterpret_cast<char*>(b);
	if(pos%2 == 1)
	{
		file_buffer[0] ^= xor_key >> 8;
		file_buffer++;
		size--;
		pos++;

		update();
	}
	
	for (size_t decrypt_size = size / 2; decrypt_size != 0; decrypt_size--, file_buffer += 2)
	{
		file_buffer[0] ^= xor_key;
		file_buffer[1] ^= xor_key >> 8;

		update();
	}
	
	if ((size & 0xFFFFFFFEU) != size)
		file_buffer[0] ^= xor_key;
	
	pos += size;
}

void HonokaMiku::V2_Dctx::decrypt_block(void* _d, const void* _s, uint32_t size)
{
	if (size == 0) return;
	
	const char* file_buffer = reinterpret_cast<const char*>(_s);
	char* out_buffer = reinterpret_cast<char*>(_d);
	if(pos%2 == 1)
	{
		out_buffer[0] = file_buffer[0] ^ (xor_key >> 8);
		file_buffer++;
		out_buffer++;
		size--;
		pos++;

		update();
	}
	
	for (size_t decrypt_size = size / 2; decrypt_size != 0; decrypt_size--, out_buffer += 2, file_buffer += 2)
	{
		out_buffer[0] = file_buffer[0] ^ xor_key;
		out_buffer[1] = file_buffer[1] ^ (xor_key >> 8);

		update();
	}
	
	if ((size & 0xFFFFFFFEU) != size)
		out_buffer[0] = file_buffer[0] ^ xor_key;
	
	pos += size;
}

void HonokaMiku::V2_Dctx::goto_offset(uint32_t offset)
{
	uint32_t loop_times;
	bool reset_dctx = false;

	
	if (offset > pos)
		loop_times = offset - pos;
	else if (offset == pos) return;
	else
	{
		loop_times = offset;
		reset_dctx = true;
	}

	if (reset_dctx)
	{
		update_key = init_key;
		xor_key = ((init_key >> 23) & 0xFF) |
				  ((init_key >> 7) & 0xFF00);
	}
	
	if (pos % 2 == 1 && reset_dctx == 0)
	{
		loop_times--;
		update();
	}
	
	loop_times /= 2;
	
	for(; loop_times != 0; loop_times--)
		update();

	pos = offset;
}

void HonokaMiku::V2_Dctx::goto_offset_relative(int32_t offset)
{
	if(offset == 0) return;

	int64_t x = pos + offset;
	if(x < 0) throw std::runtime_error(std::string("Position is negative."));

	goto_offset(x);
}

inline void HonokaMiku::V2_Dctx::update()
{
	uint32_t a,b,c,d;

	a = update_key >> 16;
	b = ((a * 0x41A70000) & 0x7FFFFFFF) + (update_key & 0xFFFF) * 0x41A7;
	c = (a * 0x41A7) >> 15;
	d = c + b - 0x7FFFFFFF;
	b = b > 0x7FFFFFFE ? d : (b + c);

	update_key = b;
	xor_key = ((b >> 23) & 0xFF) | ((b >> 7) & 0xFF00);
}

void HonokaMiku::setupEncryptV2(V2_Dctx* dctx,const char* prefix,const char* filename,void* hdr_out)
{
	MD5 mctx;
	const char* basename = __DctxGetBasename(filename);
	
	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>(prefix), strlen(prefix));
	mctx.Update(reinterpret_cast<const uint8_t*>(basename), strlen(basename));
	mctx.Final();
	
	memcpy(hdr_out,mctx.digestRaw + 4,4);

	dctx->init_key = ((mctx.digestRaw[0] & 0x7F) << 24) |
					 (mctx.digestRaw[1] << 16) |
					 (mctx.digestRaw[2] << 8) |
					 mctx.digestRaw[3];
	dctx->update_key = dctx->init_key;
	dctx->xor_key = ((dctx->init_key >> 23) & 0xFF)| ((dctx->init_key >> 7) & 0xFF00);
	dctx->pos = 0;
	dctx->version = 2;
}
