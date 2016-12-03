/**
* \file DecrypterContext.h
* Base header of SIF game files decrypter
**/

#ifndef _HONOKAMIKU_DECRYPTERCONTEXT
#define _HONOKAMIKU_DECRYPTERCONTEXT

#include <exception>
#include <stdexcept>

#include <cstring>

#include <stdint.h>

namespace HonokaMiku
{
	class V2_Dctx;
	class V3_Dctx;

	/// The decrypter context abstract class. All decrypter inherit this class.
	class DecrypterContext
	{
	public:
		/// Key used at pos 0. Used when the decrypter needs to jump to specific-position
		uint32_t init_key;
		/// Current key at `pos`
		uint32_t update_key;
		/// Variable to track current position. Needed to allow jump to specific-position
		uint32_t pos;
		/// Values to use when XOR-ing bytes
		uint32_t xor_key;
		/// Decrypter version. JP Decrypt sets this to `3` while others sets this to `2`.
		uint8_t version;
		/// \brief XOR block of memory
		/// \param buffer Buffer to be decrypted
		/// \param len Size of `buffer`
		/// \exception std::runtime_error The current decrypter context is not currently finalized (Version 3 only)
		virtual void decrypt_block(void* buffer, uint32_t len) = 0;
		/// \brief XOR block of memory and write the result to different buffer
		/// \param dest Destination buffer that will contain decrypted bytes
		/// \param src Source buffer that contains encrypted bytes
		/// \param len Size of `src`
		/// \exception std::runtime_error The current decrypter context is not currently finalized (Version 3 only)
		virtual void decrypt_block(void* dest, const void* src, uint32_t len) = 0;
		/// \brief Recalculate decrypter context to decrypt at specific position.
		/// \param offset Absolute position (starts at 0)
		/// \exception std::runtime_error The current decrypter context is not currently finalized (Version 3 only)
		virtual void goto_offset(uint32_t offset) = 0;
		/// \brief Recalculate decrypter context to decrypt at specific position.
		/// \param offset Position relative to current HonokaMiku::DecrypterContext::pos
		/// \exception std::runtime_error The current decrypter context is not currently finalized (Version 3 only)
		virtual void goto_offset_relative(int32_t offset) = 0;
		/// \brief Finalize decrypter context (Version 3 only). Does nothing in other decryption version.
		/// \param filename File name that want to be decrypted. This affects the key calculation.
		/// \param block_rest The next 12-bytes header of Version 3 encrypted file.
		virtual void final_setup(const char* filename, const void* block_rest) = 0;
	protected:
		inline DecrypterContext() {}
		/// \brief The key update function. Used to update the key. Used internally and protected
		virtual void update() = 0;
	};

	/// For encrypt_setup static members for SIF EN, TW, KR, and CN. Used internally
	void setupEncryptV2(V2_Dctx* dctx, const char* prefix, const char* filename, void* hdr_out);
	/// For encrypt_setup static members for SIF JP and EN (Version 3). Used internally
	void setupEncryptV3(V3_Dctx* dctx, const char* prefix, const unsigned int* key_tables, unsigned short name_sum_base, const char* filename, void* hdr_out);
	/// To finalize version 3 decrypter
	void finalDecryptV3(V3_Dctx* dctx, unsigned int expected_sum_name, const char* filename, const void* block_rest);

	/// Base class of Version 1 decrypter/encrypter
	class V1_Dctx: public DecrypterContext
	{
	protected:
		inline V1_Dctx() {}
		void update();
	public:
		/// \brief Initialize Version 1 decrypter context
		/// \param key_prefix String prepended before MD5 calculation
		/// \param filename File name that want to be decrypted.
		V1_Dctx(const char* key_prefix, const char* filename);
		void decrypt_block(void* buffer, uint32_t len);
		void decrypt_block(void* dest, const void* src, uint32_t len);
		void goto_offset(uint32_t offset);
		void goto_offset_relative(int32_t offset);
		inline void final_setup(const char* filename, const void* rest_block) {}
	};

