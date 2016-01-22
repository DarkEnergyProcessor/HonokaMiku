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

static const unsigned int keyTables[64]={
1210253353	,1736710334	,1030507233	,1924017366,
1603299666	,1844516425	,1102797553	,32188137,
782633907	,356258523	,957120135	,10030910,
811467044	,1226589197	,1303858438	,1423840583,
756169139	,1304954701	,1723556931	,648430219,
1560506399	,1987934810	,305677577	,505363237,
450129501	,1811702731	,2146795414	,842747461,
638394899	,51014537	,198914076	,120739502,
1973027104	,586031952	,1484278592	,1560111926,
441007634	,1006001970	,2038250142	,232546121,
827280557	,1307729428	,775964996	,483398502,
1724135019	,2125939248	,742088754	,1411519905,
136462070	,1084053905	,2039157473	,1943671327,
650795184	,151139993	,1467120569	,1883837341,
1249929516	,382015614	,1020618905	,1082135529,
870997426	,1221338057	,1623152467	,1020681319
};

#if defined(__GNUC__) && ((__GNUC__==4 && __GNUC_MINOR__<8) || __GNUC__<4)
static inline unsigned short __builtin_bswap16(unsigned short a)
{
	return (a<<8)|(a>>8);
}
#endif

JP_Dctx::JP_Dctx(const char* header,const char* filename)
{
	MD5_CTX* mctx;
	const char* basename=__DctxGetBasename(filename);
	unsigned int digcopy=0;

	mctx=new MD5_CTX;
	MD5Init(mctx);
	MD5Update(mctx,(unsigned char*)"Hello",5);
	MD5Update(mctx,(unsigned char*)basename,strlen(basename));
	MD5Final(mctx);
	memcpy(&digcopy,mctx->digest+4,3);
	digcopy=(~digcopy)&0xffffff;

	if(memcmp(&digcopy,header,3))
	{
		delete mctx;
		throw std::runtime_error(std::string("Header file doesn't match."));
	}

	init_key=keyTables[header[11]&63];
	update_key=init_key;
	xor_key=init_key>>24;
	pos=0;

	delete mctx;
}

JP_Dctx* JP_Dctx::encrypt_setup(const char* filename,void* hdr_out)
{
	JP_Dctx* dctx=new JP_Dctx;
	const char* basename=__DctxGetBasename(filename);
	MD5_CTX* mctx=new MD5_CTX;
	unsigned int digcopy=0;
	char hdr_create[16];

	MD5Init(mctx);
	MD5Update(mctx,(unsigned char*)"Hello",5);
	MD5Update(mctx,(unsigned char*)basename,strlen(basename));
	MD5Final(mctx);
	memcpy(&digcopy,mctx->digest+4,3);
	digcopy=(~digcopy)&0xffffff;
	
	memset(hdr_create,0,16);

	// Create Version3 header
	{
		unsigned short key_picker=500;
		hdr_create[3]=12;
		memcpy(hdr_create,&digcopy,3);
		for(size_t i=0;basename[i]!=0;i++)
			key_picker+=basename[i];

#ifdef _MSC_VER
		key_picker=_byteswap_ushort(key_picker);
#elif defined(__GNUG__)
		key_picker=__builtin_bswap16(key_picker);
#else
		key_picker=htons(key_picker);
#endif
		memcpy(hdr_create+10,&key_picker,2);
	}
	dctx->init_key=keyTables[hdr_create[11]&63];
	dctx->update_key=dctx->init_key;
	dctx->xor_key=dctx->init_key>>24;
	dctx->pos=0;

	memcpy(hdr_out,hdr_create,16);
	delete mctx;
	return dctx;
}

void JP_Dctx::decrypt_block(void* b,uint32_t size)
{
	if(size==0) return;

	char* buffer=(char*)b;
	for(unsigned int i=0;i<size;i++)
	{
		buffer[i]^=char(xor_key);
		update();
	}
}

void JP_Dctx::goto_offset(int32_t offset)
{
	if(offset==0) return;

	int x=pos+offset;
	if(x<0) throw std::runtime_error(std::string("Position is negative."));

	update_key=init_key;
	xor_key=init_key>>24;
	
	if(x>0)
		for(int i=0;i<x;i++)
			update();
	pos=x;
}

inline void JP_Dctx::update() {
	xor_key=(update_key=update_key*214013+2531011)>>24;
}
