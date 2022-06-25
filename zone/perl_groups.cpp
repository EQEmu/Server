#include "../common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "../common/global_define.h"
#include "embperl.h"

#ifdef seed
#undef seed
#endif

#include "groups.h"

#ifdef THIS /* this macro seems to leak out on some systems */
#undef THIS
#endif

#define VALIDATE_THIS_IS_GROUP \
	do { \
		if (sv_derived_from(ST(0), "Group")) { \
			IV tmp = SvIV((SV*)SvRV(ST(0))); \
			THIS = INT2PTR(Group*, tmp); \
		} else { \
			Perl_croak(aTHX_ "THIS is not of type Group"); \
		} \
		if (THIS == nullptr) { \
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash."); \
		} \
	} while (0);

XS(XS_Group_DisbandGroup); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_DisbandGroup) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Group::DisbandGroup(THIS)"); // @categories Script Utility, Group
	{
		Group *THIS;
		VALIDATE_THIS_IS_GROUP;
		THIS->DisbandGroup();
	}
	XSRETURN_EMPTY;
}

XS(XS_Group_IsGroupMember); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_IsGroupMember) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Group::IsGroupMember(THIS, client)"); // @categories Account and Character, Script Utility, Group
	{
		Group *THIS;
		bool  RETVAL;
		Mob   *client;
		VALIDATE_THIS_IS_GROUP;
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			client = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "client is not of type Mob");
		if (client == nullptr)
			Perl_croak(aTHX_ "client is nullptr, avoiding crash.");

		RETVAL = THIS->IsGroupMember(client);
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Group_CastGroupSpell); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_CastGroupSpell) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: Group::CastGroupSpell(THIS, Mob* caster, uint16 spell_id)"); // @categories Account and Character, Script Utility, Group
	{
		Group  *THIS;
		Mob    *caster;
		uint16 spellid = (uint16) SvUV(ST(2));
		VALIDATE_THIS_IS_GROUP;
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			caster = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "caster is not of type Mob");
		if (caster == nullptr)
			Perl_croak(aTHX_ "caster is nullptr, avoiding crash.");

		THIS->CastGroupSpell(caster, spellid);
	}
	XSRETURN_EMPTY;
}

XS(XS_Group_SplitExp); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_SplitExp) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: Group::SplitExp(THIS, uint32 exp, Mob* other)"); // @categories Account and Character, Script Utility, Group
	{
		Group  *THIS;
		uint32 exp = (uint32) SvUV(ST(1));
		Mob    *other;
		VALIDATE_THIS_IS_GROUP;
		if (sv_derived_from(ST(2), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(2)));
			other = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "other is not of type Mob");
		if (other == nullptr)
			Perl_croak(aTHX_ "other is nullptr, avoiding crash.");

		THIS->SplitExp(exp, other);
	}
	XSRETURN_EMPTY;
}

XS(XS_Group_GroupMessage); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_GroupMessage) {
	dXSARGS;
	if ((items != 3) && (items != 4))    // the 3 item version is kept for backwards compatability
		Perl_croak(aTHX_ "Usage: Group::GroupMessage(THIS, Mob* sender, uint8 language, string message)"); // @categories Script Utility, Group
	{
		Group *THIS;
		Mob   *sender;
		uint8 language;
		char  *message;
		VALIDATE_THIS_IS_GROUP;
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			sender = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "sender is not of type Mob");
		if (sender == nullptr)
			Perl_croak(aTHX_ "sender is nullptr, avoiding crash.");

		if (items == 4) {
			language     = (uint8) SvUV(ST(2));
			if ((language >= MAX_PP_LANGUAGE) || (language < 0))
				language = 0;
			message      = (char *) SvPV_nolen(ST(3));
			THIS->GroupMessage(sender, language, 100, message);
		} else {    // if no language is specificed, send it in common
			message = (char *) SvPV_nolen(ST(2));
			THIS->GroupMessage(sender, 0, 100, message);
		}
	}
	XSRETURN_EMPTY;
}

XS(XS_Group_GetTotalGroupDamage); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_GetTotalGroupDamage) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Group::GetTotalGroupDamage(THIS, Mob* other)"); // @categories Script Utility, Group
	{
		Group  *THIS;
		uint32 RETVAL;
		dXSTARG;
		Mob    *other;
		VALIDATE_THIS_IS_GROUP;
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			other = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "other is not of type Mob");
		if (other == nullptr)
			Perl_croak(aTHX_ "other is nullptr, avoiding crash.");

		RETVAL = THIS->GetTotalGroupDamage(other);
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Group_SplitMoney); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_SplitMoney) {
	dXSARGS;
	if (items != 5)
		Perl_croak(aTHX_ "Usage: Group::SplitMoney(THIS, uint32 copper, uint32 silver, uint32 gold, uint32 platinum)"); // @categories Currency and Points, Script Utility, Group
	{
		Group  *THIS;
		uint32 copper   = (uint32) SvUV(ST(1));
		uint32 silver   = (uint32) SvUV(ST(2));
		uint32 gold     = (uint32) SvUV(ST(3));
		uint32 platinum = (uint32) SvUV(ST(4));
		VALIDATE_THIS_IS_GROUP;
		THIS->SplitMoney(copper, silver, gold, platinum);
	}
	XSRETURN_EMPTY;
}

