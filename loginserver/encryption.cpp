#include <openssl/des.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <cstring>
#include <string>
#ifdef ENABLE_SECURITY
#include <sodium.h>
#endif

const char* eqcrypt_block(const char *buffer_in, size_t buffer_in_sz, char* buffer_out, bool enc) {
	DES_key_schedule k;
	DES_cblock v;

	memset(&k, 0, sizeof(DES_key_schedule));
	memset(&v, 0, sizeof(DES_cblock));

	if (!enc && buffer_in_sz && buffer_in_sz % 8 != 0) {
		return nullptr;
	}

	DES_ncbc_encrypt((const unsigned char*)buffer_in, (unsigned char*)buffer_out, (long)buffer_in_sz, &k, &v, enc);
	return buffer_out;
}

std::string eqcrypt_md5(const std::string &msg) {
	std::string ret;
	unsigned char md5_digest[16];
	char tmp[4];

	MD5((const unsigned char*)msg.c_str(), msg.length(), md5_digest);

	for (int i = 0; i < 16; ++i) {
		sprintf(&tmp[0], "%02x", md5_digest[i]);
		ret.push_back(tmp[0]);
		ret.push_back(tmp[1]);
	}

	return ret;
}

std::string eqcrypt_sha1(const std::string &msg) {
	std::string ret;
	unsigned char sha_digest[20];
	char tmp[4];

	SHA1((const unsigned char*)msg.c_str(), msg.length(), sha_digest);

	for (int i = 0; i < 20; ++i) {
		sprintf(&tmp[0], "%02x", sha_digest[i]);
		ret.push_back(tmp[0]);
		ret.push_back(tmp[1]);
	}

	return ret;
}

std::string eqcrypt_sha512(const std::string &msg) {
	std::string ret;
	unsigned char sha_digest[64];
	char tmp[4];

	SHA512((const unsigned char*)msg.c_str(), msg.length(), sha_digest);

	for (int i = 0; i < 64; ++i) {
		sprintf(&tmp[0], "%02x", sha_digest[i]);
		ret.push_back(tmp[0]);
		ret.push_back(tmp[1]);
	}

	return ret;
}

#ifdef ENABLE_SECURITY

std::string eqcrypt_argon2(const std::string &msg)
{
	std::string ret;
	ret.resize(crypto_pwhash_STRBYTES);

	if (crypto_pwhash_str(&ret[0], &msg[0], msg.length(), crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE) != 0) {
		return "";
	}

	return ret;
}

std::string eqcrypt_scrypt(const std::string &msg)
{
	std::string ret;
	ret.resize(crypto_pwhash_scryptsalsa208sha256_STRBYTES);

	if (crypto_pwhash_scryptsalsa208sha256_str(&ret[0], &msg[0], msg.length(),
		crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_SENSITIVE, crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_SENSITIVE) != 0) {
		return "";
	}

	return ret;
}

#endif

std::string eqcrypt_hash(const std::string &username, const std::string &password, int mode) {
	switch (mode)
	{
	case 1:
		return eqcrypt_md5(password);
	case 2:
		return eqcrypt_md5(password + ":" + username);
	case 3:
		return eqcrypt_md5(username + ":" + password);
	case 4:
		return eqcrypt_md5(eqcrypt_md5(username) + eqcrypt_md5(password));
	case 5:
		return eqcrypt_sha1(password);
	case 6:
		return eqcrypt_sha1(password + ":" + username);
	case 7:
		return eqcrypt_sha1(username + ":" + password);
	case 8:
		return eqcrypt_sha1(eqcrypt_sha1(username) + eqcrypt_sha1(password));
	case 9:
		return eqcrypt_sha512(password);
	case 10:
		return eqcrypt_sha512(password + ":" + username);
	case 11:
		return eqcrypt_sha512(username + ":" + password);
	case 12:
		return eqcrypt_sha512(eqcrypt_sha512(username) + eqcrypt_sha512(password));
#ifdef ENABLE_SECURITY
	case 13:
		return eqcrypt_argon2(password);
	case 14:
		return eqcrypt_scrypt(password);
#endif
		//todo bcrypt? pbkdf2?
	default:
		return "";
		break;
	}
}

bool eqcrypt_verify_hash(const std::string &username, const std::string &password, const std::string &pwhash, int mode) {
	switch (mode)
	{
#ifdef ENABLE_SECURITY
	case 13:
		return crypto_pwhash_str_verify(&pwhash[0], &password[0], password.length()) == 0;
	case 14:
		return crypto_pwhash_scryptsalsa208sha256_str_verify(&pwhash[0], &password[0], password.length()) == 0;
#endif
	default:
	{
		auto hash = eqcrypt_hash(username, password, mode);
		return hash.compare(pwhash) == 0;
	}
	}

	return false;
}