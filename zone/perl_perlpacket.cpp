#include "../common/features.h"
#ifdef EMBPERL_XS_CLASSES
#include "../common/global_define.h"
#include "../common/types.h"
#include "embperl.h"

#ifdef seed
#undef seed
#endif

#include "perlpacket.h"

#ifdef THIS	/* this macro seems to leak out on some systems */
#undef THIS
#endif

#define VALIDATE_THIS_IS_PACKET \
	do { \
		if (sv_derived_from(ST(0), "PerlPacket")) { \
			IV tmp = SvIV((SV*)SvRV(ST(0))); \
			THIS = INT2PTR(PerlPacket*, tmp); \
		} else { \
			Perl_croak(aTHX_ "THIS is not of type PerlPacket"); \
		} \
		if (THIS == nullptr) { \
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash."); \
		} \
	} while (0);

XS(XS_PerlPacket_new); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_new)
{
	dXSARGS;
	if (items < 1 || items > 3)
		Perl_croak(aTHX_ "Usage: PerlPacket::new(CLASS, opcode= \"OP_Unknown\", len= 0)");
	{
		char		*CLASS = (char *)SvPV_nolen(ST(0));
		PerlPacket	*RETVAL;
		const char	*opcode;
		uint32		len;

		if (items < 2)
			opcode = "OP_Unknown";
		else {
			opcode = (char *)SvPV_nolen(ST(1));
		}

		if (items < 3)
			len = 0;
		else {
			len = (uint32)SvUV(ST(2));
		}

		RETVAL = new PerlPacket(opcode, len);
		ST(0) = sv_newmortal();
		sv_setref_pv(ST(0), "PerlPacket", (void*)RETVAL);
	}
	XSRETURN(1);
}

XS(XS_PerlPacket_DESTROY); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_DESTROY)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: PerlPacket::DESTROY(THIS)");
	{
		PerlPacket *		THIS;
		VALIDATE_THIS_IS_PACKET;
		delete THIS;
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_SetOpcode); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_SetOpcode)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: PerlPacket::SetOpcode(THIS, opcode)");
	{
		PerlPacket *		THIS;
		bool		RETVAL;
		char *		opcode = (char *)SvPV_nolen(ST(1));
		VALIDATE_THIS_IS_PACKET;
		RETVAL = THIS->SetOpcode(opcode);
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_PerlPacket_Resize); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_Resize)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: PerlPacket::Resize(THIS, len)");
	{
		PerlPacket *		THIS;
		uint32		len = (uint32)SvUV(ST(1));
		VALIDATE_THIS_IS_PACKET;
		THIS->Resize(len);
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_SendTo); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_SendTo)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: PerlPacket::SendTo(THIS, who)");
	{
		PerlPacket *		THIS;
		Client *		who;
		VALIDATE_THIS_IS_PACKET;
		if (sv_derived_from(ST(1), "Client")) {
			IV tmp = SvIV((SV*)SvRV(ST(1)));
			who = INT2PTR(Client *,tmp);
		}
		else
			Perl_croak(aTHX_ "who is not of type Client");
		if(who == nullptr)
			Perl_croak(aTHX_ "who is nullptr, avoiding crash.");

		THIS->SendTo(who);
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_SendToAll); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_SendToAll)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: PerlPacket::SendToAll(THIS)");
	{
		PerlPacket *		THIS;
		VALIDATE_THIS_IS_PACKET;
		THIS->SendToAll();
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_Zero); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_Zero)
{
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: PerlPacket::Zero(THIS)");
	{
		PerlPacket *		THIS;
		VALIDATE_THIS_IS_PACKET;
		THIS->Zero();
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_FromArray); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_FromArray)
{
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: PerlPacket::FromArray(THIS, numbers, length)");
	{
		PerlPacket *		THIS;
		int *		numbers;
		uint32		length = (uint32)SvUV(ST(2));
		VALIDATE_THIS_IS_PACKET;
		AV *av_numbers;
		if (SvROK(ST(1)) && SvTYPE(SvRV(ST(1)))==SVt_PVAV)
			av_numbers = (AV*)SvRV(ST(1));
		else
			Perl_croak(aTHX_ "numbers is not an array reference");
		I32 len_numbers = av_len(av_numbers) + 1;
		I32 ix_numbers;
		numbers = new int[len_numbers];
		for(ix_numbers = 0; ix_numbers < len_numbers; ix_numbers ++) {
				SV **tmp = av_fetch(av_numbers, ix_numbers, 0);
				if(tmp == nullptr || *tmp == nullptr) {
						numbers[ix_numbers] = 0;
						continue;
				}
				numbers[ix_numbers] = (int)SvIV(*tmp);
		};

		THIS->FromArray(numbers, length);
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_SetByte); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_SetByte)
{
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: PerlPacket::SetByte(THIS, pos, val)");
	{
		PerlPacket *		THIS;
		uint32		pos = (uint32)SvUV(ST(1));
		uint8		val = (uint8)SvUV(ST(2));
		VALIDATE_THIS_IS_PACKET;
		THIS->SetByte(pos, val);
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_SetShort); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_SetShort)
{
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: PerlPacket::SetShort(THIS, pos, val)");
	{
		PerlPacket *		THIS;
		uint32		pos = (uint32)SvUV(ST(1));
		uint16		val = (uint16)SvUV(ST(2));
		VALIDATE_THIS_IS_PACKET;
		THIS->SetShort(pos, val);
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_SetLong); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_SetLong)
{
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: PerlPacket::SetLong(THIS, pos, val)");
	{
		PerlPacket *		THIS;
		uint32		pos = (uint32)SvUV(ST(1));
		uint32		val = (uint32)SvUV(ST(2));
		VALIDATE_THIS_IS_PACKET;
		THIS->SetLong(pos, val);
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_SetFloat); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_SetFloat)
{
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: PerlPacket::SetFloat(THIS, pos, val)");
	{
		PerlPacket *		THIS;
		uint32		pos = (uint32)SvUV(ST(1));
		float		val = (float)SvNV(ST(2));
		VALIDATE_THIS_IS_PACKET;
		THIS->SetFloat(pos, val);
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_SetString); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_SetString)
{
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: PerlPacket::SetString(THIS, pos, str)");
	{
		PerlPacket *		THIS;
		uint32		pos = (uint32)SvUV(ST(1));
		char *		str = (char *)SvPV_nolen(ST(2));
		VALIDATE_THIS_IS_PACKET;
		THIS->SetString(pos, str);
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_SetEQ1319); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_SetEQ1319)
{
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: PerlPacket::SetEQ1319(THIS, pos, part13, part19)");
	{
		PerlPacket *		THIS;
		uint32		pos = (uint32)SvUV(ST(1));
		float		part13 = (float)SvNV(ST(2));
		float		part19 = (float)SvNV(ST(3));
		VALIDATE_THIS_IS_PACKET;
		THIS->SetEQ1319(pos, part13, part19);
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_SetEQ1913); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_SetEQ1913)
{
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: PerlPacket::SetEQ1913(THIS, pos, part19, part13)");
	{
		PerlPacket *		THIS;
		uint32		pos = (uint32)SvUV(ST(1));
		float		part19 = (float)SvNV(ST(2));
		float		part13 = (float)SvNV(ST(3));
		VALIDATE_THIS_IS_PACKET;
		THIS->SetEQ1913(pos, part19, part13);
	}
	XSRETURN_EMPTY;
}