XS(XS_Group_SetLeader); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_SetLeader) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Group::SetLeader(THIS, Mob* new_leader)"); // @categories Account and Character, Script Utility, Group
	{
		Group *THIS;
		Mob   *newleader;
		VALIDATE_THIS_IS_GROUP;
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			newleader = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "newleader is not of type Mob");
		if (newleader == nullptr)
			Perl_croak(aTHX_ "newleader is nullptr, avoiding crash.");

		THIS->SetLeader(newleader);
	}
	XSRETURN_EMPTY;
}

XS(XS_Group_GetLeader); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_GetLeader) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Group::GetLeader(THIS)"); // @categories Account and Character, Script Utility, Group
	{
		Group *THIS;
		Mob   *RETVAL;
		VALIDATE_THIS_IS_GROUP;
		RETVAL = THIS->GetLeader();
		ST(0) = sv_newmortal();
		sv_setref_pv(ST(0), "Mob", (void *) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Group_GetLeaderName); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_GetLeaderName) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Group::GetLeaderName(THIS)"); // @categories Account and Character, Script Utility, Group
	{
		Group      *THIS;
		const char *RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_GROUP;
		RETVAL = THIS->GetLeaderName();
		sv_setpv(TARG, RETVAL);
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Group_SendHPPacketsTo); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_SendHPPacketsTo) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Group::SendHPPacketsTo(THIS, Mob* new_member)"); // @categories Script Utility, Group
	{
		Group *THIS;
		Mob   *newmember;
		VALIDATE_THIS_IS_GROUP;
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			newmember = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "newmember is not of type Mob");
		if (newmember == nullptr)
			Perl_croak(aTHX_ "newmember is nullptr, avoiding crash.");

		THIS->SendHPManaEndPacketsTo(newmember);
	}
	XSRETURN_EMPTY;
}

XS(XS_Group_SendHPPacketsFrom); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_SendHPPacketsFrom) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Group::SendHPPacketsFrom(THIS, Mob* new_member)"); // @categories Script Utility, Group
	{
		Group *THIS;
		Mob   *newmember;
		VALIDATE_THIS_IS_GROUP;
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			newmember = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "newmember is not of type Mob");
		if (newmember == nullptr)
			Perl_croak(aTHX_ "newmember is nullptr, avoiding crash.");

		THIS->SendHPPacketsFrom(newmember);
	}
	XSRETURN_EMPTY;
}

XS(XS_Group_IsLeader); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_IsLeader) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Group::IsLeader(THIS, Mob* target)"); // @categories Account and Character, Script Utility, Group
	{
		Group *THIS;
		bool  RETVAL;
		Mob   *leadertest;
		VALIDATE_THIS_IS_GROUP;
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			leadertest = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "leadertest is not of type Mob");
		if (leadertest == nullptr)
			Perl_croak(aTHX_ "leadertest is nullptr, avoiding crash.");

		RETVAL = THIS->IsLeader(leadertest);
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Group_GroupCount); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_GroupCount) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Group::GroupCount(THIS)"); // @categories Script Utility, Group
	{
		Group *THIS;
		uint8 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_GROUP;
		RETVAL = THIS->GroupCount();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Group_GetHighestLevel); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_GetHighestLevel) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Group::GetHighestLevel(THIS)"); // @categories Script Utility, Group
	{
		Group  *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_GROUP;
		RETVAL = THIS->GetHighestLevel();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Group_TeleportGroup); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_TeleportGroup) {
	dXSARGS;
	if (items != 7)
		Perl_croak(aTHX_ "Usage: Group::TeleportGroup(THIS, Mob* sender, uint32 zone_id, float x, float y, float z, float heading)"); // @categories Script Utility, Group
	{
		Group  *THIS;
		Mob    *sender;
		uint32 zoneID  = (uint32) SvUV(ST(2));
		float  x       = (float) SvNV(ST(3));
		float  y       = (float) SvNV(ST(4));
		float  z       = (float) SvNV(ST(5));
		float  heading = (float) SvNV(ST(6));
		VALIDATE_THIS_IS_GROUP;
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			sender = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "sender is not of type Mob");
		if (sender == nullptr)
			Perl_croak(aTHX_ "sender is nullptr, avoiding crash.");

		THIS->TeleportGroup(sender, zoneID, 0, x, y, z, heading);
	}
	XSRETURN_EMPTY;
}

