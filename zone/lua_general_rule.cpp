#ifdef LUA_EQEMU
#include <sol/sol.hpp>
#include "../common/rulesys.h"

int get_rulei(int rule) {
	return RuleManager::Instance()->GetIntRule((RuleManager::IntType)rule);
}

float get_ruler(int rule) {
	return RuleManager::Instance()->GetRealRule((RuleManager::RealType)rule);
}

bool get_ruleb(int rule) {
	return RuleManager::Instance()->GetBoolRule((RuleManager::BoolType)rule);
}

void lua_register_rules_const(sol::state_view &sv)
{
	sv.new_enum<int>("Rule",
		{
#define RULE_INT(cat, rule, default_value, notes) \
		{ #rule, RuleManager::Int__##rule },
#include "../common/ruletypes.h"
		{ "_IntRuleCount", RuleManager::_IntRuleCount },
#undef RULE_INT
#define RULE_REAL(cat, rule, default_value, notes) \
		{ #rule, RuleManager::Real__##rule },
#include "../common/ruletypes.h"
		{ "_RealRuleCount", RuleManager::_RealRuleCount },
#undef RULE_REAL
#define RULE_BOOL(cat, rule, default_value, notes) \
		{ #rule, RuleManager::Bool__##rule },
#include "../common/ruletypes.h"
		{ "_BoolRuleCount", RuleManager::_BoolRuleCount }
		}
	);
}

void lua_register_rulei(sol::state_view &sv)
{
	sv.create_named_table("RuleI", "Get", &get_rulei);
}

void lua_register_ruler(sol::state_view &sv)
{
	sv.create_named_table("RuleI", "Get", &get_ruler);
}

void lua_register_ruleb(sol::state_view &sv)
{
	sv.create_named_table("RuleB", "Get", &get_ruleb);
}
#endif
