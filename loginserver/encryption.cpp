/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <openssl/des.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <cstring>
#include <string>

#ifdef ENABLE_SECURITY

#include <sodium.h>

#endif

#include "encryption.h"

/**
 * @param buffer_in
 * @param buffer_in_sz
 * @param buffer_out
 * @param enc
 * @return
 */
const char *eqcrypt_block(const char *buffer_in, size_t buffer_in_sz, char *buffer_out, bool enc)
{
	DES_key_schedule k;
	DES_cblock       v;

	memset(&k, 0, sizeof(DES_key_schedule));
	memset(&v, 0, sizeof(DES_cblock));

	if (!enc && buffer_in_sz && buffer_in_sz % 8 != 0) {
		return nullptr;
	}

	DES_ncbc_encrypt((const unsigned char *) buffer_in, (unsigned char *) buffer_out, (long) buffer_in_sz, &k, &v, enc);
	return buffer_out;
}

/**
 * @param msg
 * @return
 */
std::string eqcrypt_md5(const std::string &msg)
{
	std::string   ret;
	unsigned char md5_digest[16];
	char          tmp[4];

	MD5((const unsigned char *) msg.c_str(), msg.length(), md5_digest);

	for (int i = 0; i < 16; ++i) {
		sprintf(&tmp[0], "%02x", md5_digest[i]);
		ret.push_back(tmp[0]);
		ret.push_back(tmp[1]);
	}

	return ret;
}

/**
 * @param msg
 * @return
 */
std::string eqcrypt_sha1(const std::string &msg)
{
	std::string   ret;
	unsigned char sha_digest[20];
	char          tmp[4];

	SHA1((const unsigned char *) msg.c_str(), msg.length(), sha_digest);

	for (int i = 0; i < 20; ++i) {
		sprintf(&tmp[0], "%02x", sha_digest[i]);
		ret.push_back(tmp[0]);
		ret.push_back(tmp[1]);
	}

	return ret;
}

/**
 * @param msg
 * @return
 */
std::string eqcrypt_sha512(const std::string &msg)
{
	std::string   ret;
	unsigned char sha_digest[64];
	char          tmp[4];

	SHA512((const unsigned char *) msg.c_str(), msg.length(), sha_digest);

	for (int i = 0; i < 64; ++i) {
		sprintf(&tmp[0], "%02x", sha_digest[i]);
		ret.push_back(tmp[0]);
		ret.push_back(tmp[1]);
	}

	return ret;
}

#ifdef ENABLE_SECURITY

/**
 * @param msg
 * @return
 */
std::string eqcrypt_argon2(const std::string &msg)
{
	char buffer[crypto_pwhash_STRBYTES];

	if (crypto_pwhash_str(
		&buffer[0],
		&msg[0],
		msg.length(),
		crypto_pwhash_OPSLIMIT_INTERACTIVE,
		crypto_pwhash_MEMLIMIT_INTERACTIVE
	) != 0) {
		return "";
	}

	return buffer;
}

/**
 * @param msg
 * @return
 */
std::string eqcrypt_scrypt(const std::string &msg)
{
	char buffer[crypto_pwhash_scryptsalsa208sha256_STRBYTES];

	if (crypto_pwhash_scryptsalsa208sha256_str(
		&buffer[0], &msg[0], msg.length(),
		crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_INTERACTIVE, crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_INTERACTIVE
	) != 0) {
		return "";
	}

	return buffer;
}

#endif

/**
 * @param username
 * @param password
 * @param mode
 * @return
 */
std::string eqcrypt_hash(const std::string &username, const std::string &password, int mode)
{
	switch (mode) {
		case EncryptionModeMD5:
			return eqcrypt_md5(password);
		case EncryptionModeMD5PassUser:
			return eqcrypt_md5(password + ":" + username);
		case EncryptionModeMD5UserPass:
			return eqcrypt_md5(username + ":" + password);
		case EncryptionModeMD5Triple:
			return eqcrypt_md5(eqcrypt_md5(username) + eqcrypt_md5(password));
		case EncryptionModeSHA:
			return eqcrypt_sha1(password);
		case EncryptionModeSHAPassUser:
			return eqcrypt_sha1(password + ":" + username);
		case EncryptionModeSHAUserPass:
			return eqcrypt_sha1(username + ":" + password);
		case EncryptionModeSHATriple:
			return eqcrypt_sha1(eqcrypt_sha1(username) + eqcrypt_sha1(password));
		case EncryptionModeSHA512:
			return eqcrypt_sha512(password);
		case EncryptionModeSHA512PassUser:
			return eqcrypt_sha512(password + ":" + username);
		case EncryptionModeSHA512UserPass:
			return eqcrypt_sha512(username + ":" + password);
		case EncryptionModeSHA512Triple:
			return eqcrypt_sha512(eqcrypt_sha512(username) + eqcrypt_sha512(password));
#ifdef ENABLE_SECURITY
		case EncryptionModeArgon2:
			return eqcrypt_argon2(password);
		case EncryptionModeSCrypt:
			return eqcrypt_scrypt(password);
#endif
			//todo bcrypt? pbkdf2?
		default:
			return "";
			break;
	}
}

/**
 * @param username
 * @param password
 * @param pwhash
 * @param mode
 * @return
 */
bool eqcrypt_verify_hash(const std::string &username, const std::string &password, const std::string &pwhash, int mode)
{
	switch (mode) {
#ifdef ENABLE_SECURITY
		case 13:
			return crypto_pwhash_str_verify(&pwhash[0], &password[0], password.length()) == 0;
		case 14:
			return crypto_pwhash_scryptsalsa208sha256_str_verify(&pwhash[0], &password[0], password.length()) == 0;
#endif
		default: {
			auto hash = eqcrypt_hash(username, password, mode);
			return hash.compare(pwhash) == 0;
		}
	}

	return false;
}

std::string GetEncryptionByModeId(uint32 mode) {
	switch (mode) {
		case EncryptionModeMD5:
			return "MD5";
		case EncryptionModeMD5PassUser:
			return "MD5PassUser";
		case EncryptionModeMD5UserPass:
			return "MD5UserPass";
		case EncryptionModeMD5Triple:
			return "MD5Triple";
		case EncryptionModeSHA:
			return "SHA";
		case EncryptionModeSHAPassUser:
			return "SHAPassUser";
		case EncryptionModeSHAUserPass:
			return "SHAUserPass";
		case EncryptionModeSHATriple:
			return "SHATriple";
		case EncryptionModeSHA512:
			return "SHA512";
		case EncryptionModeSHA512PassUser:
			return "SHA512PassUser";
		case EncryptionModeSHA512UserPass:
			return "SHA512UserPass";
		case EncryptionModeSHA512Triple:
			return "SHA512Triple";
		case EncryptionModeArgon2:
			return "Argon2";
		case EncryptionModeSCrypt:
			return "SCrypt";
		default:
			return "";
	}
}