XS(XS_Group_GetID); /* prototype to pass -Wmissing-prototypes */
XS(XS_Group_GetID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Group::GetID(THIS)"); // @categories Script Utility, Group
	{
		Group  *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_GROUP;
		RETVAL = THIS->GetID();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Group_GetMember);
XS(XS_Group_GetMember) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Group::GetMember(THIS, int group_index)"); // @categories Account and Character, Script Utility, Group
	{
		Group  *THIS;
		Mob    *member;
		Client *RETVAL = nullptr;
		dXSTARG;
		VALIDATE_THIS_IS_GROUP;
		int index = (int) SvUV(ST(1));
		if (index < 0 || index > 5)
			RETVAL = nullptr;
		else {
			member     = THIS->members[index];
			if (member != nullptr)
				RETVAL = member->CastToClient();
		}

		ST(0) = sv_newmortal();
		sv_setref_pv(ST(0), "Client", (void *) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Group_DoesAnyMemberHaveExpeditionLockout);
XS(XS_Group_DoesAnyMemberHaveExpeditionLockout) {
	dXSARGS;
	if (items != 3 && items != 4) {
		Perl_croak(aTHX_ "Usage: Group::DoesAnyMemberHaveExpeditionLockout(THIS, string expedition_name, string event_name, [int max_check_count = 0])");
	}

	Group* THIS = nullptr;
	VALIDATE_THIS_IS_GROUP;
	std::string expedition_name(SvPV_nolen(ST(1)));
	std::string event_name(SvPV_nolen(ST(2)));
	int max_check_count = (items == 4) ? static_cast<int>(SvIV(ST(3))) : 0;

	bool result = THIS->DoesAnyMemberHaveExpeditionLockout(expedition_name, event_name, max_check_count);
	ST(0) = boolSV(result);
	XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_Group); /* prototype to pass -Wmissing-prototypes */
XS(boot_Group) {
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;

	if (items != 1)
		fprintf(stderr, "boot_quest does not take any arguments.");
	char buf[128];

	//add the strcpy stuff to get rid of const warnings....

	XS_VERSION_BOOTCHECK;
	newXSproto(strcpy(buf, "CastGroupSpell"), XS_Group_CastGroupSpell, file, "$$$");
	newXSproto(strcpy(buf, "DisbandGroup"), XS_Group_DisbandGroup, file, "$");
	newXSproto(strcpy(buf, "DoesAnyMemberHaveExpeditionLockout"), XS_Group_DoesAnyMemberHaveExpeditionLockout, file, "$$$;$");
	newXSproto(strcpy(buf, "GetHighestLevel"), XS_Group_GetHighestLevel, file, "$");
	newXSproto(strcpy(buf, "GetID"), XS_Group_GetID, file, "$");
	newXSproto(strcpy(buf, "GetLeader"), XS_Group_GetLeader, file, "$");
	newXSproto(strcpy(buf, "GetLeaderName"), XS_Group_GetLeaderName, file, "$");
	newXSproto(strcpy(buf, "GetMember"), XS_Group_GetMember, file, "$$");
	newXSproto(strcpy(buf, "GetTotalGroupDamage"), XS_Group_GetTotalGroupDamage, file, "$$");
	newXSproto(strcpy(buf, "GroupCount"), XS_Group_GroupCount, file, "$");
	newXSproto(strcpy(buf, "GroupMessage"), XS_Group_GroupMessage, file, "$$$");
	newXSproto(strcpy(buf, "IsGroupMember"), XS_Group_IsGroupMember, file, "$$");
	newXSproto(strcpy(buf, "IsLeader"), XS_Group_IsLeader, file, "$$");
	newXSproto(strcpy(buf, "SendHPPacketsFrom"), XS_Group_SendHPPacketsFrom, file, "$$");
	newXSproto(strcpy(buf, "SendHPPacketsTo"), XS_Group_SendHPPacketsTo, file, "$$");
	newXSproto(strcpy(buf, "SetLeader"), XS_Group_SetLeader, file, "$$");
	newXSproto(strcpy(buf, "SplitExp"), XS_Group_SplitExp, file, "$$$");
	newXSproto(strcpy(buf, "SplitMoney"), XS_Group_SplitMoney, file, "$$$$$");
	newXSproto(strcpy(buf, "TeleportGroup"), XS_Group_TeleportGroup, file, "$$$$$$$");
	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES

