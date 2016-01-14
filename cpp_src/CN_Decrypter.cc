/**
* CN_Decrypter.cc
* Decrypts SIF CN game files
**/

#include <stdint.h>

#include <exception>
#include <stdexcept>
#include <cstring>

#include "DecrypterContext.h"
#include "md5.h"

CN_Dctx::CN_Dctx(const char* header,const char* filename):EN_Dctx()
{
	MD5_CTX* mctx;
	const char* basename=__DctxGetBasename(filename);

	mctx=new MD5_CTX;
	MD5Init(mctx);
	MD5Update(mctx,(unsigned char*)"iLbs0LpvJrXm3zjdhAr4",20);
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

CN_Dctx* CN_Dctx::encrypt_setup(const char* filename,void* hdr_out)
{
	CN_Dctx* dctx=new CN_Dctx;
	const char* basename=__DctxGetBasename(filename);
	MD5_CTX* mctx=new MD5_CTX;
	
	MD5Init(mctx);
	MD5Update(mctx,(unsigned char*)"iLbs0LpvJrXm3zjdhAr4",20);
	MD5Update(mctx,(unsigned char*)basename,strlen(basename));
	MD5Final(mctx);
	
	memcpy(&dctx->init_key,mctx->digest,4);
	memcpy(hdr_out,mctx->digest,4);
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