#include "../common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "../common/global_define.h"
#include "embperl.h"

#ifdef seed
#undef seed
#endif

#include "corpse.h"

#ifdef THIS /* this macro seems to leak out on some systems */
#undef THIS
#endif

#define VALIDATE_THIS_IS_CORPSE \
	do { \
		if (sv_derived_from(ST(0), "Corpse")) { \
			IV tmp = SvIV((SV*)SvRV(ST(0))); \
			THIS = INT2PTR(Corpse*, tmp); \
		} else { \
			Perl_croak(aTHX_ "THIS is not of type Corpse"); \
		} \
		if (THIS == nullptr) { \
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash."); \
		} \
	} while (0);

XS(XS_Corpse_GetCharID); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_GetCharID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::GetCharID(THIS)"); // @categories Account and Character, Corpse
	{
		Corpse *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->GetCharID();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Corpse_GetDecayTime); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_GetDecayTime) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::GetDecayTime(THIS)"); // @categories Script Utility, Corpse
	{
		Corpse *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->GetDecayTime();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Corpse_Lock); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_Lock) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::Lock(THIS)"); // @categories Corpse
	{
		Corpse *THIS;
		VALIDATE_THIS_IS_CORPSE;
		THIS->Lock();
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_UnLock); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_UnLock) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::UnLock(THIS)"); // @categories Corpse
	{
		Corpse *THIS;
		VALIDATE_THIS_IS_CORPSE;
		THIS->UnLock();
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_IsLocked); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_IsLocked) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::IsLocked(THIS)"); // @categories Corpse
	{
		Corpse *THIS;
		bool RETVAL;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->IsLocked();
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Corpse_ResetLooter); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_ResetLooter) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::ResetLooter(THIS)"); // @categories Corpse
	{
		Corpse *THIS;
		VALIDATE_THIS_IS_CORPSE;
		THIS->ResetLooter();
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_GetDBID); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_GetDBID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::GetDBID(THIS)"); // @categories Script Utility, Corpse
	{
		Corpse *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->GetCorpseDBID();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Corpse_GetOwnerName); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_GetOwnerName) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::GetOwnerName(THIS)"); // @categories Account and Character, Corpse
	{
		Corpse *THIS;
		char   *RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->GetOwnerName();
		sv_setpv(TARG, RETVAL);
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Corpse_SetDecayTimer); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_SetDecayTimer) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Corpse::SetDecayTimer(THIS, uint32 decay_time)"); // @categories Corpse
	{
		Corpse *THIS;
		uint32 decaytime = (uint32) SvUV(ST(1));
		VALIDATE_THIS_IS_CORPSE;
		THIS->SetDecayTimer(decaytime);
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_IsEmpty); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_IsEmpty) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::IsEmpty(THIS)"); // @categories Inventory and Items, Corpse
	{
		Corpse *THIS;
		bool RETVAL;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->IsEmpty();
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Corpse_AddItem); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_AddItem) {
	dXSARGS;
	if (items < 3 || items > 4)
		Perl_croak(aTHX_ "Usage: Corpse::AddItem(THIS, uint32 item_id, uint16 charges, [unt16 slot = 0])"); // @categories Inventory and Items, Corpse
	{
		Corpse *THIS;
		uint32 itemnum = (uint32) SvUV(ST(1));
		uint16 charges = (uint16) SvUV(ST(2));
		int16  slot;
		VALIDATE_THIS_IS_CORPSE;
		if (items < 4)
			slot = 0;
		else {
			slot = (int16) SvIV(ST(3));
		}

		THIS->AddItem(itemnum, charges, slot);
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_GetWornItem); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_GetWornItem) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Corpse::GetWornItem(THIS, equipSlot)"); // @categories Inventory and Items, Corpse
	{
		Corpse *THIS;
		uint32 RETVAL;
		dXSTARG;
		int16  equipSlot = (int16) SvIV(ST(1));
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->GetWornItem(equipSlot);
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Corpse_RemoveItem); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_RemoveItem) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Corpse::RemoveItem(THIS, uint16 loot_slot)"); // @categories Inventory and Items, Corpse
	{
		Corpse *THIS;
		uint16 lootslot = (uint16) SvUV(ST(1));
		VALIDATE_THIS_IS_CORPSE;
		THIS->RemoveItem(lootslot);
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_SetCash); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_SetCash) {
	dXSARGS;
	if (items != 5)
		Perl_croak(aTHX_ "Usage: Corpse::SetCash(THIS, uint16 copper, uint16 silver, uint16 gold, uint16 platinum)"); // @categories Currency and Points, Corpse
	{
		Corpse *THIS;
		uint16 in_copper   = (uint16) SvUV(ST(1));
		uint16 in_silver   = (uint16) SvUV(ST(2));
		uint16 in_gold     = (uint16) SvUV(ST(3));
		uint16 in_platinum = (uint16) SvUV(ST(4));
		VALIDATE_THIS_IS_CORPSE;
		THIS->SetCash(in_copper, in_silver, in_gold, in_platinum);
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_RemoveCash); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_RemoveCash) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::RemoveCash(THIS)"); // @categories Currency and Points, Corpse
	{
		Corpse *THIS;
		VALIDATE_THIS_IS_CORPSE;
		THIS->RemoveCash();
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_CountItems); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_CountItems) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::CountItems(THIS)"); // @categories Inventory and Items, Corpse
	{
		Corpse *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->CountItems();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Corpse_Delete); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_Delete) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::Delete(THIS)"); // @categories Corpse
	{
		Corpse *THIS;
		VALIDATE_THIS_IS_CORPSE;
		THIS->Delete();
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_GetCopper); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_GetCopper) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::GetCopper(THIS)"); // @categories Currency and Points, Corpse
	{
		Corpse *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->GetCopper();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Corpse_GetSilver); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_GetSilver) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::GetSilver(THIS)"); // @categories Currency and Points, Corpse
	{
		Corpse *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->GetSilver();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Corpse_GetGold); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_GetGold) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::GetGold(THIS)"); // @categories Currency and Points, Corpse
	{
		Corpse *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->GetGold();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Corpse_GetPlatinum); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_GetPlatinum) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::GetPlatinum(THIS)"); // @categories Currency and Points, Corpse
	{
		Corpse *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->GetPlatinum();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Corpse_Summon); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_Summon) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: Corpse::Summon(THIS, Client* client, bool is_spell)"); // @categories Corpse
	{
		Corpse *THIS;
		Client *client;
		bool spell = (bool) SvTRUE(ST(2));
		VALIDATE_THIS_IS_CORPSE;
		if (sv_derived_from(ST(1), "Client")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			client = INT2PTR(Client *, tmp);
		} else
			Perl_croak(aTHX_ "client is not of type Client");
		if (client == nullptr)
			Perl_croak(aTHX_ "client is nullptr, avoiding crash.");

		THIS->Summon(client, spell, true);
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_CastRezz); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_CastRezz) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: Corpse::CastRezz(THIS, uint16 spell_id, [Mob* caster = nullptr])"); // @categories Spells and Disciplines, Corpse
	{
		Corpse *THIS;
		uint16 spellid = (uint16) SvUV(ST(1));
		Mob *Caster;
		VALIDATE_THIS_IS_CORPSE;
		if (sv_derived_from(ST(2), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(2)));
			Caster = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "Caster is not of type Mob");
		if (Caster == nullptr)
			Perl_croak(aTHX_ "Caster is nullptr, avoiding crash.");

		THIS->CastRezz(spellid, Caster);
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_CompleteRezz); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_CompleteRezz) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::CompleteRezz(THIS)"); // @categories Spells and Disciplines, Corpse
	{
		Corpse *THIS;
		VALIDATE_THIS_IS_CORPSE;
		THIS->CompleteResurrection();
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_CanMobLoot); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_CanMobLoot) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Corpse::CanMobLoot(THIS, int character_id)"); // @categories Script Utility, Corpse
	{
		Corpse *THIS;
		bool RETVAL;
		int  charid = (int) SvIV(ST(1));
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->CanPlayerLoot(charid);
		ST(0)       = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Corpse_AllowMobLoot); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_AllowMobLoot) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: Corpse::AllowMobLoot(THIS, Mob* them, uint8 slot)"); // @categories Account and Character, Corpse
	{
		Corpse *THIS;
		Mob    *them;
		uint8 slot = (uint8) SvUV(ST(2));
		VALIDATE_THIS_IS_CORPSE;
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			them = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "them is not of type Mob");
		if (them == nullptr)
			Perl_croak(aTHX_ "them is nullptr, avoiding crash.");

		THIS->AllowPlayerLoot(them, slot);
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_AddLooter); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_AddLooter) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Corpse::AddLooter(THIS, Mob* who)"); // @categories Account and Character, Corpse
	{
		Corpse *THIS;
		Mob    *who;
		VALIDATE_THIS_IS_CORPSE;
		if (sv_derived_from(ST(1), "Mob")) {
			IV tmp = SvIV((SV *) SvRV(ST(1)));
			who = INT2PTR(Mob *, tmp);
		} else
			Perl_croak(aTHX_ "who is not of type Mob");
		if (who == nullptr)
			Perl_croak(aTHX_ "who is nullptr, avoiding crash.");

		THIS->AddLooter(who);
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_IsRezzed); /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_IsRezzed) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::IsRezzed(THIS)"); // @categories Corpse
	{
		Corpse *THIS;
		bool RETVAL;
		VALIDATE_THIS_IS_CORPSE;
		RETVAL = THIS->IsRezzed();
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Corpse_HasItem);  /* prototype to pass -Wmissing-prototypes */
XS(XS_Corpse_HasItem) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Corpse::HasItem(THIS, uint32 item_id)"); // @categories Script Utility
	{
		Corpse *THIS;
		bool has_item = false;
		uint32 item_id = (uint32) SvUV(ST(1));
		VALIDATE_THIS_IS_CORPSE;
		has_item = THIS->HasItem(item_id);
		ST(0) = boolSV(has_item);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Corpse_CountItem);
