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

#pragma once

#include <string>
#include "../common/types.h"

enum EncryptionMode {
	EncryptionModeMD5            = 1,
	EncryptionModeMD5PassUser    = 2,
	EncryptionModeMD5UserPass    = 3,
	EncryptionModeMD5Triple      = 4,
	EncryptionModeSHA            = 5,
	EncryptionModeSHAPassUser    = 6,
	EncryptionModeSHAUserPass    = 7,
	EncryptionModeSHATriple      = 8,
	EncryptionModeSHA512         = 9,
	EncryptionModeSHA512PassUser = 10,
	EncryptionModeSHA512UserPass = 11,
	EncryptionModeSHA512Triple   = 12,
	EncryptionModeArgon2         = 13,
	EncryptionModeSCrypt         = 14
};

namespace CryptoHash {
	const int md5_hash_length    = 32;
	const int sha1_hash_length   = 40;
	const int sha512_hash_length = 128;
}

/**
 * @param mode
 * @return
 */
std::string GetEncryptionByModeId(uint32 mode);
const char *eqcrypt_block(const char *buffer_in, size_t buffer_in_sz, char *buffer_out, bool enc);
std::string eqcrypt_hash(const std::string &username, const std::string &password, int mode);
bool eqcrypt_verify_hash(const std::string &username, const std::string &password, const std::string &pwhash, int mode);
