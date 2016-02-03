/**
* V2_Decrypter.cc
* Base class of Version2 decrypter
* Used in all SIF game files except SIF JP which has it's own class
**/

#include <stdint.h>

#include <exception>
#include <stdexcept>
#include <cstring>

// Byte-order function. Copied from Itsudemo
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

#include "DecrypterContext.h"
#include "md5.h"

V2_Dctx::V2_Dctx(const char* prefix,const char* header,const char* filename)
{
	MD5 mctx;
	const char* basename=__DctxGetBasename(filename);

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>(prefix),strlen(prefix));
	mctx.Update(reinterpret_cast<const uint8_t*>(basename),strlen(basename));
	mctx.Final();

	if(memcmp(header,mctx.digestRaw+4,4))
		throw std::runtime_error(std::string("Header file doesn't match."));

	memcpy(&init_key,mctx.digestRaw,4);
	init_key=htonl(init_key)&2147483647;
	update_key=init_key;
	xor_key=(init_key>>23)&255|(init_key>>7)&65280;
	pos=0;
}

void V2_Dctx::decrypt_block(void* b,uint32_t size)
{
	if(size==0) return;

	char* buffer=(char*)b;
	if(pos%2==1)
	{
		buffer[0]^=char(xor_key>>8);
		update();
		pos++;
		buffer++;
	}

	for(unsigned int i=0;i<size;i++)
	{
		if(i%2==0)
			buffer[i]^=char(xor_key);
		else
		{
			buffer[i]^=char(xor_key>>8);
			update();
		}
		pos++;
	}
}

void V2_Dctx::goto_offset(int32_t offset)
{
	if(offset==0) return;

	int64_t x=pos+offset;
	if(x<0) throw std::runtime_error(std::string("Position is negative."));

	update_key=init_key;
	xor_key=(init_key>>23)&255|(init_key>>7)&65280;

	if(x>1)
		for(int i=1;i<=x/2;i++)
			update();
	pos=x;
}

/*
// Old inline update function.
inline void V2_Dctx::update()
{
	uint32_t a,b,c,d,e,f;

	a=update_key;
	b=a>>16;
	c=(b*1101463552)&2147483647;
	d=c+(a&65535)*16807;
	e=(b*16807)>>15;
	f=e+d-2147483647;
	if(d>2147483646)
		d=f;
	else
		d+=e;

	update_key=d;
	xor_key=(d>>23)&255|(d>>7)&65280;
}*/

inline void V2_Dctx::update()
{
	uint32_t a,b,c,d;

	a=update_key>>16;
	b=((a*1101463552)&2147483647)+(update_key&65535)*16807;
	c=(a*16807)>>15;
	d=c+b-2147483647;
	b=b>2147483646?d:b+c;
	update_key=b;
	xor_key=(b>>23)&255|(b>>7)&65280;
}

void setupEncryptV2(DecrypterContext* dctx,const char* prefix,const char* filename,void* hdr_out)
{
	const char* basename=__DctxGetBasename(filename);
	MD5 mctx;
	
	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>(prefix),strlen(prefix));
	mctx.Update(reinterpret_cast<const uint8_t*>(basename),strlen(basename));
	mctx.Final();
	
	memcpy(&dctx->init_key,mctx.digestRaw,4);
	memcpy(hdr_out,mctx.digestRaw+4,4);
	dctx->init_key=htonl(dctx->init_key)&2147483647;
	dctx->update_key=dctx->init_key;
	dctx->xor_key=(dctx->init_key>>23)&255|(dctx->init_key>>7)&65280;
	dctx->pos=0;
}
