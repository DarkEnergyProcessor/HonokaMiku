/**
* DecrypterContext.h
* Base class of SIF game files decrypter
**/

//#ifndef __SIF_DECRYPTER
//#define __SIF_DECRYPTER

struct DecrypterContext {
	unsigned int init_key;
	unsigned int update_key;
	unsigned short xor_key;
	unsigned int pos;
	virtual void decrypt_block(void* b,unsigned int len)=0;
	virtual void goto_offset(long offset)=0;
protected:
	virtual void update()=0;
	DecrypterContext() {};
	~DecrypterContext() {};
};

class EN_Dctx:public DecrypterContext {
protected:
	void update();
public:
	EN_Dctx();
	EN_Dctx(const char* header,const char* filename);
	void decrypt_block(void* b,unsigned int len);
	void goto_offset(long offset);
};

class JP_Dctx:public DecrypterContext {
protected:
	void update();
public:
	JP_Dctx();
	JP_Dctx(const char* header,const char* filename);
	void decrypt_block(void* b,unsigned int len);
	void goto_offset(long offset);
};

typedef DecrypterContext Dctx;

//#endif