XS(XS_Corpse_CountItem) {	
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Corpse::CountItem(THIS, uint32 item_id)"); // @categories Script Utility
	{
		Corpse *THIS;
		uint16 item_count = 0;
		uint32 item_id = (uint32) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		item_count = THIS->CountItem(item_id);
		XSprePUSH;
		PUSHu((UV) item_count);
	}
	XSRETURN(1);
}

XS(XS_Corpse_GetItemIDBySlot);
XS(XS_Corpse_GetItemIDBySlot) {	
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Corpse::GetItemIDBySlot(THIS, uint16 loot_slot)"); // @categories Script Utility
	{
		Corpse *THIS;
		uint32 item_id = 0;
		uint16 loot_slot = (uint16) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		item_id = THIS->GetItemIDBySlot(loot_slot);
		XSprePUSH;
		PUSHu((UV) item_id);
	}
	XSRETURN(1);
}

XS(XS_Corpse_GetFirstSlotByItemID);
XS(XS_Corpse_GetFirstSlotByItemID) {	
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Corpse::GetFirstSlotByItemID(THIS, uint32 item_id)"); // @categories Script Utility
	{
		Corpse *THIS;
		uint16 loot_slot = 0;
		uint32 item_id = (uint32) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_CORPSE;
		loot_slot = THIS->GetFirstSlotByItemID(item_id);
		XSprePUSH;
		PUSHu((UV) loot_slot);
	}
	XSRETURN(1);
}

