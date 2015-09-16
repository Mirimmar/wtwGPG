#pragma once

#include <map>
#include <string>
#include <gpgme.h>

namespace wtwGPG
{
	struct GPGWrapperError : public std::runtime_error
	{
		GPGWrapperError(const wchar_t* msg) : runtime_error("Error in GPG"), errorMessage(msg) 
		{}

		const wchar_t* errorMessage;
	}; // struct GPGWrapperError

	struct GPGWrapperDecryptedData
	{
		GPGWrapperDecryptedData() : isSignValid(false)
		{}

		std::string data;
		bool isSignValid;
	};

	class GPGWrapper
	{
	public:
		GPGWrapper(const char* privateKeyId);
		~GPGWrapper();

		std::string encrypt(const std::string& recipientName, const std::string& message);
		GPGWrapperDecryptedData decrypt(const std::string& message);

	private:
		bool initialized;
		gpgme_ctx_t context;
		gpgme_key_t privateKey;
		std::string privateKeyId;

		typedef std::map<std::string, gpgme_key_t> IdToGPGKey;
		IdToGPGKey reciversIdToKey;

		std::string copyData(gpgme_data_t data);

		void init();
		void deinit();

		gpgme_key_t getPrivateKey(const char* identifier = NULL);
		gpgme_key_t getPublicKey(const char* identifier = NULL);
		gpgme_key_t getKey(const char* identifier, bool isPrivate);
	}; // class GPGWrapper

} // namespace wtwGPG