	/// Base class of Version 2 decrypter
	class V2_Dctx: public DecrypterContext
	{
	protected:
		inline V2_Dctx() {}
		V2_Dctx(const char* prefix, const void* header, const char* filename);
		void update();
	public:
		void decrypt_block(void* buffer, uint32_t len);
		void decrypt_block(void* dest, const void* src, uint32_t len);
		void goto_offset(uint32_t offset);
		void goto_offset_relative(int32_t offset);
		inline void final_setup(const char* filename, const void* rest_block)
		{
			// Do nothing for Version2 in here
		}
	};

	/// Base class of Version 3 decrypter
	class V3_Dctx: public DecrypterContext
	{
	protected:
		/// Value to check if the decrypter context is already finalized
		bool is_finalized;
		/// Contains pointer to key table used for decryption
		const unsigned int* key_tables;

		V3_Dctx(const char* prefix, const unsigned int* key_tables, const void* header, const char* filename);
		inline V3_Dctx() {}

		void update();
	public:
		void decrypt_block(void* buffer, uint32_t len);
		void decrypt_block(void* dest, const void* src, uint32_t len);
		void goto_offset(uint32_t offset);
		void goto_offset_relative(int32_t offset);
		static V3_Dctx* encrypt_setup(const char* prefix, const unsigned int* key_tables, const char* filename, void* hdr_out);
		virtual void final_setup(const char* , const void* ) = 0;

		friend void setupEncryptV3(V3_Dctx* , const char* , const unsigned int* , unsigned short , const char* , void* );
		friend void finalDecryptV3(V3_Dctx* , unsigned int , const char* , const void* );
	};

	/// Japanese SIF decrypter context
	class JP3_Dctx: public V3_Dctx
	{
	protected:
		inline JP3_Dctx() {}
	public:
		/// \brief Initialize SIF JP decrypter context
		/// \param header The first 4-bytes contents of the file
		/// \param filename File name that want to be decrypted. This affects the key calculation.
		/// \exception std::runtime_error The header does not match and this decrypter context can't decrypt it.
		JP3_Dctx(const void* header, const char* filename);
		/// \brief Creates SIF JP decrypter context specialized for encryption.
		/// \param filename File name that want to be encrypted. This affects the key calculation.
		/// \param hdr_out Pointer with size of 16-bytes to store the file header.
		static JP3_Dctx* encrypt_setup(const char* filename, void* hdr_out);
		void final_setup(const char* filename, const void* block_rest);
	};

	/// International SIF decrypter context (Version 3)
	class EN3_Dctx: public V3_Dctx
	{
	protected:
		inline EN3_Dctx() {}
	public:
		/// \brief Initialize SIF EN decrypter context (version 3)
		/// \param header The first 4-bytes contents of the file
		/// \param filename File name that want to be decrypted. This affects the key calculation.
		/// \exception std::runtime_error The header does not match and this decrypter context can't decrypt it.
		EN3_Dctx(const void* header, const char* filename);
		/// \brief Creates SIF EN decrypter context specialized for encryption. (version 3)
		/// \param filename File name that want to be encrypted. This affects the key calculation.
		/// \param hdr_out Pointer with size of 16-bytes to store the file header.
		static EN3_Dctx* encrypt_setup(const char* filename, void* hdr_out);
		void final_setup(const char* filename, const void* block_rest);
	};

	/// Taiwanese SIF decrypter context (Version 3)
	class TW3_Dctx: public V3_Dctx
	{
	protected:
		inline TW3_Dctx() {}
	public:
		/// \brief Initialize SIF TW decrypter context (version 3)
		/// \param header The first 4-bytes contents of the file
		/// \param filename File name that want to be decrypted. This affects the key calculation.
		/// \exception std::runtime_error The header does not match and this decrypter context can't decrypt it.
		TW3_Dctx(const void* header, const char* filename);
		/// \brief Creates SIF TW decrypter context specialized for encryption. (version 3)
		/// \param filename File name that want to be encrypted. This affects the key calculation.
		/// \param hdr_out Pointer with size of 16-bytes to store the file header.
		static TW3_Dctx* encrypt_setup(const char* filename, void* hdr_out);
		void final_setup(const char* filename, const void* block_rest);
	};

