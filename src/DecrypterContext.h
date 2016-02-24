/**
* DecrypterContext.h
* Base class of SIF game files decrypter
**/

//#ifndef __SIF_DECRYPTER
//#define __SIF_DECRYPTER

#include <stdint.h>
#include <cstring>

struct DecrypterContext {
	uint32_t init_key;
	uint32_t update_key;
	uint16_t xor_key;
	uint32_t pos;
	virtual void decrypt_block(void* b,uint32_t len)=0;
	virtual void goto_offset(int32_t offset)=0;
protected:
	DecrypterContext() {}
	virtual void update()=0;
};

// For encrypt_setup static members for SIF EN,TW,KR,CN
void setupEncryptV2(DecrypterContext* dctx,const char* prefix,const char* filename,void* hdr_out);

// SIF JP uses version 3 decrypter
class JP_Dctx:public DecrypterContext {
protected:
	JP_Dctx() {}
	void update();
public:
	JP_Dctx(const void* header,const char* filename);
	void decrypt_block(void* b,uint32_t len);
	void goto_offset(int32_t offset);
	static JP_Dctx* encrypt_setup(const char* filename,void* hdr_out);
};

// Base class of version 2 decrypter
class V2_Dctx:public DecrypterContext {
protected:
	V2_Dctx() {}
	V2_Dctx(const char* prefix,const void* header,const char* filename);
	void update();
public:
	void decrypt_block(void* b,uint32_t len);
	void goto_offset(int32_t offset);
};

class EN_Dctx:public V2_Dctx {
protected:
	EN_Dctx():V2_Dctx() {}
public:
	EN_Dctx(const void* header,const char* filename):V2_Dctx("BFd3EnkcKa",header,filename) {}
	inline static EN_Dctx* encrypt_setup(const char* filename,void* hdr_out)
	{
		EN_Dctx* dctx=new EN_Dctx();
		setupEncryptV2(dctx,"BFd3EnkcKa",filename,hdr_out);
		return dctx;
	}
};

// SIF TW Decryption method shares with SIF WW decryption method but with different constructor
class TW_Dctx:public V2_Dctx
{
protected:
	TW_Dctx():V2_Dctx() {}
public:
	TW_Dctx(const void* header,const char* filename):V2_Dctx("M2o2B7i3M6o6N88",header,filename) {}
	inline static TW_Dctx* encrypt_setup(const char* filename,void* hdr_out)
	{
		TW_Dctx* dctx=new TW_Dctx();
		setupEncryptV2(dctx,"M2o2B7i3M6o6N88",filename,hdr_out);
		return dctx;
	}
};

// Also SIF KR Decryption method shares with SIF WW decryption.
// This also can be used to encrypt SIF JP game files but it's untested.
class KR_Dctx:public V2_Dctx
{
protected:
	KR_Dctx():V2_Dctx() {}
public:
	KR_Dctx(const void* header,const char* filename):V2_Dctx("Hello",header,filename) {}
	inline static KR_Dctx* encrypt_setup(const char* filename,void* hdr_out)
	{
		KR_Dctx* dctx=new KR_Dctx();
		setupEncryptV2(dctx,"Hello",filename,hdr_out);
		return dctx;
	}
};

// I think SIF CN decryption uses Version 1 but actually it uses Version 2.
class CN_Dctx:public V2_Dctx
{
protected:
	CN_Dctx():V2_Dctx() {}
public:
	CN_Dctx(const void* header,const char* filename):V2_Dctx("iLbs0LpvJrXm3zjdhAr4",header,filename) {}
	inline static CN_Dctx* encrypt_setup(const char* filename,void* hdr_out)
	{
		CN_Dctx* dctx=new CN_Dctx();
		setupEncryptV2(dctx,"iLbs0LpvJrXm3zjdhAr4",filename,hdr_out);
		return dctx;
	}
};

// Inline function to retrieve the basename
inline const char* __DctxGetBasename(const char* filename)
{
	const char* basename;
	const char* basename2;

	basename=strrchr(filename,'/');
	basename2=strrchr(filename,'\\');
	basename=basename==basename2?filename:((basename>basename2?basename:basename2)+1);

	return basename;
}

typedef DecrypterContext Dctx;
typedef EN_Dctx WW_Dctx;

//#endif