XS(XS_PerlPacket_GetByte); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_GetByte)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: PerlPacket::GetByte(THIS, pos)");
	{
		PerlPacket *		THIS;
		uint8		RETVAL;
		dXSTARG;
		uint32		pos = (uint32)SvUV(ST(1));
		VALIDATE_THIS_IS_PACKET;
		RETVAL = THIS->GetByte(pos);
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_PerlPacket_GetShort); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_GetShort)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: PerlPacket::GetShort(THIS, pos)");
	{
		PerlPacket *		THIS;
		uint16		RETVAL;
		dXSTARG;
		uint32		pos = (uint32)SvUV(ST(1));
		VALIDATE_THIS_IS_PACKET;
		RETVAL = THIS->GetShort(pos);
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_PerlPacket_GetLong); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_GetLong)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: PerlPacket::GetLong(THIS, pos)");
	{
		PerlPacket *		THIS;
		uint32		RETVAL;
		dXSTARG;
		uint32		pos = (uint32)SvUV(ST(1));
		VALIDATE_THIS_IS_PACKET;
		RETVAL = THIS->GetLong(pos);
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_PerlPacket_GetFloat); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlPacket_GetFloat)
{
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: PerlPacket::GetFloat(THIS, pos)");
	{
		PerlPacket *		THIS;
		float		RETVAL;
		dXSTARG;
		uint32		pos = (uint32)SvUV(ST(1));
		VALIDATE_THIS_IS_PACKET;
		RETVAL = THIS->GetFloat(pos);
		XSprePUSH;
		PUSHn((double) RETVAL);
	}
	XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_PerlPacket); /* prototype to pass -Wmissing-prototypes */
XS(boot_PerlPacket)
{
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;

	if(items != 1)
		fprintf(stderr, "boot_quest does not take any arguments.");
	char buf[128];

	//add the strcpy stuff to get rid of const warnings....

	XS_VERSION_BOOTCHECK ;
	newXSproto(strcpy(buf, "DESTROY"), XS_PerlPacket_DESTROY, file, "$");
	newXSproto(strcpy(buf, "FromArray"), XS_PerlPacket_FromArray, file, "$$$");
	newXSproto(strcpy(buf, "GetByte"), XS_PerlPacket_GetByte, file, "$$");
	newXSproto(strcpy(buf, "GetFloat"), XS_PerlPacket_GetFloat, file, "$$");
	newXSproto(strcpy(buf, "GetLong"), XS_PerlPacket_GetLong, file, "$$");
	newXSproto(strcpy(buf, "GetShort"), XS_PerlPacket_GetShort, file, "$$");
	newXSproto(strcpy(buf, "Resize"), XS_PerlPacket_Resize, file, "$$");
	newXSproto(strcpy(buf, "SendTo"), XS_PerlPacket_SendTo, file, "$$");
	newXSproto(strcpy(buf, "SendToAll"), XS_PerlPacket_SendToAll, file, "$");
	newXSproto(strcpy(buf, "SetByte"), XS_PerlPacket_SetByte, file, "$$$");
	newXSproto(strcpy(buf, "SetEQ1319"), XS_PerlPacket_SetEQ1319, file, "$$$$");
	newXSproto(strcpy(buf, "SetEQ1913"), XS_PerlPacket_SetEQ1913, file, "$$$$");
	newXSproto(strcpy(buf, "SetFloat"), XS_PerlPacket_SetFloat, file, "$$$");
	newXSproto(strcpy(buf, "SetLong"), XS_PerlPacket_SetLong, file, "$$$");
	newXSproto(strcpy(buf, "SetOpcode"), XS_PerlPacket_SetOpcode, file, "$$");
	newXSproto(strcpy(buf, "SetShort"), XS_PerlPacket_SetShort, file, "$$$");
	newXSproto(strcpy(buf, "SetString"), XS_PerlPacket_SetString, file, "$$$");
	newXSproto(strcpy(buf, "Zero"), XS_PerlPacket_Zero, file, "$");
	newXSproto(strcpy(buf, "new"), XS_PerlPacket_new, file, "$;$$");
	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES

