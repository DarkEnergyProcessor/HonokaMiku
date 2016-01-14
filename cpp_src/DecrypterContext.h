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
};

class EN_Dctx:public DecrypterContext {
protected:
	EN_Dctx() {}
	void update();
public:
	EN_Dctx(const char* header,const char* filename);
	void decrypt_block(void* b,uint32_t len);
	void goto_offset(int32_t offset);
	static EN_Dctx* encrypt_setup(const char* filename,void* hdr_out);
};

class JP_Dctx:public DecrypterContext {
protected:
	JP_Dctx() {}
	void update();
public:
	JP_Dctx(const char* header,const char* filename);
	void decrypt_block(void* b,uint32_t len);
	void goto_offset(int32_t offset);
	static JP_Dctx* encrypt_setup(const char* filename,void* hdr_out);
};

// SIF TW Decryption method shares with SIF WW decryption method but with different constructor
class TW_Dctx:public EN_Dctx
{
protected:
	TW_Dctx():EN_Dctx() {}
public:
	TW_Dctx(const char* header,const char* filename);
	static TW_Dctx* encrypt_setup(const char* filename,void* hdr_out);
};

// Also SIF KR Decryption method shares with SIF WW decryption.
// This also can be used to encrypt SIF JP game files but it's untested.
class KR_Dctx:public EN_Dctx
{
protected:
	KR_Dctx():EN_Dctx() {}
public:
	KR_Dctx(const char* header,const char* filename);
	static KR_Dctx* encrypt_setup(const char* filename,void* hdr_out);
};

// I think SIF CN decryption uses Version 1 but actually it uses Version 2.
class CN_Dctx:public EN_Dctx
{
protected:
	CN_Dctx():EN_Dctx() {}
public:
	CN_Dctx(const char* header,const char* filename);
	static CN_Dctx* encrypt_setup(const char* filename,void* hdr_out);
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

//#endif
