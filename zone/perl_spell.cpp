#include "../common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "../common/global_define.h"
#include "embperl.h"

#ifdef seed
#undef seed
#endif

#include "../common/spdat.h"

#ifdef THIS /* this macro seems to leak out on some systems */
#undef THIS
#endif

#define VALIDATE_THIS_IS_SPELL \
	do { \
		if (sv_derived_from(ST(0), "Spell")) { \
			IV tmp = SvIV((SV*)SvRV(ST(0))); \
			THIS = INT2PTR(SPDat_Spell_Struct*, tmp); \
		} else { \
			Perl_croak(aTHX_ "THIS is not of type SPDat_Spell_Struct"); \
		} \
		if (THIS == nullptr) { \
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash."); \
		} \
	} while (0);

XS(XS_Spell_GetActivated);
XS(XS_Spell_GetActivated) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetActivated(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int activated;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		activated = THIS->activated;
		XSprePUSH;
		PUSHi((IV) activated);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetAllowRest);
XS(XS_Spell_GetAllowRest) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetAllowRest(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool allow_rest;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		allow_rest = THIS->allow_rest;
		ST(0) = boolSV(allow_rest);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetAOEDuration);
XS(XS_Spell_GetAOEDuration) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetAOEDuration(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint32 aoe_duration;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		aoe_duration = THIS->aoe_duration;
		XSprePUSH;
		PUSHu((UV) aoe_duration);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetAOEMaxTargets);
XS(XS_Spell_GetAOEMaxTargets) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetAOEMaxTargets(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int aoe_max_targets;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		aoe_max_targets = THIS->aoe_max_targets;
		XSprePUSH;
		PUSHi((IV) aoe_max_targets);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetAOERange);
XS(XS_Spell_GetAOERange) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetAOERange(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		float aoe_range;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		aoe_range = THIS->aoe_range;
		XSprePUSH;
		PUSHn((double) aoe_range);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetBaseDifficulty);
XS(XS_Spell_GetBaseDifficulty) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetBaseDifficulty(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int base_difficulty;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		base_difficulty = THIS->base_difficulty;
		XSprePUSH;
		PUSHi((IV) base_difficulty);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetBaseValue);
XS(XS_Spell_GetBaseValue) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::GetBaseValue(THIS, uint8 slot)");
	{
		SPDat_Spell_Struct* THIS;
		int base_value;
		uint8 slot = (uint8) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		base_value = THIS->base_value[slot];
		XSprePUSH;
		PUSHi((IV) base_value);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetBonusHate);
XS(XS_Spell_GetBonusHate) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetBonusHate(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int bonus_hate;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		bonus_hate = THIS->bonus_hate;
		XSprePUSH;
		PUSHi((IV) bonus_hate);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetBuffDuration);
XS(XS_Spell_GetBuffDuration) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetBuffDuration(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint32 buff_duration;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		buff_duration = THIS->buff_duration;
		XSprePUSH;
		PUSHu((UV) buff_duration);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetBuffDurationFormula);
XS(XS_Spell_GetBuffDurationFormula) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetBuffDurationFormula(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint32 buff_duration_formula;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		buff_duration_formula = THIS->buff_duration_formula;
		XSprePUSH;
		PUSHu((UV) buff_duration_formula);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetCanCastInCombat);
XS(XS_Spell_GetCanCastInCombat) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCanCastInCombat(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool can_cast_in_combat;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		can_cast_in_combat = THIS->can_cast_in_combat;
		ST(0) = boolSV(can_cast_in_combat);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetCanCastOutOfCombat);
XS(XS_Spell_GetCanCastOutOfCombat) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCanCastOutOfCombat(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool can_cast_out_of_combat;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		can_cast_out_of_combat = THIS->can_cast_out_of_combat;
		ST(0) = boolSV(can_cast_out_of_combat);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetCanMGB);
XS(XS_Spell_GetCanMGB) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCanMGB(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool can_mgb;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		can_mgb = THIS->can_mgb;
		ST(0) = boolSV(can_mgb);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetCastNotStanding);
XS(XS_Spell_GetCastNotStanding) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCastNotStanding(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool cast_not_standing;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		cast_not_standing = THIS->cast_not_standing;
		ST(0) = boolSV(cast_not_standing);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetCastOnOther);
XS(XS_Spell_GetCastOnOther) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCastOnOther(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		std::string cast_on_other;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		cast_on_other = THIS->cast_on_other;
		sv_setpv(TARG, cast_on_other.c_str());
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Spell_GetCastOnYou);
XS(XS_Spell_GetCastOnYou) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCastOnYou(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		std::string cast_on_you;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		cast_on_you = THIS->cast_on_you;
		sv_setpv(TARG, cast_on_you.c_str());
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Spell_GetCastRestriction);
XS(XS_Spell_GetCastRestriction) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCastRestriction(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int cast_restriction;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		cast_restriction = THIS->cast_restriction;
		XSprePUSH;
		PUSHi((IV) cast_restriction);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetCastTime);
XS(XS_Spell_GetCastTime) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCastTime(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint32 cast_time;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		cast_time = THIS->cast_time;
		XSprePUSH;
		PUSHu((UV) cast_time);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetCasterRequirementID);
XS(XS_Spell_GetCasterRequirementID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCasterRequirementID(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int caster_requirement_id;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		caster_requirement_id = THIS->caster_requirement_id;
		XSprePUSH;
		PUSHi((IV) caster_requirement_id);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetCastingAnimation);
XS(XS_Spell_GetCastingAnimation) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetCastingAnimation(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint8 casting_animation;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		casting_animation = THIS->casting_animation;
		XSprePUSH;
		PUSHu((UV) casting_animation);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetClasses);
XS(XS_Spell_GetClasses) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::GetClasses(THIS, uint8 slot)");
	{
		SPDat_Spell_Struct* THIS;
		uint8 classes;
		uint8 slot = (uint8) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		classes = THIS->classes[slot];
		XSprePUSH;
		PUSHu((UV) classes);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetComponent);
XS(XS_Spell_GetComponent) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::GetComponent(THIS, uint8 slot)");
	{
		SPDat_Spell_Struct* THIS;
		int component;
		uint8 slot = (uint8) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		component = THIS->component[slot];
		XSprePUSH;
		PUSHi((IV) component);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetComponentCount);
XS(XS_Spell_GetComponentCount) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::GetComponentCount(THIS, uint8 slot)");
	{
		SPDat_Spell_Struct* THIS;
		int component_count;
		uint8 slot = (uint8) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		component_count = THIS->component_count[slot];
		XSprePUSH;
		PUSHi((IV) component_count);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetDeities);
XS(XS_Spell_GetDeities) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::GetDeities(THIS, uint8 slot)");
	{
		SPDat_Spell_Struct* THIS;
		int8 deities;
		uint8 slot = (uint8) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		deities = THIS->deities[slot];
		XSprePUSH;
		PUSHi((IV) deities);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetDeityAgnostic);
XS(XS_Spell_GetDeityAgnostic) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetDeityAgnostic(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int8 deity_agnostic;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		deity_agnostic = THIS->deity_agnostic;
		XSprePUSH;
		PUSHi((IV) deity_agnostic);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetDescriptionID);
XS(XS_Spell_GetDescriptionID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetDescriptionID(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int description_id;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		description_id = THIS->description_id;
		XSprePUSH;
		PUSHi((IV) description_id);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetDirectionalEnd);
XS(XS_Spell_GetDirectionalEnd) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetDirectionalEnd(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		float directional_end;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		directional_end = THIS->directional_end;
		XSprePUSH;
		PUSHn((double) directional_end);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetDirectionalStart);
XS(XS_Spell_GetDirectionalStart) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetDirectionalStart(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		float directional_start;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		directional_start = THIS->directional_start;
		XSprePUSH;
		PUSHn((double) directional_start);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetDisallowSit);
XS(XS_Spell_GetDisallowSit) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetDisallowSit(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int8 disallow_sit;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		disallow_sit = THIS->disallow_sit;
		XSprePUSH;
		PUSHi((IV) disallow_sit);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetDispelFlag);
XS(XS_Spell_GetDispelFlag) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetDispelFlag(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int dispel_flag;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		dispel_flag = THIS->dispel_flag;
		XSprePUSH;
		PUSHi((IV) dispel_flag);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetEffectDescriptionID);
XS(XS_Spell_GetEffectDescriptionID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetEffectDescriptionID(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int effect_description_id;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		effect_description_id = THIS->effect_description_id;
		XSprePUSH;
		PUSHi((IV) effect_description_id);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetEffectID);
XS(XS_Spell_GetEffectID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::GetEffectID(THIS, uint8 slot)");
	{
		SPDat_Spell_Struct* THIS;
		int effect_id;
		uint8 slot = (uint8) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		effect_id = THIS->effect_id[slot];
		XSprePUSH;
		PUSHi((IV) effect_id);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetEnduranceCost);
XS(XS_Spell_GetEnduranceCost) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetEnduranceCost(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int endurance_cost;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		endurance_cost = THIS->endurance_cost;
		XSprePUSH;
		PUSHi((IV) endurance_cost);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetEnduranceUpkeep);
XS(XS_Spell_GetEnduranceUpkeep) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetEnduranceUpkeep(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int endurance_upkeep;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		endurance_upkeep = THIS->endurance_upkeep;
		XSprePUSH;
		PUSHi((IV) endurance_upkeep);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetEnvironmentType);
XS(XS_Spell_GetEnvironmentType) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetEnvironmentType(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int8 environment_type;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		environment_type = THIS->environment_type;
		XSprePUSH;
		PUSHi((IV) environment_type);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetFeedbackable);
XS(XS_Spell_GetFeedbackable) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetFeedbackable(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool feedbackable;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		feedbackable = THIS->feedbackable;
		ST(0) = boolSV(feedbackable);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetFormula);
XS(XS_Spell_GetFormula) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::GetFormula(THIS, uint8 slot)");
	{
		SPDat_Spell_Struct* THIS;
		uint16 formula;
		uint8 slot = (uint8) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		formula = THIS->formula[slot];
		XSprePUSH;
		PUSHu((UV) formula);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetGoodEffect);
XS(XS_Spell_GetGoodEffect) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetGoodEffect(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int8 good_effect;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		good_effect = THIS->good_effect;
		XSprePUSH;
		PUSHi((IV) good_effect);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetHateAdded);
XS(XS_Spell_GetHateAdded) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetHateAdded(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int hate_added;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		hate_added = THIS->hate_added;
		XSprePUSH;
		PUSHi((IV) hate_added);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetHitNumber);
XS(XS_Spell_GetHitNumber) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetHitNumber(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int hit_number;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		hit_number = THIS->hit_number;
		XSprePUSH;
		PUSHi((IV) hit_number);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetHitNumberType);
XS(XS_Spell_GetHitNumberType) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetHitNumberType(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int hit_number_type;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		hit_number_type = THIS->hit_number_type;
		XSprePUSH;
		PUSHi((IV) hit_number_type);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetID);
XS(XS_Spell_GetID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetID(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int id;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		id = THIS->id;
		XSprePUSH;
		PUSHi((IV) id);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetIsDiscipline);
XS(XS_Spell_GetIsDiscipline) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetIsDiscipline(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool is_discipline;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		is_discipline = THIS->is_discipline;
		ST(0) = boolSV(is_discipline);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetLDoNTrap);
XS(XS_Spell_GetLDoNTrap) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetLDoNTrap(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool ldon_trap;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		ldon_trap = THIS->ldon_trap;
		ST(0) = boolSV(ldon_trap);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetLimitValue);
XS(XS_Spell_GetLimitValue) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::GetLimitValue(THIS, uint8 slot)");
	{
		SPDat_Spell_Struct* THIS;
		int limit_value;
		uint8 slot = (uint8) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		limit_value = THIS->limit_value[slot];
		XSprePUSH;
		PUSHi((IV) limit_value);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetMana);
XS(XS_Spell_GetMana) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetMana(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint16 mana;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		mana = THIS->mana;
		XSprePUSH;
		PUSHu((UV) mana);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetMaxDistance);
XS(XS_Spell_GetMaxDistance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetMaxDistance(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		float max_distance;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		max_distance = THIS->max_distance;
		XSprePUSH;
		PUSHn((double) max_distance);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetMaxDistanceMod);
XS(XS_Spell_GetMaxDistanceMod) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetMaxDistanceMod(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		float max_distance_mod;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		max_distance_mod = THIS->max_distance_mod;
		XSprePUSH;
		PUSHn((double) max_distance_mod);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetMaxResist);
XS(XS_Spell_GetMaxResist) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetMaxResist(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int max_resist;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		max_resist = THIS->max_resist;
		XSprePUSH;
		PUSHi((IV) max_resist);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetMaxValue);
XS(XS_Spell_GetMaxValue) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::GetMaxValue(THIS, uint8 slot)");
	{
		SPDat_Spell_Struct* THIS;
		int max_value;
		uint8 slot = (uint8) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		max_value = THIS->max_value[slot];
		XSprePUSH;
		PUSHi((IV) max_value);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetMinDistance);
XS(XS_Spell_GetMinDistance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetMinDistance(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		float min_distance;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		min_distance = THIS->min_distance;
		XSprePUSH;
		PUSHn((double) min_distance);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetMinDistanceMod);
XS(XS_Spell_GetMinDistanceMod) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetMinDistanceMod(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		float min_distance_mod;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		min_distance_mod = THIS->min_distance_mod;
		XSprePUSH;
		PUSHn((double) min_distance_mod);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetMinRange);
XS(XS_Spell_GetMinRange) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetMinRange(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		float min_range;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		min_range = THIS->min_range;
		XSprePUSH;
		PUSHn((double) min_range);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetMinResist);
XS(XS_Spell_GetMinResist) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetMinResist(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int min_resist;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		min_resist = THIS->min_resist;
		XSprePUSH;
		PUSHi((IV) min_resist);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetName);
XS(XS_Spell_GetName) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetName(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		std::string name;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		name = THIS->name;
		sv_setpv(TARG, name.c_str());
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Spell_GetNewIcon);
XS(XS_Spell_GetNewIcon) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetNewIcon(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int16 new_icon;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		new_icon = THIS->new_icon;
		XSprePUSH;
		PUSHi((IV) new_icon);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetNimbusEffect);
XS(XS_Spell_GetNimbusEffect) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetNimbusEffect(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int nimbus_effect;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		nimbus_effect = THIS->nimbus_effect;
		XSprePUSH;
		PUSHi((IV) nimbus_effect);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetNoBlock);
XS(XS_Spell_GetNoBlock) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetNoBlock(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool no_block;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		no_block = THIS->no_block;
		ST(0) = boolSV(no_block);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetNoDetrimentalSpellAggro);
XS(XS_Spell_GetNoDetrimentalSpellAggro) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetNoDetrimentalSpellAggro(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool no_detrimental_spell_aggro;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		no_detrimental_spell_aggro = THIS->no_detrimental_spell_aggro;
		ST(0) = boolSV(no_detrimental_spell_aggro);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetNoExpendReagent);
XS(XS_Spell_GetNoExpendReagent) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Spell::GetNoExpendReagent(THIS, uint8 slot)");
	{
		SPDat_Spell_Struct* THIS;
		int no_expend_reagent;
		uint8 slot = (uint8) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		no_expend_reagent = THIS->no_expend_reagent[slot];
		XSprePUSH;
		PUSHi((IV) no_expend_reagent);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetNoHealDamageItemMod);
XS(XS_Spell_GetNoHealDamageItemMod) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetNoHealDamageItemMod(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int no_heal_damage_item_mod;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		no_heal_damage_item_mod = THIS->no_heal_damage_item_mod;
		XSprePUSH;
		PUSHi((IV) no_heal_damage_item_mod);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetNoPartialResist);
XS(XS_Spell_GetNoPartialResist) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetNoPartialResist(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool no_partial_resist;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		no_partial_resist = THIS->no_partial_resist;
		ST(0) = boolSV(no_partial_resist);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetNoRemove);
XS(XS_Spell_GetNoRemove) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetNoRemove(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool no_remove;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		no_remove = THIS->no_remove;
		ST(0) = boolSV(no_remove);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetNoResist);
XS(XS_Spell_GetNoResist) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetNoResist(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int no_resist;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		no_resist = THIS->no_resist;
		XSprePUSH;
		PUSHi((IV) no_resist);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetNotFocusable);
XS(XS_Spell_GetNotFocusable) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetNotFocusable(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool not_focusable;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		not_focusable = THIS->not_focusable;
		ST(0) = boolSV(not_focusable);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetNPCNoLOS);
XS(XS_Spell_GetNPCNoLOS) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetNPCNoLOS(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool npc_no_los;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		npc_no_los = THIS->npc_no_los;
		ST(0) = boolSV(npc_no_los);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetOtherCasts);
XS(XS_Spell_GetOtherCasts) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetOtherCasts(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		std::string other_casts;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		other_casts = THIS->other_casts;
		sv_setpv(TARG, other_casts.c_str());
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Spell_GetOverrideCritChance);
XS(XS_Spell_GetOverrideCritChance) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetOverrideCritChance(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int override_crit_chance;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		override_crit_chance = THIS->override_crit_chance;
		XSprePUSH;
		PUSHi((IV) override_crit_chance);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetPCNPCOnlyFlag);
XS(XS_Spell_GetPCNPCOnlyFlag) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetPCNPCOnlyFlag(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int pcnpc_only_flag;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		pcnpc_only_flag = THIS->pcnpc_only_flag;
		XSprePUSH;
		PUSHi((IV) pcnpc_only_flag);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetPersistDeath);
XS(XS_Spell_GetPersistDeath) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetPersistDeath(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool persist_death;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		persist_death = THIS->persist_death;
		ST(0) = boolSV(persist_death);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetPlayer_1);
XS(XS_Spell_GetPlayer_1) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetPlayer_1(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		std::string player_1;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		player_1 = THIS->player_1;
		sv_setpv(TARG, player_1.c_str());
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Spell_GetPushBack);
XS(XS_Spell_GetPushBack) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetPushBack(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		float push_back;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		push_back = THIS->push_back;
		XSprePUSH;
		PUSHn((double) push_back);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetPushUp);
XS(XS_Spell_GetPushUp) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetPushUp(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		float push_up;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		push_up = THIS->push_up;
		XSprePUSH;
		PUSHn((double) push_up);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetPVPDuration);
XS(XS_Spell_GetPVPDuration) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetPVPDuration(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int pvp_duration;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		pvp_duration = THIS->pvp_duration;
		XSprePUSH;
		PUSHi((IV) pvp_duration);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetPVPDurationCap);
XS(XS_Spell_GetPVPDurationCap) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetPVPDurationCap(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int pvp_duration_cap;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		pvp_duration_cap = THIS->pvp_duration_cap;
		XSprePUSH;
		PUSHi((IV) pvp_duration_cap);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetPVPResistBase);
XS(XS_Spell_GetPVPResistBase) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetPVPResistBase(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int pvp_resist_base;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		pvp_resist_base = THIS->pvp_resist_base;
		XSprePUSH;
		PUSHi((IV) pvp_resist_base);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetPVPResistCap);
XS(XS_Spell_GetPVPResistCap) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetPVPResistCap(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int pvp_resist_cap;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		pvp_resist_cap = THIS->pvp_resist_cap;
		XSprePUSH;
		PUSHi((IV) pvp_resist_cap);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetPVPResistPerLevel);
XS(XS_Spell_GetPVPResistPerLevel) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetPVPResistPerLevel(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int pvp_resist_per_level;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		pvp_resist_per_level = THIS->pvp_resist_per_level;
		XSprePUSH;
		PUSHi((IV) pvp_resist_per_level);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetRange);
XS(XS_Spell_GetRange) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetRange(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		float range;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		range = THIS->range;
		XSprePUSH;
		PUSHn((double) range);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetRank);
XS(XS_Spell_GetRank) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetRank(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int rank;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		rank = THIS->rank;
		XSprePUSH;
		PUSHi((IV) rank);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetRecastTime);
XS(XS_Spell_GetRecastTime) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetRecastTime(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint32 recast_time;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		recast_time = THIS->recast_time;
		XSprePUSH;
		PUSHu((UV) recast_time);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetRecourseLink);
XS(XS_Spell_GetRecourseLink) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetRecourseLink(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint16 recourse_link;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		recourse_link = THIS->recourse_link;
		XSprePUSH;
		PUSHu((UV) recourse_link);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetRecoveryTime);
XS(XS_Spell_GetRecoveryTime) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetRecoveryTime(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint32 recovery_time;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		recovery_time = THIS->recovery_time;
		XSprePUSH;
		PUSHu((UV) recovery_time);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetReflectable);
XS(XS_Spell_GetReflectable) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetReflectable(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool reflectable;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		reflectable = THIS->reflectable;
		ST(0) = boolSV(reflectable);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetResistDifficulty);
XS(XS_Spell_GetResistDifficulty) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetResistDifficulty(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int16 resist_difficulty;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		resist_difficulty = THIS->resist_difficulty;
		XSprePUSH;
		PUSHi((IV) resist_difficulty);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetResistType);
XS(XS_Spell_GetResistType) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetResistType(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int resist_type;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		resist_type = THIS->resist_type;
		XSprePUSH;
		PUSHi((IV) resist_type);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetShortBuffBox);
XS(XS_Spell_GetShortBuffBox) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetShortBuffBox(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int8 short_buff_box;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		short_buff_box = THIS->short_buff_box;
		XSprePUSH;
		PUSHi((IV) short_buff_box);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetSkill);
XS(XS_Spell_GetSkill) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSkill(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		EQ::skills::SkillType skill;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		skill = THIS->skill;
		XSprePUSH;
		PUSHi((IV) skill);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetSneak);
XS(XS_Spell_GetSneak) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSneak(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool sneak;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		sneak = THIS->sneak;
		ST(0) = boolSV(sneak);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetSongCap);
XS(XS_Spell_GetSongCap) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSongCap(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int song_cap;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		song_cap = THIS->song_cap;
		XSprePUSH;
		PUSHi((IV) song_cap);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetSpellAffectIndex);
XS(XS_Spell_GetSpellAffectIndex) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSpellAffectIndex(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint16 spell_affect_index;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		spell_affect_index = THIS->spell_affect_index;
		XSprePUSH;
		PUSHu((UV) spell_affect_index);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetSpellCategory);
XS(XS_Spell_GetSpellCategory) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSpellCategory(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int spell_category;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		spell_category = THIS->spell_category;
		XSprePUSH;
		PUSHi((IV) spell_category);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetSpellClass);
XS(XS_Spell_GetSpellClass) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSpellClass(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int spell_class;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		spell_class = THIS->spell_class;
		XSprePUSH;
		PUSHi((IV) spell_class);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetSpellFades);
XS(XS_Spell_GetSpellFades) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSpellFades(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		std::string spell_fades;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		spell_fades = THIS->spell_fades;
		sv_setpv(TARG, spell_fades.c_str());
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Spell_GetSpellGroup);
XS(XS_Spell_GetSpellGroup) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSpellGroup(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int spell_group;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		spell_group = THIS->spell_group;
		XSprePUSH;
		PUSHi((IV) spell_group);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetSpellSubclass);
XS(XS_Spell_GetSpellSubclass) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSpellSubclass(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int spell_subclass;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		spell_subclass = THIS->spell_subclass;
		XSprePUSH;
		PUSHi((IV) spell_subclass);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetSuspendable);
XS(XS_Spell_GetSuspendable) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetSuspendable(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool suspendable;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		suspendable = THIS->suspendable;
		ST(0) = boolSV(suspendable);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetTargetType);
XS(XS_Spell_GetTargetType) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetTargetType(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		SpellTargetType target_type;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		target_type = THIS->target_type;
		XSprePUSH;
		PUSHi((IV) target_type);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetTeleportZone);
XS(XS_Spell_GetTeleportZone) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetTeleportZone(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		std::string teleport_zone;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		teleport_zone = THIS->teleport_zone;
		sv_setpv(TARG, teleport_zone.c_str());
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Spell_GetTimeOfDay);
XS(XS_Spell_GetTimeOfDay) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetTimeOfDay(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int8 time_of_day;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		time_of_day = THIS->time_of_day;
		XSprePUSH;
		PUSHi((IV) time_of_day);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetTimerID);
XS(XS_Spell_GetTimerID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetTimerID(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int8 timer_id;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		timer_id = THIS->timer_id;
		XSprePUSH;
		PUSHi((IV) timer_id);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetTypeDescriptionID);
XS(XS_Spell_GetTypeDescriptionID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetTypeDescriptionID(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int type_description_id;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		type_description_id = THIS->type_description_id;
		XSprePUSH;
		PUSHi((IV) type_description_id);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetUninterruptable);
XS(XS_Spell_GetUninterruptable) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetUninterruptable(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool uninterruptable;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		uninterruptable = THIS->uninterruptable;
		ST(0) = boolSV(uninterruptable);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetUnstackableDOT);
XS(XS_Spell_GetUnstackableDOT) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetUnstackableDOT(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		bool unstackable_dot;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		unstackable_dot = THIS->unstackable_dot;
		ST(0) = boolSV(unstackable_dot);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Spell_GetViralRange);
XS(XS_Spell_GetViralRange) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetViralRange(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int viral_range;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		viral_range = THIS->viral_range;
		XSprePUSH;
		PUSHi((IV) viral_range);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetViralTargets);
XS(XS_Spell_GetViralTargets) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetViralTargets(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint8 viral_targets;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		viral_targets = THIS->viral_targets;
		XSprePUSH;
		PUSHu((UV) viral_targets);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetViralTimer);
XS(XS_Spell_GetViralTimer) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetViralTimer(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		uint8 viral_timer;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		viral_timer = THIS->viral_timer;
		XSprePUSH;
		PUSHu((UV) viral_timer);
	}
	XSRETURN(1);
}

XS(XS_Spell_GetYouCast);
XS(XS_Spell_GetYouCast) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetYouCast(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		std::string you_cast;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		you_cast = THIS->you_cast;
		sv_setpv(TARG, you_cast.c_str());
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Spell_GetZoneType);
XS(XS_Spell_GetZoneType) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Spell::GetZoneType(THIS)");
	{
		SPDat_Spell_Struct* THIS;
		int8 zone_type;
		dXSTARG;
		VALIDATE_THIS_IS_SPELL;

		zone_type = THIS->zone_type;
		XSprePUSH;
		PUSHi((IV) zone_type);
	}
	XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif

XS(boot_Spell);
XS(boot_Spell) {
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;
	if (items != 1)
		fprintf(stderr, "boot_Spell does not take any arguments.");
	
	char buf[128];
	XS_VERSION_BOOTCHECK;
	newXSproto(strcpy(buf, "GetActivated"), XS_Spell_GetActivated, file, "$");
	newXSproto(strcpy(buf, "GetAllowRest"), XS_Spell_GetAllowRest, file, "$");
	newXSproto(strcpy(buf, "GetAOEDuration"), XS_Spell_GetAOEDuration, file, "$");
	newXSproto(strcpy(buf, "GetAOEMaxTargets"), XS_Spell_GetAOEMaxTargets, file, "$");
	newXSproto(strcpy(buf, "GetAOERange"), XS_Spell_GetAOERange, file, "$");
	newXSproto(strcpy(buf, "GetBaseDifficulty"), XS_Spell_GetBaseDifficulty, file, "$");
	newXSproto(strcpy(buf, "GetBaseValue"), XS_Spell_GetBaseValue, file, "$$");
	newXSproto(strcpy(buf, "GetBonusHate"), XS_Spell_GetBonusHate, file, "$");
	newXSproto(strcpy(buf, "GetBuffDuration"), XS_Spell_GetBuffDuration, file, "$");
	newXSproto(strcpy(buf, "GetBuffDurationFormula"), XS_Spell_GetBuffDurationFormula, file, "$");
	newXSproto(strcpy(buf, "GetCanCastInCombat"), XS_Spell_GetCanCastInCombat, file, "$");
	newXSproto(strcpy(buf, "GetCanCastOutOfCombat"), XS_Spell_GetCanCastOutOfCombat, file, "$");
	newXSproto(strcpy(buf, "GetCanMGB"), XS_Spell_GetCanMGB, file, "$");
	newXSproto(strcpy(buf, "GetCastNotStanding"), XS_Spell_GetCastNotStanding, file, "$");
	newXSproto(strcpy(buf, "GetCastOnOther"), XS_Spell_GetCastOnOther, file, "$");
	newXSproto(strcpy(buf, "GetCastOnYou"), XS_Spell_GetCastOnYou, file, "$");
	newXSproto(strcpy(buf, "GetCastRestriction"), XS_Spell_GetCastRestriction, file, "$");
	newXSproto(strcpy(buf, "GetCastTime"), XS_Spell_GetCastTime, file, "$");
	newXSproto(strcpy(buf, "GetCasterRequirementID"), XS_Spell_GetCasterRequirementID, file, "$");
	newXSproto(strcpy(buf, "GetCastingAnimation"), XS_Spell_GetCastingAnimation, file, "$");
	newXSproto(strcpy(buf, "GetClasses"), XS_Spell_GetClasses, file, "$$");
	newXSproto(strcpy(buf, "GetComponent"), XS_Spell_GetComponent, file, "$$");
	newXSproto(strcpy(buf, "GetComponentCount"), XS_Spell_GetComponentCount, file, "$$");
	newXSproto(strcpy(buf, "GetDeities"), XS_Spell_GetDeities, file, "$$");
	newXSproto(strcpy(buf, "GetDeityAgnostic"), XS_Spell_GetDeityAgnostic, file, "$");
	newXSproto(strcpy(buf, "GetDescriptionID"), XS_Spell_GetDescriptionID, file, "$");
	newXSproto(strcpy(buf, "GetDirectionalEnd"), XS_Spell_GetDirectionalEnd, file, "$");
	newXSproto(strcpy(buf, "GetDirectionalStart"), XS_Spell_GetDirectionalStart, file, "$");
	newXSproto(strcpy(buf, "GetDisallowSit"), XS_Spell_GetDisallowSit, file, "$");
	newXSproto(strcpy(buf, "GetDispelFlag"), XS_Spell_GetDispelFlag, file, "$");
	newXSproto(strcpy(buf, "GetEffectDescriptionID"), XS_Spell_GetEffectDescriptionID, file, "$");
	newXSproto(strcpy(buf, "GetEffectID"), XS_Spell_GetEffectID, file, "$$");
	newXSproto(strcpy(buf, "GetEnduranceCost"), XS_Spell_GetEnduranceCost, file, "$");
	newXSproto(strcpy(buf, "GetEnduranceUpkeep"), XS_Spell_GetEnduranceUpkeep, file, "$");
	newXSproto(strcpy(buf, "GetEnvironmentType"), XS_Spell_GetEnvironmentType, file, "$");
	newXSproto(strcpy(buf, "GetFeedbackable"), XS_Spell_GetFeedbackable, file, "$");
	newXSproto(strcpy(buf, "GetFormula"), XS_Spell_GetFormula, file, "$$");
	newXSproto(strcpy(buf, "GetGoodEffect"), XS_Spell_GetGoodEffect, file, "$");
	newXSproto(strcpy(buf, "GetHateAdded"), XS_Spell_GetHateAdded, file, "$");
	newXSproto(strcpy(buf, "GetHitNumber"), XS_Spell_GetHitNumber, file, "$");
	newXSproto(strcpy(buf, "GetHitNumberType"), XS_Spell_GetHitNumberType, file, "$");
	newXSproto(strcpy(buf, "GetID"), XS_Spell_GetID, file, "$");
	newXSproto(strcpy(buf, "GetIsDiscipline"), XS_Spell_GetIsDiscipline, file, "$");
	newXSproto(strcpy(buf, "GetLDoNTrap"), XS_Spell_GetLDoNTrap, file, "$");
	newXSproto(strcpy(buf, "GetLimitValue"), XS_Spell_GetLimitValue, file, "$$");
	newXSproto(strcpy(buf, "GetMana"), XS_Spell_GetMana, file, "$");
	newXSproto(strcpy(buf, "GetMaxDistance"), XS_Spell_GetMaxDistance, file, "$");
	newXSproto(strcpy(buf, "GetMaxDistanceMod"), XS_Spell_GetMaxDistanceMod, file, "$");
	newXSproto(strcpy(buf, "GetMaxResist"), XS_Spell_GetMaxResist, file, "$");
	newXSproto(strcpy(buf, "GetMaxValue"), XS_Spell_GetMaxValue, file, "$$");
	newXSproto(strcpy(buf, "GetMinDistance"), XS_Spell_GetMinDistance, file, "$");
	newXSproto(strcpy(buf, "GetMinDistanceMod"), XS_Spell_GetMinDistanceMod, file, "$");
	newXSproto(strcpy(buf, "GetMinRange"), XS_Spell_GetMinRange, file, "$");
	newXSproto(strcpy(buf, "GetMinResist"), XS_Spell_GetMinResist, file, "$");
	newXSproto(strcpy(buf, "GetName"), XS_Spell_GetName, file, "$");
	newXSproto(strcpy(buf, "GetNewIcon"), XS_Spell_GetNewIcon, file, "$");
	newXSproto(strcpy(buf, "GetNimbusEffect"), XS_Spell_GetNimbusEffect, file, "$");
	newXSproto(strcpy(buf, "GetNoBlock"), XS_Spell_GetNoBlock, file, "$");
	newXSproto(strcpy(buf, "GetNoDetrimentalSpellAggro"), XS_Spell_GetNoDetrimentalSpellAggro, file, "$");
	newXSproto(strcpy(buf, "GetNoExpendReagent"), XS_Spell_GetNoExpendReagent, file, "$$");
	newXSproto(strcpy(buf, "GetNoHealDamageItemMod"), XS_Spell_GetNoHealDamageItemMod, file, "$");
	newXSproto(strcpy(buf, "GetNoPartialResist"), XS_Spell_GetNoPartialResist, file, "$");
	newXSproto(strcpy(buf, "GetNoRemove"), XS_Spell_GetNoRemove, file, "$");
	newXSproto(strcpy(buf, "GetNoResist"), XS_Spell_GetNoResist, file, "$");
	newXSproto(strcpy(buf, "GetNotFocusable"), XS_Spell_GetNotFocusable, file, "$");
	newXSproto(strcpy(buf, "GetNPCNoLOS"), XS_Spell_GetNPCNoLOS, file, "$");
	newXSproto(strcpy(buf, "GetOtherCasts"), XS_Spell_GetOtherCasts, file, "$");
	newXSproto(strcpy(buf, "GetOverrideCritChance"), XS_Spell_GetOverrideCritChance, file, "$");
	newXSproto(strcpy(buf, "GetPCNPCOnlyFlag"), XS_Spell_GetPCNPCOnlyFlag, file, "$");
	newXSproto(strcpy(buf, "GetPersistDeath"), XS_Spell_GetPersistDeath, file, "$");
	newXSproto(strcpy(buf, "GetPlayer_1"), XS_Spell_GetPlayer_1, file, "$");
	newXSproto(strcpy(buf, "GetPushBack"), XS_Spell_GetPushBack, file, "$");
	newXSproto(strcpy(buf, "GetPushUp"), XS_Spell_GetPushUp, file, "$");
	newXSproto(strcpy(buf, "GetPVPDuration"), XS_Spell_GetPVPDuration, file, "$");
	newXSproto(strcpy(buf, "GetPVPDurationCap"), XS_Spell_GetPVPDurationCap, file, "$");
	newXSproto(strcpy(buf, "GetPVPResistBase"), XS_Spell_GetPVPResistBase, file, "$");
	newXSproto(strcpy(buf, "GetPVPResistCap"), XS_Spell_GetPVPResistCap, file, "$");
	newXSproto(strcpy(buf, "GetPVPResistPerLevel"), XS_Spell_GetPVPResistPerLevel, file, "$");
	newXSproto(strcpy(buf, "GetRange"), XS_Spell_GetRange, file, "$");
	newXSproto(strcpy(buf, "GetRank"), XS_Spell_GetRank, file, "$");
	newXSproto(strcpy(buf, "GetRecastTime"), XS_Spell_GetRecastTime, file, "$");
	newXSproto(strcpy(buf, "GetRecourseLink"), XS_Spell_GetRecourseLink, file, "$");
	newXSproto(strcpy(buf, "GetRecoveryTime"), XS_Spell_GetRecoveryTime, file, "$");
	newXSproto(strcpy(buf, "GetReflectable"), XS_Spell_GetReflectable, file, "$");
	newXSproto(strcpy(buf, "GetResistDifficulty"), XS_Spell_GetResistDifficulty, file, "$");
	newXSproto(strcpy(buf, "GetResistType"), XS_Spell_GetResistType, file, "$");
	newXSproto(strcpy(buf, "GetShortBuffBox"), XS_Spell_GetShortBuffBox, file, "$");
	newXSproto(strcpy(buf, "GetSkill"), XS_Spell_GetSkill, file, "$");
	newXSproto(strcpy(buf, "GetSneak"), XS_Spell_GetSneak, file, "$");
	newXSproto(strcpy(buf, "GetSongCap"), XS_Spell_GetSongCap, file, "$");
	newXSproto(strcpy(buf, "GetSpellAffectIndex"), XS_Spell_GetSpellAffectIndex, file, "$");
	newXSproto(strcpy(buf, "GetSpellCategory"), XS_Spell_GetSpellCategory, file, "$");
	newXSproto(strcpy(buf, "GetSpellClass"), XS_Spell_GetSpellClass, file, "$");
	newXSproto(strcpy(buf, "GetSpellFades"), XS_Spell_GetSpellFades, file, "$");
	newXSproto(strcpy(buf, "GetSpellGroup"), XS_Spell_GetSpellGroup, file, "$");
	newXSproto(strcpy(buf, "GetSpellSubclass"), XS_Spell_GetSpellSubclass, file, "$");
	newXSproto(strcpy(buf, "GetSuspendable"), XS_Spell_GetSuspendable, file, "$");
	newXSproto(strcpy(buf, "GetTargetType"), XS_Spell_GetTargetType, file, "$");
	newXSproto(strcpy(buf, "GetTeleportZone"), XS_Spell_GetTeleportZone, file, "$");
	newXSproto(strcpy(buf, "GetTimeOfDay"), XS_Spell_GetTimeOfDay, file, "$");
	newXSproto(strcpy(buf, "GetTimerID"), XS_Spell_GetTimerID, file, "$");
	newXSproto(strcpy(buf, "GetTypeDescriptionID"), XS_Spell_GetTypeDescriptionID, file, "$");
	newXSproto(strcpy(buf, "GetUninterruptable"), XS_Spell_GetUninterruptable, file, "$");
	newXSproto(strcpy(buf, "GetUnstackableDOT"), XS_Spell_GetUnstackableDOT, file, "$");
	newXSproto(strcpy(buf, "GetViralRange"), XS_Spell_GetViralRange, file, "$");
	newXSproto(strcpy(buf, "GetViralTargets"), XS_Spell_GetViralTargets, file, "$");
	newXSproto(strcpy(buf, "GetViralTimer"), XS_Spell_GetViralTimer, file, "$");
	newXSproto(strcpy(buf, "GetYouCast"), XS_Spell_GetYouCast, file, "$");
	newXSproto(strcpy(buf, "GetZoneType"), XS_Spell_GetZoneType, file, "$");
	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES
