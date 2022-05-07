#ifdef BOTS
#include "../common/features.h"
#ifdef EMBPERL_XS_CLASSES
#include "../common/global_define.h"
#include "embperl.h"

#ifdef seed
#undef seed
#endif

#include "bot.h"

#ifdef THIS	
#undef THIS
#endif

#define VALIDATE_THIS_IS_BOT \
	do { \
		if (sv_derived_from(ST(0), "Bot")) { \
			IV tmp = SvIV((SV*)SvRV(ST(0))); \
			THIS = INT2PTR(Bot*, tmp); \
		} else { \
			Perl_croak(aTHX_ "THIS is not of type Bot"); \
		} \
		if (THIS == nullptr) { \
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash."); \
		} \
	} while (0);

XS(XS_Bot_GetOwner);
XS(XS_Bot_GetOwner)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Bot::GetOwner(THIS)"); // @categories Script Utility, Bot
	{
		Bot* THIS;
		Mob* bot_owner;
        VALIDATE_THIS_IS_BOT;
		bot_owner = THIS->GetBotOwner();
		ST(0) = sv_newmortal();
		sv_setref_pv(ST(0), "Mob", (void*)bot_owner);
	}
	XSRETURN(1);
}

XS(XS_Bot_AddBotItem); /* prototype to pass -Wmissing-prototypes */
XS(XS_Bot_AddBotItem) {
	dXSARGS;
	if (items < 3 || items > 11)
		Perl_croak(aTHX_ "Usage: Bot::AddBotItem(THIS, uint16 slot_id, uint32 item_id, [int16 charges = -1], [bool attuned = false], [uint32 augment_one = 0], [uint32 augment_two = 0], [uint32 augment_three = 0], [uint32 augment_four = 0], [uint32 augment_five = 0], [uint32 augment_six = 0])"); // @categories Inventory and Items, Script Utility
	{
		Bot* THIS;
		uint16 slot_id = (uint16) SvUV(ST(1));
		uint32 item_id = (uint32) SvUV(ST(2));
		int16 charges = -1;
		bool attuned = false;
		uint32 augment_one = 0;
		uint32 augment_two = 0;
		uint32 augment_three = 0;
		uint32 augment_four = 0;
		uint32 augment_five = 0;
		uint32 augment_six = 0;
		VALIDATE_THIS_IS_BOT;

		if (items > 3) {
			charges = (int16) SvIV(ST(3));
		}

		if (items > 4) {
			attuned = (bool) SvTRUE(ST(4));
		}

		if (items > 5) {
			augment_one = (uint32) SvUV(ST(5));
		}

		if (items > 6) {
			augment_two = (uint32) SvUV(ST(6));
		}

		if (items > 7) {
			augment_three = (uint32) SvUV(ST(7));
		}

		if (items > 8) {
			augment_four = (uint32) SvUV(ST(8));
		}

		if (items > 9) {
			augment_five = (uint32) SvUV(ST(9));
		}

		if (items > 10) {
			augment_six = (uint32) SvUV(ST(10));
		}

		THIS->AddBotItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three, augment_four, augment_five, augment_six);
	}
	XSRETURN_EMPTY;
}

XS(XS_Bot_CountBotItem);
XS(XS_Bot_CountBotItem) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Bot::CountBotItem(THIS, uint32 item_id)");
	{
		Bot* THIS;
		int item_count = 0;
		uint32 item_id = (uint32) SvUV(ST(1));
		dXSTARG;
		VALIDATE_THIS_IS_BOT;
		item_count = THIS->CountBotItem(item_id);
		XSprePUSH;
		PUSHu((UV) item_count);
	}
	XSRETURN(1);
}

XS(XS_Bot_HasBotItem);
XS(XS_Bot_HasBotItem) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Bot:HasBotItem(THIS, uint32 item_id)");
	{
		Bot* THIS;
		bool has_item = false;
		uint32 item_id = (uint32) SvUV(ST(1));
		VALIDATE_THIS_IS_BOT;
		has_item = THIS->HasBotItem(item_id);
		ST(0) = boolSV(has_item);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Bot_RemoveBotItem);
XS(XS_Bot_RemoveBotItem) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Bot::RemoveBotItem(THIS, uint32 item_id)"); // @categories Spells and Disciplines
	{
		Bot* THIS;
		uint32 item_id = (uint32) SvUV(ST(1));
		VALIDATE_THIS_IS_BOT;
		THIS->RemoveBotItem(item_id);
	}
	XSRETURN_EMPTY;
}

#ifdef __cplusplus
extern "C"
#endif

XS(boot_Bot);
XS(boot_Bot)
{
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;

	if (items != 1)
		fprintf(stderr, "boot_Bot does not take any arguments.");

	char buf[128];

	XS_VERSION_BOOTCHECK;
	newXSproto(strcpy(buf, "AddBotItem"), XS_Bot_AddBotItem, file, "$$$;$$$$$$$$");
	newXSproto(strcpy(buf, "CountBotItem"), XS_Bot_CountBotItem, file, "$$");
	newXSproto(strcpy(buf, "GetOwner"), XS_Bot_GetOwner, file, "$");
	newXSproto(strcpy(buf, "HasBotItem"), XS_Bot_HasBotItem, file, "$$");
	newXSproto(strcpy(buf, "RemoveBotItem"), XS_Bot_RemoveBotItem, file, "$$");
	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES
#endif //BOTS