	/// Chinese SIF decrypter context (Version 3)
	class CN3_Dctx: public V3_Dctx
	{
	protected:
		inline CN3_Dctx() {}
	public:
		/// \brief Initialize SIF CN decrypter context (version 3)
		/// \param header The first 4-bytes contents of the file
		/// \param filename File name that want to be decrypted. This affects the key calculation.
		/// \exception std::runtime_error The header does not match and this decrypter context can't decrypt it.
		CN3_Dctx(const void* header, const char* filename);
		/// \brief Creates SIF CN decrypter context specialized for encryption. (version 3)
		/// \param filename File name that want to be encrypted. This affects the key calculation.
		/// \param hdr_out Pointer with size of 16-bytes to store the file header.
		static CN3_Dctx* encrypt_setup(const char* filename, void* hdr_out);
		void final_setup(const char* filename, const void* block_rest);
	};

	/// International SIF decrypter context
	class EN2_Dctx:public V2_Dctx
	{
	protected:
		EN2_Dctx():V2_Dctx() {}
	public:
		/// \brief Initialize SIF EN decrypter context
		/// \param header The first 4-bytes contents of the file
		/// \param filename File name that want to be decrypted. This affects the key calculation.
		/// \exception std::runtime_error The header does not match and this decrypter context can't decrypt it.
		EN2_Dctx(const void* header, const char* filename):V2_Dctx("BFd3EnkcKa", header, filename) {}
		/// \brief Creates SIF EN decrypter context specialized for encryption.
		/// \param filename File name that want to be encrypted. This affects the key calculation.
		/// \param hdr_out Pointer with size of 16-bytes to store the file header.
		inline static EN2_Dctx* encrypt_setup(const char* filename, void* hdr_out)
		{
			EN2_Dctx* dctx = new EN2_Dctx();
			setupEncryptV2(dctx, "BFd3EnkcKa", filename, hdr_out);
			return dctx;
		}
	};

	/// Taiwanese SIF decrypter context
	class TW2_Dctx:public V2_Dctx
	{
	protected:
		TW2_Dctx():V2_Dctx() {}
	public:
		/// \brief Initialize SIF TW decrypter context
		/// \param header The first 4-bytes contents of the file
		/// \param filename File name that want to be decrypted. This affects the key calculation.
		/// \exception std::runtime_error The header does not match and this decrypter context can't decrypt it.
		TW2_Dctx(const void* header, const char* filename):V2_Dctx("M2o2B7i3M6o6N88", header, filename) {}
		/// \brief Creates SIF TW decrypter context specialized for encryption.
		/// \param filename File name that want to be encrypted. This affects the key calculation.
		/// \param hdr_out Pointer with size of 16-bytes to store the file header.
		inline static TW2_Dctx* encrypt_setup(const char* filename, void* hdr_out)
		{
			TW2_Dctx* dctx = new TW2_Dctx();
			setupEncryptV2(dctx,"M2o2B7i3M6o6N88", filename, hdr_out);
			return dctx;
		}
	};

	/// SIF JP decrypter version 2
	class JP2_Dctx: public V2_Dctx
	{
	protected:
		JP2_Dctx():V2_Dctx() {}
	public:
		/// \brief Initialize SIF JP decrypter context (version 2)
		/// \param header The first 4-bytes contents of the file
		/// \param filename File name that want to be decrypted. This affects the key calculation.
		/// \exception std::runtime_error The header does not match and this decrypter context can't decrypt it.
		JP2_Dctx(const void* header, const char* filename):V2_Dctx("Hello", header, filename) {}
		/// \brief Creates SIF JP decrypter context specialized for encryption. (Version 2)
		/// \param filename File name that want to be encrypted. This affects the key calculation.
		/// \param hdr_out Pointer with size of 16-bytes to store the file header.
		inline static JP2_Dctx* encrypt_setup(const char* filename, void* hdr_out)
		{
			JP2_Dctx* dctx = new JP2_Dctx();
			setupEncryptV2(dctx, "Hello", filename, hdr_out);
			return dctx;
		}
	};

