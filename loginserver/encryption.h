#pragma once

#include <string>

const char* eqcrypt_block(const char *buffer_in, size_t buffer_in_sz, char* buffer_out, bool enc);
std::string eqcrypt_hash(const std::string &username, const std::string &password, int mode);
bool eqcrypt_verify_hash(const std::string &username, const std::string &password, const std::string &pwhash, int mode);
