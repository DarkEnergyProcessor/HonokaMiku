/**
* EN_Decrypter.cc
* Decrypts SIF EN game files
**/

#include <stdint.h>

#include <exception>
#include <stdexcept>
#include <cstring>

#include "DecrypterContext.h"
#include "md5.h"

static const unsigned int keyTables[64] = {
	2861607190u, 3623207331u, 3775582911u, 3285432773u,
	2211141973u, 3078448744u, 464780620u, 714479011u,
	439907422u, 421011207u, 2997499268u, 630739911u,
	1488792645u, 1334839443u, 3136567329u, 796841981u,
	2604917769u, 4035806207u, 693592067u, 1142167757u,
	1158290436u, 568289681u, 3621754479u, 3645263650u,
	4125133444u, 3226430103u, 3090611485u, 1144327221u,
	879762852u, 2932733487u, 1916506591u, 2754493440u,
	1489123288u, 3555253860u, 2353824933u, 1682542640u,
	635743937u, 3455367432u, 532501229u, 4106615561u,
	2081902950u, 143042908u, 2637612210u, 1140910436u,
	3402665631u, 334620177u, 1874530657u, 863688911u,
	1651916050u, 1216533340u, 2730854202u, 1488870464u,
	2778406960u, 3973978011u, 1602100650u, 2877224961u,
	1406289939u, 1442089725u, 2196364928u, 2599396125u,
	2963448367u, 3316646782u, 322755307u, 3531653795u
};

HonokaMiku::EN_Dctx::EN_Dctx(const void* header, const char* filename)
{
	MD5 mctx;
	const char* basename = __DctxGetBasename(filename);
	uint8_t digcopy[3];

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>("BFd3EnkcKa"), 10);
	mctx.Update((unsigned char*)basename, strlen(basename));
	mctx.Final();

	memcpy(digcopy, mctx.digestRaw + 4, 3);

	digcopy[0] = ~digcopy[0];
	digcopy[1] = ~digcopy[1];
	digcopy[2] = ~digcopy[2];

	if (memcmp(digcopy, header, 3))
	{
		throw std::runtime_error(std::string("Header file doesn't match."));
	}

	printf("step 1 ok\n");

	is_finalized = false;
	version = 3;
}

void HonokaMiku::EN_Dctx::final_setup(const char* filename, const void* block_rest)
{
	// Already assumed that the first 4 bytes already processed above.
	printf("final_setup\n");
	if (!is_finalized)
	{
		const char* basename = __DctxGetBasename(filename);
		const uint8_t* second_header = reinterpret_cast<const uint8_t*>(block_rest);
		uint32_t name_sum = 844;
		uint32_t expected_sum = second_header[7] | (second_header[6] << 8);

		for (; *basename != 0; name_sum += *basename, basename++) continue;

		printf("%08x %08x\n", name_sum, expected_sum);

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

HonokaMiku::EN_Dctx* HonokaMiku::EN_Dctx::encrypt_setup(const char* filename, void* hdr_out)
{
	EN_Dctx* dctx = new EN_Dctx;
	MD5 mctx;
	const char* basename = __DctxGetBasename(filename);
	uint8_t hdr_create[16];
	uint8_t digcopy[3];
	uint16_t key_picker = 844;

	printf("encrypt_setup %s %08x\n", filename, (unsigned int)hdr_out);

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>("BFd3EnkcKa"), 10);
	mctx.Update((unsigned char*)basename, strlen(basename));
	mctx.Final();

	memcpy(digcopy, mctx.digestRaw + 4, 3);
	memset(hdr_create, 0, 16);

	digcopy[0] = ~digcopy[0];
	digcopy[1] = ~digcopy[1];
	digcopy[2] = ~digcopy[2];

	hdr_create[3] = 12;

	memcpy(hdr_create, &digcopy, 3);

	for (; *basename != 0; key_picker += *basename, basename++) {}

	hdr_create[10] = key_picker >> 8;
	hdr_create[11] = key_picker & 0xFF;

	dctx->init_key = keyTables[hdr_create[11] & 0x3F];
	dctx->update_key = dctx->init_key;
	dctx->xor_key = dctx->init_key >> 24;
	dctx->pos = 0;
	dctx->version = 3;
	dctx->is_finalized = true;

	memcpy(hdr_out, hdr_create, 16);
	return dctx;
}

void HonokaMiku::EN_Dctx::decrypt_block(void* b, uint32_t size)
{
	if (size == 0) return;

	if (is_finalized)
	{
		uint8_t* buffer = reinterpret_cast<uint8_t*>(b);

		pos += size;

		for (; size != 0; buffer++, size--)
		{
			*buffer ^= uint8_t(xor_key);
			update();
		}

		return;
	}

	throw std::runtime_error(std::string("Decrypter is not fully initialized."));
}

void HonokaMiku::EN_Dctx::decrypt_block(void* _d, const void* _s, uint32_t size)
{
	if (size == 0) return;

	if (is_finalized)
	{
		uint8_t* out_buffer = reinterpret_cast<uint8_t*>(_d);
		const uint8_t* in_buffer = reinterpret_cast<const uint8_t*>(_s);

		pos += size;

		for (; size != 0; out_buffer++, in_buffer++, size--)
		{
			*out_buffer = *in_buffer ^ uint8_t(xor_key);
			update();
		}

		return;
	}

	throw std::runtime_error(std::string("Decrypter is not fully initialized."));
}

void HonokaMiku::EN_Dctx::goto_offset(uint32_t offset)
{
	uint32_t loop_times;
	bool reset_dctx = false;

	if (!is_finalized) throw std::runtime_error(std::string("Decrypter is not fully initialized."));

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

	for (; loop_times != 0; loop_times--)
		update();

	pos = offset;
}

void HonokaMiku::EN_Dctx::goto_offset_relative(int32_t offset)
{
	if (offset == 0) return;

	int64_t x = pos + offset;
	if (x < 0) throw std::runtime_error(std::string("Position is negative."));

	goto_offset(x);
}

inline void HonokaMiku::EN_Dctx::update() {
	xor_key = (update_key = update_key * 214013 + 2531011) >> 24;
}
