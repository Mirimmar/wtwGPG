#include "stdafx.h"
#include "GPGWrapper.h"

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <locale.h>
#include <stdlib.h>
//#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "..\common\StringUtils.h"

#define fail_if_err(err, msg) \
	do \
	{ \
		if (err) \
		{ \
			throw wtwGPG::GPGWrapperError(msg); \
		} \
	} \
	while (0)

namespace
{	
	class ScopedGPGData
	{
	public:

		ScopedGPGData() : initialized(false)
		{
		}

		~ScopedGPGData()
		{
			if (initialized)
				gpgme_data_release(data);
		}

		gpgme_error_t init()
		{
			initialized = true;
			return gpgme_data_new(&data);
		}

		gpgme_error_t init(const char* msg, size_t size)
		{
			initialized = true;
			return gpgme_data_new_from_mem(&data, msg, size, 0);
		}

		gpgme_data_t get()
		{
			return data;
		}

	private:

		gpgme_data_t data;
		bool initialized;

	}; // class ScopedGPGData

} // namespace [unnamed]

namespace wtwGPG
{

	GPGWrapper::GPGWrapper(const char* privateKeyId) : initialized(false), privateKeyId(privateKeyId)
	{
	}

	GPGWrapper::~GPGWrapper()
	{
		deinit();
	}

	std::string GPGWrapper::encrypt(const std::string & recipientName, const std::string& message)
	{
		init();

		ScopedGPGData clear_text, sign_text, encrypted_text;

		gpgme_error_t error = clear_text.init(message.c_str(), message.length());
		fail_if_err(error, L"Nie uda³o siê zainicjowaæ danych do zaszyfrowana (clear_text).");
		error = sign_text.init();
		fail_if_err(error, L"Nie uda³o siê zainicjowaæ danych do zaszyfrowana (sign_text).");

		error = gpgme_op_sign(context, clear_text.get(), sign_text.get(), GPGME_SIG_MODE_NORMAL);
		fail_if_err(error, L"Nie uda³o siê podpisaæ wiadomoœci.");

		error = gpgme_data_rewind(sign_text.get());
		fail_if_err(error, L"Nie uda³o siê przewin¹æ na pocz¹tek podpisanego strumienia, aby go póŸniej zaszyfrowaæ.");

		error = encrypted_text.init();
		fail_if_err(error, L"Nie uda³o siê zainicjowaæ danych do zaszyfrowana (encrypted_text).");

		gpgme_key_t recipient = getPublicKey(recipientName.c_str());

		gpgme_key_t recipients[2] = { NULL, NULL };
		recipients[0] = recipient;
		error = gpgme_op_encrypt(context, recipients, GPGME_ENCRYPT_ALWAYS_TRUST, sign_text.get(), encrypted_text.get());
		fail_if_err(error, L"Nie uda³o siê zaszyfrowaæ podpisanej wiadomoœci.");

		gpgme_encrypt_result_t result = gpgme_op_encrypt_result(context);
		fail_if_err(result->invalid_recipients, L"Nie poprawny klucz szyfrowania odbiorcy.");

		return copyData(encrypted_text.get());
	}

	GPGWrapperDecryptedData GPGWrapper::decrypt(const std::string& message)
	{
		init();

		ScopedGPGData clear_text, sign_text, encrypted_text;

		gpgme_error_t error = encrypted_text.init(message.c_str(), message.length());
		fail_if_err(error, L"Nie uda³o siê zainicjowaæ danych do odszyfrowania (encrypted_text).");
		error = sign_text.init();
		fail_if_err(error, L"Nie uda³o siê zainicjowaæ danych do odszyfrowania (sign_text).");

		error = gpgme_op_decrypt(context, encrypted_text.get(), sign_text.get());
		fail_if_err(error, L"Nie uda³o siê odszyfrowaæ wiadomoœci.");

		error = gpgme_data_rewind(sign_text.get());
		fail_if_err(error, L"Nie uda³o siê przewin¹æ na pocz¹tek podpisanego strumienia, aby go póŸniej zweryfikowaæ.");

		error = clear_text.init();
		fail_if_err(error, L"Nie uda³o siê zainicjowaæ danych do odszyfrowania (clear_text).");

		error = gpgme_op_verify(context, sign_text.get(), NULL, clear_text.get());
		fail_if_err(error, L"Nie powiod³a siê próba potwierdzenia podpisu nadawcy.");

		GPGWrapperDecryptedData result;

		gpgme_verify_result_t verifyResult = gpgme_op_verify_result(context);
		if (verifyResult != NULL && verifyResult->signatures != NULL && verifyResult->signatures->summary & GPGME_SIGSUM_VALID)
			result.isSignValid = true;

		result.data = copyData(clear_text.get());

		return result;
	}