	/// Simplified Chinese SIF decrypter context.
	/// It has longest prefix key currently
	class CN2_Dctx: public V2_Dctx
	{
	protected:
		CN2_Dctx():V2_Dctx() {}
	public:
		/// \brief Initialize SIF CN decrypter context
		/// \param header The first 4-bytes contents of the file
		/// \param filename File name that want to be decrypted. This affects the key calculation.
		/// \exception std::runtime_error The header does not match and this decrypter context can't decrypt it.
		CN2_Dctx(const void* header, const char* filename):V2_Dctx("iLbs0LpvJrXm3zjdhAr4", header, filename) {}
		/// \brief Creates SIF CN decrypter context specialized for encryption.
		/// \param filename File name that want to be encrypted. This affects the key calculation.
		/// \param hdr_out Pointer with size of 16-bytes to store the file header.
		inline static CN2_Dctx* encrypt_setup(const char* filename, void* hdr_out)
		{
			CN2_Dctx* dctx=new CN2_Dctx();
			setupEncryptV2(dctx, "iLbs0LpvJrXm3zjdhAr4", filename,hdr_out);
			return dctx;
		}
	};
	
	/// Alias of DecrypterContext
	typedef DecrypterContext Dctx;
	/// Alias of EN2_Dctx
	typedef EN2_Dctx WW_Dctx;
	/// Alias of EN3_Dctx
	typedef EN3_Dctx WW3_Dctx;
	/// Alias of JP2_Dctx
	typedef JP2_Dctx KR_Dctx;

	// Helper functions.

	/// \brief Creates decrypter context based from the given headers. Auto detect
	/// \param filename File name that want to be decrypted. This affects the key calculation.
	/// \param header The first 4-bytes contents of the file
	/// \param game_type Pointer to store the game ID. See GetPrefixFromGameId() for valid game IDs.
	/// \returns DecrypterContext or NULL if no suitable decryption method is available.
	DecrypterContext* FindSuitable(const char* filename, const void* header, char* game_type = NULL);
	
	/// \brief Creates decrypter context based the game ID.
	/// \param game_id The game ID. Valid game IDs can be seen in HonokaMiku.cc Line 86
	/// \param header The first 4-bytes contents of the file
	/// \param filename File name that want to be decrypted. This affects the key calculation.
	/// \returns DecrypterContext or NULL if specificed game ID is invalid
	/// \exception std::runtime_error Thrown if header is not valid for decryption
	DecrypterContext* CreateFrom(char game_id, const void* header, const char* filename);
	
	/// \brief Creates decrypter context for encryption based the game ID.
	/// \param game_id The game ID. See GetPrefixFromGameId() for valid game IDs.
	/// \param filename File name that want to be decrypted. This affects the key calculation.
	/// \param header_out Pointer to store the file header. The memory size should be 16-bytes
	///                   to reserve space for Version 3 decrypter.
	/// \returns DecrypterContext ready for encryption
	DecrypterContext* EncryptPrepare(char game_id, const char* filename, void* header_out);

	/// \brief Gets game key prefix for specificed game id.
	/// \param game_id The game ID. Valid game IDs are
	///                   1. EN/WW Version 2
	///                   2. JP Version 2/KR
	///                   3. TW Version 2
	///                   4. JP Version 3
	///                   5. CN Version 2
	///                   6. EN/WW Version 3
	///                   7. CN Version 3
	///                   8. TW Version 3
	/// \returns prefix key of specificed game ID or NULL if game ID is invalid
	/// \note If you just want to get prefix key, the version doesn't matter.
	inline const char* GetPrefixFromGameId(char game_id)
	{
		switch(game_id)
		{
			case 1:
			case 6: return "BFd3EnkcKa";
			case 2:
			case 4: return "Hello";
			case 3:
			case 8: return "M2o2B7i3M6o6N88";
			case 5:
			case 7: return "iLbs0LpvJrXm3zjdhAr4";
			default: return NULL;
		}
	}
}

// Inline function to retrieve the basename
inline const char* __DctxGetBasename(const char* filename)
{
	const char* basename = filename + strlen(filename);

	for(; *basename != '/' && *basename != '\\' && basename != filename; basename--) {}

	if(basename != filename) return basename + 1;
	else return filename;
}

#endif
