/**
* DecrypterContext.h
* Base class of SIF game files decrypter
**/

//#ifndef __SIF_DECRYPTER
//#define __SIF_DECRYPTER

#include <cstdint>

struct DecrypterContext {
	uint32_t init_key;
	uint32_t update_key;
	uint16_t xor_key;
	uint32_t pos;
	virtual void decrypt_block(void* b,uint32_t len)=0;
	virtual void goto_offset(int32_t offset)=0;
protected:
	virtual void update()=0;
};

class EN_Dctx:public DecrypterContext {
protected:
	void update();
public:
	EN_Dctx();
	EN_Dctx(const char* header,const char* filename);
	void decrypt_block(void* b,uint32_t len);
	void goto_offset(int32_t offset);
};

class JP_Dctx:public DecrypterContext {
protected:
	void update();
public:
	JP_Dctx();
	JP_Dctx(const char* header,const char* filename);
	void decrypt_block(void* b,uint32_t len);
	void goto_offset(int32_t offset);
};

typedef DecrypterContext Dctx;

//#endif
