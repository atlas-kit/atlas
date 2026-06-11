// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "rsa.h"

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

namespace {

/**
 * @struct Deleter
 * @brief Custom deleter for OpenSSL RAII wrappers.
 */
struct Deleter
{
	void operator()(BIO* bio) const { BIO_free(bio); }
	void operator()(EVP_PKEY* pkey) const { EVP_PKEY_free(pkey); }
	void operator()(EVP_PKEY_CTX* ctx) const { EVP_PKEY_CTX_free(ctx); }
};

/**
 * @brief Alias for std::unique_ptr with OpenSSL custom deleter.
 */
template <class T>
using C_ptr = std::unique_ptr<T, Deleter>;

/**
 * @brief The global RSA private key used for decryption.
 */
C_ptr<EVP_PKEY> pkey = nullptr;

} // namespace

namespace tfs::rsa {

/**
 * @brief Decrypts data in-place using the loaded RSA private key with no padding.
 *
 * @param {msg} Pointer to the ciphertext buffer. The decrypted plaintext is written in-place.
 * @param {len} Length of the ciphertext in bytes.
 */
void decrypt(uint8_t* msg, size_t len)
{
	C_ptr<EVP_PKEY_CTX> pctx{EVP_PKEY_CTX_new_from_pkey(nullptr, pkey.get(), nullptr)};
	EVP_PKEY_decrypt_init(pctx.get());
	EVP_PKEY_CTX_set_rsa_padding(pctx.get(), RSA_NO_PADDING);

	EVP_PKEY_decrypt(pctx.get(), msg, &len, msg, len);
}

/**
 * @brief Loads an RSA private key from a PEM string and stores it globally.
 *
 * @param {pem} A PEM-encoded private key string.
 * @return A pointer to the loaded EVP_PKEY.
 * @throws std::runtime_error If reading the PEM or private key fails.
 */
EVP_PKEY* loadPEM(std::string_view pem)
{
	C_ptr<BIO> bio{BIO_new(BIO_s_mem())};
	if (!BIO_write(bio.get(), pem.data(), pem.size())) {
		throw std::runtime_error(
		    std::format("Error while reading PEM data: {}", ERR_error_string(ERR_get_error(), nullptr)));
	}

	EVP_PKEY* pkey_ = nullptr;
	if (!PEM_read_bio_PrivateKey(bio.get(), &pkey_, nullptr, nullptr)) {
		throw std::runtime_error(
		    std::format("Error while reading private key: {}", ERR_error_string(ERR_get_error(), nullptr)));
	}

	pkey.reset(pkey_);
	return pkey_;
}

} // namespace tfs::rsa
