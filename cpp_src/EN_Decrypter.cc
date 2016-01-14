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

EN_Dctx::EN_Dctx(const char* header,const char* filename)
{
	MD5_CTX* mctx;
	const char* basename=__DctxGetBasename(filename);

	mctx=new MD5_CTX;
	MD5Init(mctx);
	MD5Update(mctx,(unsigned char*)"BFd3EnkcKa",10);
	MD5Update(mctx,(unsigned char*)basename,strlen(basename));
	MD5Final(mctx);

	if(memcmp(header,mctx->digest+4,4))
	{
		delete mctx;
		throw std::runtime_error(std::string("Header file doesn't match."));
	}

	memcpy(&init_key,mctx->digest,4);
#ifdef _MSC_VER
	init_key=_byteswap_ulong(init_key)&2147483647;
#elif defined(__GNUG__)
	init_key=__builtin_bswap32(init_key)&2147483647;
#else
	init_key=htonl(init_key)&2147483647;
#endif
	update_key=init_key;
	xor_key=(init_key>>23)&255|(init_key>>7)&65280;
	pos=0;

	delete mctx;
}

EN_Dctx* EN_Dctx::encrypt_setup(const char* filename,void* hdr_out)
{
	EN_Dctx* dctx=new EN_Dctx;
	const char* basename=__DctxGetBasename(filename);
	MD5_CTX* mctx=new MD5_CTX;
	
	MD5Init(mctx);
	MD5Update(mctx,(unsigned char*)"BFd3EnkcKa",10);
	MD5Update(mctx,(unsigned char*)basename,strlen(basename));
	MD5Final(mctx);
	
	memcpy(&dctx->init_key,mctx->digest,4);
	memcpy(hdr_out,mctx->digest+4,4);
#ifdef _MSC_VER
	dctx->init_key=_byteswap_ulong(dctx->init_key)&2147483647;
#elif defined(__GNUG__)
	dctx->init_key=__builtin_bswap32(dctx->init_key)&2147483647;
#else
	dctx->init_key=htonl(dctx->init_key)&2147483647;
#endif
	dctx->update_key=dctx->init_key;
	dctx->xor_key=(dctx->init_key>>23)&255|(dctx->init_key>>7)&65280;
	dctx->pos=0;

	delete mctx;
	return dctx;
}

void EN_Dctx::decrypt_block(void* b,uint32_t size)
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

	for(int i=0;i<size;i++)
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

void EN_Dctx::goto_offset(int32_t offset)
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

inline void EN_Dctx::update()
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
}
