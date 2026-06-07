// Copyright 2024 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_RSA_H
#define FS_RSA_H

struct evp_pkey_st;
typedef struct evp_pkey_st EVP_PKEY;

/**
 * @namespace tfs::rsa
 * @brief This namespace provides functions to load and use RSA keys for decryption
 * (See https://wikipedia.org/wiki/RSA_cryptosystem).
 */
namespace tfs::rsa {

/**
 * @brief Loads an RSA private key from a PEM-encoded string.
 *
 * @param {pem} A PEM-encoded private key string.
 * @return A pointer to the loaded EVP_PKEY. The key is stored internally and reused across calls.
 * @throws std::runtime_error If the PEM data is malformed or the private key cannot be read.
 */
EVP_PKEY* loadPEM(std::string_view pem);

/**
 * @brief Decrypts data using the loaded RSA private key.
 *
 * @param {msg} A pointer to the data to be decrypted. The decryption is performed in-place.
 * @param {len} The length of the data to be decrypted, in bytes.
 */
void decrypt(uint8_t* msg, size_t len);

} // namespace tfs::rsa

#endif // FS_RSA_H