XS(XS_Corpse_RemoveItemByID);
XS(XS_Corpse_RemoveItemByID) {
	dXSARGS;
	if (items != 2 && items != 3)
		Perl_croak(aTHX_ "Usage: Corpse::RemoveItemByID(THIS, uint32 item_id, [int quantity = 1])"); // @categories Script Utility
	{
		Corpse *THIS;
		uint32 item_id = (uint32) SvUV(ST(1));
		int quantity = 1;
		VALIDATE_THIS_IS_CORPSE;
		if (items == 3)
			quantity = (int) SvIV(ST(2));

		THIS->RemoveItemByID(item_id, quantity);
	}
	XSRETURN_EMPTY;
}

XS(XS_Corpse_GetLootList);
XS(XS_Corpse_GetLootList) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Corpse::GetLootList(THIS)"); // @categories Script Utility
	{
		Corpse *THIS;
		VALIDATE_THIS_IS_CORPSE;
		auto corpse_items = THIS->GetLootList();
		auto item_count = corpse_items.size();
		if (item_count > 0) {
			EXTEND(sp, item_count);
			for (int index = 0; index < item_count; ++index) {
				ST(index) = sv_2mortal(newSVuv(corpse_items[index]));
			}
			XSRETURN(item_count);
		}
		SV* return_value = &PL_sv_undef;
		ST(0) = return_value;
		XSRETURN(1);
	}
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_Corpse); /* prototype to pass -Wmissing-prototypes */
XS(boot_Corpse) {
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;

	if (items != 1)
		fprintf(stderr, "boot_quest does not take any arguments.");
	char buf[128];

	//add the strcpy stuff to get rid of const warnings....

	XS_VERSION_BOOTCHECK;
	newXSproto(strcpy(buf, "AddItem"), XS_Corpse_AddItem, file, "$$$;$");
	newXSproto(strcpy(buf, "AddLooter"), XS_Corpse_AddLooter, file, "$$");
	newXSproto(strcpy(buf, "AllowMobLoot"), XS_Corpse_AllowMobLoot, file, "$$$");
	newXSproto(strcpy(buf, "CanMobLoot"), XS_Corpse_CanMobLoot, file, "$$");
	newXSproto(strcpy(buf, "CastRezz"), XS_Corpse_CastRezz, file, "$$$");
	newXSproto(strcpy(buf, "CompleteRezz"), XS_Corpse_CompleteRezz, file, "$");
	newXSproto(strcpy(buf, "CountItem"), XS_Corpse_CountItem, file, "$$");
	newXSproto(strcpy(buf, "CountItems"), XS_Corpse_CountItems, file, "$");
	newXSproto(strcpy(buf, "Delete"), XS_Corpse_Delete, file, "$");
	newXSproto(strcpy(buf, "GetCharID"), XS_Corpse_GetCharID, file, "$");
	newXSproto(strcpy(buf, "GetCopper"), XS_Corpse_GetCopper, file, "$");
	newXSproto(strcpy(buf, "GetDBID"), XS_Corpse_GetDBID, file, "$");
	newXSproto(strcpy(buf, "GetDecayTime"), XS_Corpse_GetDecayTime, file, "$");
	newXSproto(strcpy(buf, "GetFirstSlotByItemID"), XS_Corpse_GetFirstSlotByItemID, file, "$$");
	newXSproto(strcpy(buf, "GetGold"), XS_Corpse_GetGold, file, "$");
	newXSproto(strcpy(buf, "GetItemIDBySlot"), XS_Corpse_GetItemIDBySlot, file, "$$");
	newXSproto(strcpy(buf, "GetLootList"), XS_Corpse_GetLootList, file, "$");
	newXSproto(strcpy(buf, "GetOwnerName"), XS_Corpse_GetOwnerName, file, "$");
	newXSproto(strcpy(buf, "GetPlatinum"), XS_Corpse_GetPlatinum, file, "$");
	newXSproto(strcpy(buf, "GetSilver"), XS_Corpse_GetSilver, file, "$");
	newXSproto(strcpy(buf, "GetWornItem"), XS_Corpse_GetWornItem, file, "$$");
	newXSproto(strcpy(buf, "HasItem"), XS_Corpse_HasItem, file, "$$");
	newXSproto(strcpy(buf, "IsEmpty"), XS_Corpse_IsEmpty, file, "$");
	newXSproto(strcpy(buf, "IsLocked"), XS_Corpse_IsLocked, file, "$");
	newXSproto(strcpy(buf, "IsRezzed"), XS_Corpse_IsRezzed, file, "$");
	newXSproto(strcpy(buf, "Lock"), XS_Corpse_Lock, file, "$");
	newXSproto(strcpy(buf, "RemoveCash"), XS_Corpse_RemoveCash, file, "$");
	newXSproto(strcpy(buf, "RemoveItem"), XS_Corpse_RemoveItem, file, "$$");
	newXSproto(strcpy(buf, "RemoveItemByID"), XS_Corpse_RemoveItemByID, file, "$$;$");
	newXSproto(strcpy(buf, "ResetLooter"), XS_Corpse_ResetLooter, file, "$");
	newXSproto(strcpy(buf, "SetCash"), XS_Corpse_SetCash, file, "$$$$$");
	newXSproto(strcpy(buf, "SetDecayTimer"), XS_Corpse_SetDecayTimer, file, "$$");
	newXSproto(strcpy(buf, "Summon"), XS_Corpse_Summon, file, "$$$");
	newXSproto(strcpy(buf, "UnLock"), XS_Corpse_UnLock, file, "$");
	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES

