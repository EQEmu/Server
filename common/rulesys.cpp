/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "rulesys.h"
#include "database.h"
#include "string_util.h"
#include <cstdlib>
#include <cstring>

/*
	Commands:
	#rules:
	current -> lists current set name
	switch (set name) -> change set in the DB, but dont reload
	load (set name) -> load set into this zone without changing the world
	wload (set name) -> tell world and all zones to load this rule set
	store [set name] -> store the current rules in this zone to the set (or active if not specified)
	reset -> reset all rule values to their defaults.
	list [catname]
	set (cat) (rule) (value)
	values [catname] -> show the values of all rules in the specified category/
*/

const char *RuleManager::s_categoryNames[_CatCount+1] = {
	#define RULE_CATEGORY(catname) \
		#catname ,
	#include "ruletypes.h"
	"InvalidCategory"
};

const RuleManager::RuleInfo RuleManager::s_RuleInfo[_IntRuleCount+_RealRuleCount+_BoolRuleCount+1] = {
	/* this is done in three steps so we can reliably get to them by index*/
	#define RULE_INT(cat, rule, default_value) \
		{ #cat ":" #rule, Category__##cat, IntRule, Int__##rule },
	#include "ruletypes.h"
	#define RULE_REAL(cat, rule, default_value) \
		{ #cat ":" #rule, Category__##cat, RealRule, Real__##rule },
	#include "ruletypes.h"
	#define RULE_BOOL(cat, rule, default_value) \
		{ #cat ":" #rule, Category__##cat, BoolRule, Bool__##rule },
	#include "ruletypes.h"
	{ "Invalid Rule", _CatCount, IntRule }
};

RuleManager::RuleManager()
:	m_activeRuleset(0),
	m_activeName("default")
{
	ResetRules();
}

RuleManager::CategoryType RuleManager::FindCategory(const char *catname) {
	int i;
	for (i = 0; i < _CatCount; i++) {
		if (strcasecmp(catname, s_categoryNames[i]) == 0)
			return((CategoryType)i);
	}
	return(InvalidCategory);
}

bool RuleManager::ListRules(const char *catname, std::vector<const char *> &into) {
	CategoryType cat = InvalidCategory;
	if (catname != nullptr) {
		cat = FindCategory(catname);
		if (cat == InvalidCategory) {
			Log(Logs::Detail, Logs::Rules, "Unable to find category '%s'", catname);
			return(false);
		}
	}
	int i;
	int rule_count = CountRules();
	for (i = 0; i < rule_count; i++) {
		const RuleInfo &rule = s_RuleInfo[i];
		if (catname == nullptr || cat == rule.category) {
			into.push_back(rule.name);
		}
	}
	return(true);
}

bool RuleManager::ListCategories(std::vector<const char *> &into) {
	int i;
	for (i = 0; i < _CatCount; i++) {
		into.push_back(s_categoryNames[i]);
	}
	return(true);
}

bool RuleManager::GetRule(const char *rule_name, std::string &return_value) {
	RuleType type;
	uint16 index;
	if (!_FindRule(rule_name, type, index))
		return false;

	char tmp[255] = "";
	switch(type) {
		case IntRule:
			sprintf(tmp, "%i", m_RuleIntValues[index]);
			break;
		case RealRule:
			sprintf(tmp, "%f", m_RuleRealValues[index]);
			break;
		case BoolRule:
			std::string tmp_val = m_RuleBoolValues[index] ? "true" : "false";
			sprintf(tmp, "%s", tmp_val.c_str());
			break;
	}

	return_value = tmp;

	return true;
}

bool RuleManager::SetRule(const char *rule_name, const char *rule_value, Database *database, bool db_save) {
	if(rule_name == nullptr || rule_value == nullptr)
		return(false);

	RuleType type;
	uint16 index;
	if(!_FindRule(rule_name, type, index))
		return(false);

	switch(type) {
		case IntRule:
			m_RuleIntValues[index] = atoi(rule_value);
			Log(Logs::Detail, Logs::Rules, "Set rule %s to value %d", rule_name, m_RuleIntValues[index]);
			break;
		case RealRule:
			m_RuleRealValues[index] = atof(rule_value);
			Log(Logs::Detail, Logs::Rules, "Set rule %s to value %.13f", rule_name, m_RuleRealValues[index]);
			break;
		case BoolRule:
			uint32 val = 0;
			if (!strcasecmp(rule_value, "on") || !strcasecmp(rule_value, "true") || !strcasecmp(rule_value, "yes") || !strcasecmp(rule_value, "enabled") || !strcmp(rule_value, "1"))
				val = 1;

			m_RuleBoolValues[index] = val;
			Log(Logs::Detail, Logs::Rules, "Set rule %s to value %s", rule_name, m_RuleBoolValues[index] == 1 ? "true" : "false");
			break;
	}

	if(db_save)
		_SaveRule(database, type, index);

	return(true);
}

void RuleManager::ResetRules() {
	Log(Logs::Detail, Logs::Rules, "Resetting running rules to default values");
	#define RULE_INT(cat, rule, default_value) \
		m_RuleIntValues[ Int__##rule ] = default_value;
	#define RULE_REAL(cat, rule, default_value) \
		m_RuleRealValues[ Real__##rule ] = default_value;
	#define RULE_BOOL(cat, rule, default_value) \
		m_RuleBoolValues[ Bool__##rule ] = default_value;
	#include "ruletypes.h"
}

bool RuleManager::_FindRule(const char *rule_name, RuleType &type_into, uint16 &index_into) {
	if (rule_name == nullptr)
		return(false);

	int i;
	int rule_count = CountRules();
	for (i = 0; i < rule_count; i++) {
		const RuleInfo &rule = s_RuleInfo[i];
		if (strcmp(rule_name, rule.name) == 0) {
			type_into = rule.type;
			index_into = rule.rule_index;
			return(true);
		}
	}
	Log(Logs::Detail, Logs::Rules, "Unable to find rule '%s'", rule_name);
	return(false);
}

//assumes index is valid!
const char *RuleManager::_GetRuleName(RuleType type, uint16 index) {
	switch (type) {
		case IntRule:
			return(s_RuleInfo[index].name);
		case RealRule:
			return(s_RuleInfo[index + _IntRuleCount].name);
		case BoolRule:
			return(s_RuleInfo[index + _IntRuleCount + _RealRuleCount].name);
	}
	//should never happen
	return("InvalidRule??");
}

void RuleManager::SaveRules(Database *database, const char *ruleset_name) {

	if (ruleset_name != nullptr) {
		//saving to a specific name
		if (m_activeName != ruleset_name) {
			//a new name...

			m_activeRuleset = _FindOrCreateRuleset(database, ruleset_name);
			if (m_activeRuleset == -1) {
				Log(Logs::Detail, Logs::Rules, "Unable to find or create rule set %s", ruleset_name);
				return;
			}
			m_activeName = ruleset_name;
		}
		Log(Logs::Detail, Logs::Rules, "Saving running rules into rule set %s (%d)", ruleset_name, m_activeRuleset);
	}
	else {
		Log(Logs::Detail, Logs::Rules, "Saving running rules into running rule set %s", m_activeName.c_str(), m_activeRuleset);
	}

	int i;
	for (i = 0; i < _IntRuleCount; i++) {
		_SaveRule(database, IntRule, i);
	}
	for (i = 0; i < _RealRuleCount; i++) {
		_SaveRule(database, RealRule, i);
	}
	for (i = 0; i < _BoolRuleCount; i++) {
		_SaveRule(database, BoolRule, i);
	}
}

bool RuleManager::LoadRules(Database *database, const char *ruleset_name) {

	int ruleset_id = this->GetRulesetID(database, ruleset_name);
	if (ruleset_id < 0) {
		Log(Logs::Detail, Logs::Rules, "Failed to find ruleset '%s' for load operation. Canceling.", ruleset_name);
		return (false);
	}

	Log(Logs::Detail, Logs::Rules, "Loading rule set '%s' (%d)", ruleset_name, ruleset_id);

	m_activeRuleset = ruleset_id;
	m_activeName    = ruleset_name;

	/* Load default ruleset values first if we're loading something other than default */
	if (strcasecmp(ruleset_name, "default") != 0) {
		std::string default_ruleset_name = "default";
		int         default_ruleset_id   = GetRulesetID(database, default_ruleset_name.c_str());
		if (default_ruleset_id < 0) {
			Log(Logs::Detail, Logs::Rules, "Failed to find default ruleset '%s' for load operation. Canceling.",
			    default_ruleset_name.c_str());
			return (false);
		}
		Log(Logs::Detail, Logs::Rules, "Loading rule set '%s' (%d)", default_ruleset_name.c_str(), default_ruleset_id);

		std::string query = StringFormat(
				"SELECT rule_name, rule_value FROM rule_values WHERE ruleset_id = %d",
				default_ruleset_id
		);

		auto results = database->QueryDatabase(query);
		if (!results.Success())
			return false;

		for (auto row = results.begin(); row != results.end(); ++row)
			if (!SetRule(row[0], row[1], nullptr, false))
				Log(Logs::Detail, Logs::Rules, "Unable to interpret rule record for %s", row[0]);
	}

	std::string query   = StringFormat("SELECT rule_name, rule_value FROM rule_values WHERE ruleset_id=%d", ruleset_id);
	auto        results = database->QueryDatabase(query);
	if (!results.Success())
		return false;

	for (auto row = results.begin(); row != results.end(); ++row)
		if (!SetRule(row[0], row[1], nullptr, false))
			Log(Logs::Detail, Logs::Rules, "Unable to interpret rule record for %s", row[0]);

	return true;
}

void RuleManager::_SaveRule(Database *database, RuleType type, uint16 index) {
	char value_string[100];

	switch (type) {
		case IntRule:
			sprintf(value_string, "%d", m_RuleIntValues[index]);
			break;
		case RealRule:
			sprintf(value_string, "%.13f", m_RuleRealValues[index]);
			break;
		case BoolRule:
			sprintf(value_string, "%s", m_RuleBoolValues[index] ? "true" : "false");
			break;
	}

	std::string query = StringFormat(
			"REPLACE INTO rule_values "
			"(ruleset_id, rule_name, rule_value) "
			" VALUES(%d, '%s', '%s')",
			m_activeRuleset,
			_GetRuleName(type, index),
			value_string
	);

	database->QueryDatabase(query);
}


int RuleManager::GetRulesetID(Database *database, const char *ruleset_name) {

	uint32 len = static_cast<uint32>(strlen(ruleset_name));
	auto rst = new char[2 * len + 1];
	database->DoEscapeString(rst, ruleset_name, len);

	std::string query = StringFormat("SELECT ruleset_id FROM rule_sets WHERE name='%s'", rst);
	safe_delete_array(rst);
	auto results = database->QueryDatabase(query);
	if (!results.Success())
		return -1;

	if (results.RowCount() == 0)
		return -1;

	auto row = results.begin();

	return atoi(row[0]);
}

int RuleManager::_FindOrCreateRuleset(Database *database, const char *in_ruleset_name) {

	int ruleset_id = GetRulesetID(database, in_ruleset_name);
	if (ruleset_id >= 0)
		return ruleset_id;	//found and existing one...

	uint32 len = strlen(in_ruleset_name);
	auto ruleset_name = new char[2 * len + 1];
	database->DoEscapeString(ruleset_name, in_ruleset_name, len);

	std::string query = StringFormat("INSERT INTO rule_sets (ruleset_id, name) VALUES(0, '%s')", ruleset_name);
	safe_delete_array(ruleset_name);
	auto results = database->QueryDatabase(query);
	if (!results.Success())
		return -1;

	return results.LastInsertedID();
}

std::string RuleManager::GetRulesetName(Database *database, int ruleset_id) {
	std::string query = StringFormat("SELECT name FROM rule_sets WHERE ruleset_id=%d", ruleset_id);
	auto results = database->QueryDatabase(query);
	if (!results.Success())
		return "";

	if (results.RowCount() == 0)
		return "";

	auto row = results.begin();

	return row[0];
}

bool RuleManager::ListRulesets(Database *database, std::map<int, std::string> &into) {

	//start out with the default set which is always present.
	into[0] = "default";

	std::string query = "SELECT ruleset_id, name FROM rule_sets";
	auto results = database->QueryDatabase(query);
	if (results.Success())
		return false;

	for (auto row = results.begin(); row != results.end(); ++row)
		into[atoi(row[0])] = row[1];

	return true;
}

int32 RuleManager::GetIntRule(RuleManager::IntType t) const
{
	return (m_RuleIntValues[t]);
}

float RuleManager::GetRealRule(RuleManager::RealType t) const
{
	return (m_RuleRealValues[t]);
}

bool RuleManager::GetBoolRule(RuleManager::BoolType t) const
{
	return (m_RuleBoolValues[t] == 1);
}

