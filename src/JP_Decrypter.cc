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

JP_Dctx::JP_Dctx(const void* _hdr,const char* filename)
{
	MD5 mctx;
	const uint8_t* header=reinterpret_cast<const uint8_t*>(_hdr);
	const char* basename=__DctxGetBasename(filename);
	size_t base_len=strlen(basename);
	uint32_t digcopy=0;
	uint16_t name_sum=500;

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>("Hello"),5);
	mctx.Update((unsigned char*)basename,base_len);
	mctx.Final();
	memcpy(&digcopy,mctx.digestRaw+4,3);
	digcopy=(~digcopy)&0xffffff;
	for(uint32_t i=0;i<base_len;i++)
		name_sum+=basename[i];

	if(memcmp(&digcopy,header,3) || ((uint16_t(header[10])<<8)|header[11])!=name_sum)
		throw std::runtime_error(std::string("Header file doesn't match."));

	init_key=keyTables[header[11]&63];
	update_key=init_key;
	xor_key=init_key>>24;
	pos=0;
}

JP_Dctx* JP_Dctx::encrypt_setup(const char* filename,void* hdr_out)
{
	JP_Dctx* dctx=new JP_Dctx;
	MD5 mctx;
	const char* basename=__DctxGetBasename(filename);
	uint8_t hdr_create[16];
	uint32_t digcopy=0;
	uint16_t key_picker=500;

	mctx.Init();
	mctx.Update(reinterpret_cast<const uint8_t*>("Hello"),5);
	mctx.Update((unsigned char*)basename,strlen(basename));
	mctx.Final();
	memcpy(&digcopy,mctx.digestRaw+4,3);
	memset(hdr_create,0,16);
	digcopy=(~digcopy)&0xffffff;

	hdr_create[3]=12;
	memcpy(hdr_create,&digcopy,3);
	for(size_t i=0;basename[i]!=0;i++)
		key_picker+=basename[i];

	hdr_create[10]=key_picker>>8;
	hdr_create[11]=key_picker&255;

	dctx->init_key=keyTables[hdr_create[11]&63];
	dctx->update_key=dctx->init_key;
	dctx->xor_key=dctx->init_key>>24;
	dctx->pos=0;

	memcpy(hdr_out,hdr_create,16);
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
	pos+=size;
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
