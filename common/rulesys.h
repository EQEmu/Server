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
#ifndef RULESYS_H_
#define RULESYS_H_

/*
* Access to the rules system in normal code is done with three calls:
* - RuleI(category, rule) -> fetch an integer rule's value
* - RuleR(category, rule) -> fetch a real (float) rule's value
* - RuleB(category, rule) -> fetch a boolean/flag rule's value
*
*/

//note, these macros assume there is always a RuleManager *rules in scope,
//which makes it a global for now, but with instancing we will do exactly
//what we do with the zone global and just make it a member of core classes
#define RuleI(cat, rule) \
	RuleManager::Instance()->GetIntRule( RuleManager::Int__##rule )
#define RuleR(cat, rule) \
	RuleManager::Instance()->GetRealRule( RuleManager::Real__##rule )
#define RuleB(cat, rule) \
	RuleManager::Instance()->GetBoolRule( RuleManager::Bool__##rule )


#include <vector>
#include <string>
#include <map>

#include "types.h"

class Database;

class RuleManager {
public:
	//generate our rule enums:
	typedef enum {
	#define RULE_INT(cat, rule, default_value, notes) \
		Int__##rule,
	#include "ruletypes.h"
		_IntRuleCount
	} IntType;

	typedef enum {
	#define RULE_REAL(cat, rule, default_value, notes) \
		Real__##rule,
	#include "ruletypes.h"
		_RealRuleCount
	} RealType;

	typedef enum {
	#define RULE_BOOL(cat, rule, default_value, notes) \
		Bool__##rule,
	#include "ruletypes.h"
		_BoolRuleCount
	} BoolType;

	typedef enum {
	#define RULE_CATEGORY(catname) \
		Category__##catname,
	#include "ruletypes.h"
		_CatCount
	} CategoryType;

	static RuleManager* Instance() {
		static RuleManager rules;
		return &rules;
	}

	static const IntType InvalidInt = _IntRuleCount;
	static const RealType InvalidReal = _RealRuleCount;
	static const BoolType InvalidBool = _BoolRuleCount;
	static const CategoryType InvalidCategory = _CatCount;

	static const uint32 _RulesCount = _IntRuleCount+_RealRuleCount+_BoolRuleCount;

	//fetch routines, you should generally use the Rule* macros instead of this
	int32 GetIntRule (IntType t) const;
	float GetRealRule(RealType t) const;
	bool GetBoolRule(BoolType t) const;

	//management routines
	static const char *GetRuleName(IntType t) { return(s_RuleInfo[t].name); }
	static const char *GetRuleName(RealType t) { return(s_RuleInfo[t+_IntRuleCount].name); }
	static const char *GetRuleName(BoolType t) { return(s_RuleInfo[t+_IntRuleCount+_RealRuleCount].name); }
	static const std::string &GetRuleNotes(IntType t) { return(s_RuleInfo[t].notes); }
	static const std::string &GetRuleNotes(RealType t) { return(s_RuleInfo[t+_IntRuleCount].notes); }
	static const std::string &GetRuleNotes(BoolType t) { return(s_RuleInfo[t+_IntRuleCount+_RealRuleCount].notes); }
	static uint32 CountRules() { return(_RulesCount); }
	static CategoryType FindCategory(const char *catname);
	bool ListRules(const char *catname, std::vector<const char *> &into);
	bool ListCategories(std::vector<const char *> &into);
	bool GetRule(const char *rule_name, std::string &ret_val);
	bool SetRule(const char *rule_name, const char *rule_value, Database *db = nullptr, bool db_save = false, bool reload = false);

	int GetActiveRulesetID() const { return(m_activeRuleset); }
	const char *GetActiveRuleset() const { return(m_activeName.c_str()); }
	static int GetRulesetID(Database *db, const char *rulesetname);
	static std::string GetRulesetName(Database *db, int id);
	static bool ListRulesets(Database *db, std::map<int, std::string> &into);

	void ResetRules(bool reload = false);
	bool LoadRules(Database *db, const char *ruleset = nullptr, bool reload = false);
	void SaveRules(Database *db, const char *ruleset = nullptr);
	bool UpdateInjectedRules(Database *db, const char *ruleset_name, bool quiet_update = false);
	bool UpdateOrphanedRules(Database *db, bool quiet_update = false);
	bool RestoreRuleNotes(Database *db);

private:
	RuleManager();
	RuleManager(const RuleManager&);
	const RuleManager& operator=(const RuleManager&);

	int	m_activeRuleset;
	std::string m_activeName;
#ifdef WIN64
	uint32	m_RuleIntValues [_IntRuleCount ];
#else
	int m_RuleIntValues [_IntRuleCount ];
#endif
	float	m_RuleRealValues[_RealRuleCount];
	uint32	m_RuleBoolValues[_BoolRuleCount];

	typedef enum {
		IntRule,
		RealRule,
		BoolRule
	} RuleType;

	static bool _FindRule(const char *rule_name, RuleType &type_into, uint16 &index_into);
	static const char *_GetRuleName(RuleType type, uint16 index);
	static const std::string &_GetRuleNotes(RuleType type, uint16 index);
	static int _FindOrCreateRuleset(Database *db, const char *ruleset);
	void _SaveRule(Database *db, RuleType type, uint16 index);
	
	static const char *s_categoryNames[];
	typedef struct {
		const char *name;
		CategoryType category;
		RuleType type;
		uint16 rule_index;	//index into its 'type' array
		const std::string notes;
	} RuleInfo;
	static const RuleInfo s_RuleInfo[];

};

#endif /*RULESYS_H_*/

