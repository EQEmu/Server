/*	EQEMu: Everquest Server Emulator
	
	Copyright (C) 2001-2019 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "profanity_manager.h"
#include "dbcore.h"

#include <ctype.h>
#include <cstring>
#include <algorithm>


static std::list<std::string> profanity_list;
static bool update_originator_flag = false;

bool EQEmu::ProfanityManager::LoadProfanityList(DBcore *db) {
	if (update_originator_flag == true) {
		update_originator_flag = false;
		return true;
	}

	if (!load_database_entries(db))
		return false;

	return true;
}

bool EQEmu::ProfanityManager::UpdateProfanityList(DBcore *db) {
	if (!load_database_entries(db))
		return false;

	update_originator_flag = true;

	return true;
}

bool EQEmu::ProfanityManager::DeleteProfanityList(DBcore *db) {
	if (!clear_database_entries(db))
		return false;

	update_originator_flag = true;

	return true;
}

bool EQEmu::ProfanityManager::AddProfanity(DBcore *db, const char *profanity) {
	if (!db || !profanity)
		return false;

	std::string entry(profanity);

	std::transform(entry.begin(), entry.end(), entry.begin(), [](unsigned char c) -> unsigned char { return tolower(c); });

	if (check_for_existing_entry(entry.c_str()))
		return true;

	if (entry.length() < REDACTION_LENGTH_MIN)
		return false;

	profanity_list.push_back(entry);

	std::string query = "REPLACE INTO `profanity_list` (`word`) VALUES ('";
	query.append(entry);
	query.append("')");
	auto results = db->QueryDatabase(query);
	if (!results.Success())
		return false;

	update_originator_flag = true;

	return true;
}

bool EQEmu::ProfanityManager::RemoveProfanity(DBcore *db, const char *profanity) {
	if (!db || !profanity)
		return false;

	std::string entry(profanity);

	std::transform(entry.begin(), entry.end(), entry.begin(), [](unsigned char c) -> unsigned char { return tolower(c); });

	if (!check_for_existing_entry(entry.c_str()))
		return true;

	profanity_list.remove(entry);

	std::string query = "DELETE FROM `profanity_list` WHERE `word` LIKE '";
	query.append(entry);
	query.append("'");
	auto results = db->QueryDatabase(query);
	if (!results.Success())
		return false;

	update_originator_flag = true;

	return true;
}

void EQEmu::ProfanityManager::RedactMessage(char *message) {
	if (!message)
		return;

	std::string test_message(message);
	// hard-coded max length based on channel message buffer size (4096 bytes)..
	// ..will need to change or remove if other sources are used for redaction
	if (test_message.length() < REDACTION_LENGTH_MIN || test_message.length() >= 4096)
		return;

	std::transform(test_message.begin(), test_message.end(), test_message.begin(), [](unsigned char c) -> unsigned char { return tolower(c); });
	
	for (const auto &iter : profanity_list) { // consider adding textlink checks if it becomes an issue
		size_t pos = 0;
		size_t start_pos = 0;

		while (pos != std::string::npos) {
			pos = test_message.find(iter, start_pos);
			if (pos == std::string::npos)
				continue;

			if ((pos + iter.length()) == test_message.length() || !isalpha(test_message.at(pos + iter.length()))) {
				if (pos == 0 || !isalpha(test_message.at(pos - 1)))
					memset((message + pos), REDACTION_CHARACTER, iter.length());
			}

			start_pos = (pos + iter.length());
		}
	}
}

void EQEmu::ProfanityManager::RedactMessage(std::string &message) {
	if (message.length() < REDACTION_LENGTH_MIN || message.length() >= 4096)
		return;

	std::string test_message(const_cast<const std::string&>(message));

	std::transform(test_message.begin(), test_message.end(), test_message.begin(), [](unsigned char c) -> unsigned char { return tolower(c); });

	for (const auto &iter : profanity_list) { // consider adding textlink checks if it becomes an issue
		size_t pos = 0;
		size_t start_pos = 0;

		while (pos != std::string::npos) {
			pos = test_message.find(iter, start_pos);
			if (pos == std::string::npos)
				continue;

			if ((pos + iter.length()) == test_message.length() || !isalpha(test_message.at(pos + iter.length()))) {
				if (pos == 0 || !isalpha(test_message.at(pos - 1)))
					message.replace(pos, iter.length(), iter.length(), REDACTION_CHARACTER);
			}

			start_pos = (pos + iter.length());
		}
	}
}

bool EQEmu::ProfanityManager::ContainsCensoredLanguage(const char *message) {
	if (!message)
		return false;

	return ContainsCensoredLanguage(std::string(message));
}

bool EQEmu::ProfanityManager::ContainsCensoredLanguage(const std::string &message) {
	if (message.length() < REDACTION_LENGTH_MIN || message.length() >= 4096)
		return false;

	std::string test_message(message);

	std::transform(test_message.begin(), test_message.end(), test_message.begin(), [](unsigned char c) -> unsigned char { return tolower(c); });

	for (const auto &iter : profanity_list) {
		if (test_message.find(iter) != std::string::npos)
			return true;
	}

	return false;
}

const std::list<std::string> &EQEmu::ProfanityManager::GetProfanityList() {
	return profanity_list;
}

bool EQEmu::ProfanityManager::IsCensorshipActive() {
	return (profanity_list.size() != 0);
}

bool EQEmu::ProfanityManager::load_database_entries(DBcore *db) {
	if (!db)
		return false;

	profanity_list.clear();

	std::string query = "SELECT `word` FROM `profanity_list`";
	auto results = db->QueryDatabase(query);
	if (!results.Success())
		return false;

	for (auto row = results.begin(); row != results.end(); ++row) {
		if (std::strlen(row[0]) >= REDACTION_LENGTH_MIN) {
			std::string entry(row[0]);
			std::transform(entry.begin(), entry.end(), entry.begin(), [](unsigned char c) -> unsigned char { return tolower(c); });
			if (!check_for_existing_entry(entry.c_str()))
				profanity_list.push_back((std::string)entry);
		}
	}

	return true;
}

bool EQEmu::ProfanityManager::clear_database_entries(DBcore *db) {
	if (!db)
		return false;

	profanity_list.clear();

	std::string query = "DELETE FROM `profanity_list`";
	auto results = db->QueryDatabase(query);
	if (!results.Success())
		return false;

	return true;
}

bool EQEmu::ProfanityManager::check_for_existing_entry(const char *profanity) {
	if (!profanity)
		return false;

	for (const auto &iter : profanity_list) {
		if (iter.compare(profanity) == 0)
			return true;
	}
	
	return false;
}