	std::string GPGWrapper::copyData(gpgme_data_t data)
	{
		gpgme_error_t error = gpgme_data_rewind(data);
		fail_if_err(error, L"Nie uda³o siê przewin¹æ na pocz¹tek strumienia, aby go póŸniej odczytaæ.");

		const int bufferSize = 4096;
		char buffer[bufferSize + 1];

		gpgme_ssize_t nbytes;
		std::string dataCopy;

		while ((nbytes = gpgme_data_read(data, buffer, bufferSize)) > 0)
		{
			size_t actualLength = dataCopy.length();
			dataCopy.resize(dataCopy.length() + nbytes);
			memcpy(&dataCopy[actualLength], buffer, nbytes);
		}

		if (nbytes == -1)
			throw GPGWrapperError(L"B³¹d podczas kopiowania danyh ze strumienia.");

		return dataCopy;
	}

	void GPGWrapper::init()
	{
		if (initialized)
			return;

		initialized = true;

		gpgme_check_version(NULL);
		setlocale(LC_ALL, "");
		gpgme_set_locale(NULL, LC_CTYPE, setlocale(LC_CTYPE, NULL));
#ifdef LC_MESSAGES
		gpgme_set_locale(NULL, LC_MESSAGES, setlocale(LC_MESSAGES, NULL));
#endif

		gpgme_error_t error = gpgme_new(&context);
		fail_if_err(error, L"Nie uda³o siê zainicjowaæ kontekstu GPG.");

		gpgme_set_textmode(context, 1);
		gpgme_set_armor(context, 1);

		error = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP);
		fail_if_err(error, L"Nie zainstalowano OpenPGP.");

		error = gpgme_set_protocol(context, GPGME_PROTOCOL_OpenPGP);
		fail_if_err(error, L"Nie uda³o siê ustawiæ OpenPGP.");

		privateKey = getPrivateKey(privateKeyId.c_str());
		error = gpgme_signers_add(context, privateKey);
		fail_if_err(error, L"Nie uda³o siê ustawiæ klucza prywatnego.");
	}

	void GPGWrapper::deinit()
	{
		if (!initialized)
			return;

		gpgme_signers_clear(context);
		gpgme_release(context);
	}

	gpgme_key_t GPGWrapper::getPrivateKey(const char * identifier)
	{
		return getKey(identifier, true);
	}

	gpgme_key_t GPGWrapper::getPublicKey(const char * identifier)
	{
		IdToGPGKey::iterator it = reciversIdToKey.find(identifier);
		if (it != reciversIdToKey.end())
			return it->second;

		return getKey(identifier, false);
	}

	gpgme_key_t GPGWrapper::getKey(const char * identifier, bool isPrivate)
	{
		gpgme_key_t result;

		gpgme_error_t error = gpgme_op_keylist_start(context, identifier, isPrivate ? 1 : 0);
		fail_if_err(error, L"Nie uda³o siê zainicjowaæ pobierania klucza.");

		error = gpgme_op_keylist_next(context, &result);
		fail_if_err(error, L"Nie uda³o siê pobraæ klucza.");

		reciversIdToKey[identifier] = result;

		error = gpgme_op_keylist_end(context);
		fail_if_err(error, L"Nie uda³o siê zakoñczyæ pobieranie klucza.");

		return result;
	}

} // namespace wtwGPG