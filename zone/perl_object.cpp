#include "../common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "../common/global_define.h"
#include "embperl.h"

#ifdef seed
#undef seed
#endif

#include "object.h"

#ifdef THIS /* this macro seems to leak out on some systems */
#undef THIS
#endif

#define VALIDATE_THIS_IS_OBJECT \
	do { \
		if (sv_derived_from(ST(0), "Object")) { \
			IV tmp = SvIV((SV*)SvRV(ST(0))); \
			THIS = INT2PTR(Object*, tmp); \
		} else { \
			Perl_croak(aTHX_ "THIS is not of type Object"); \
		} \
		if (THIS == nullptr) { \
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash."); \
		} \
	} while (0);

XS(XS_Object_IsGroundSpawn); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_IsGroundSpawn) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::IsGroundSpawn(THIS)");  // @categories Objects
	{
		Object *THIS;
		bool   RETVAL;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->IsGroundSpawn();
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}


XS(XS_Object_Close); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_Close) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::Close(THIS)");  // @categories Objects
	{
		Object *THIS;
		VALIDATE_THIS_IS_OBJECT;
		THIS->Close();
	}
	XSRETURN_EMPTY;
}


XS(XS_Object_Delete); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_Delete) {
	dXSARGS;
	if (items < 1 || items > 2)
		Perl_croak(aTHX_ "Usage: Object::Delete(THIS, [bool reset_state = false])");  // @categories Objects
	{
		Object *THIS;
		bool   reset_state;
		VALIDATE_THIS_IS_OBJECT;
		if (items < 2)
			reset_state = false;
		else {
			reset_state = (bool) SvTRUE(ST(1));
		}

		THIS->Delete(reset_state);
	}
	XSRETURN_EMPTY;
}
XS(XS_Object_StartDecay); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_StartDecay) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::StartDecay(THIS)");  // @categories Objects
	{
		Object *THIS;
		VALIDATE_THIS_IS_OBJECT;
		THIS->StartDecay();
	}
	XSRETURN_EMPTY;
}


XS(XS_Object_DeleteItem); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_DeleteItem) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::DeleteItem(THIS, uint8 index)");  // @categories Objects
	{
		Object *THIS;
		uint8  index = (uint8) SvUV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->DeleteItem(index);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_IsObject); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_IsObject) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::IsObject(THIS)");  // @categories Objects
	{
		Object *THIS;
		bool   RETVAL;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->IsObject();
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}


XS(XS_Object_Save); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_Save) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::Save(THIS)");  // @categories Objects
	{
		Object *THIS;
		bool   RETVAL;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->Save();
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}


XS(XS_Object_SetID); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetID(THIS, uint16 id)");  // @categories Objects
	{
		Object *THIS;
		uint16 set_id = (uint16) SvUV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetID(set_id);
	}
	XSRETURN_EMPTY;
}


XS(XS_Object_ClearUser); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_ClearUser) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::ClearUser(THIS)");  // @categories Objects
	{
		Object *THIS;
		VALIDATE_THIS_IS_OBJECT;
		THIS->ClearUser();
	}
	XSRETURN_EMPTY;
}


XS(XS_Object_GetDBID); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetDBID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetDBID(THIS)");  // @categories Objects
	{
		Object *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetDBID();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Object_GetID); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetID(THIS)");  // @categories Objects
	{
		Object *THIS;
		uint16 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetID();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Object_GetX); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetX) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetX(THIS)");  // @categories Objects
	{
		Object *THIS;
		float  RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetX();
		XSprePUSH;
		PUSHn((double) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Object_GetY); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetY) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetY(THIS)");  // @categories Objects
	{
		Object *THIS;
		float  RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetY();
		XSprePUSH;
		PUSHn((double) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Object_GetZ); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetZ) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetZ(THIS)");  // @categories Objects
	{
		Object *THIS;
		float  RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetZ();
		XSprePUSH;
		PUSHn((double) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Object_GetHeading); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetHeading) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetHeading(THIS)");  // @categories Objects
	{
		Object *THIS;
		float  RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetHeadingData();
		XSprePUSH;
		PUSHn((double) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Object_VarSave); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_VarSave) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::VarSave(THIS)");  // @categories Objects
	{
		Object *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->VarSave();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}


XS(XS_Object_GetType); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetType) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetType(THIS)");  // @categories Objects
	{
		Object *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetType();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}


XS(XS_Object_SetType); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetType) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetType(THIS, uint32 type)");  // @categories Objects
	{
		Object *THIS;
		uint32 type = (uint32) SvUV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetType(type);
	}
	XSRETURN_EMPTY;
}


XS(XS_Object_GetIcon); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetIcon) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetIcon(THIS)");  // @categories Objects
	{
		Object *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetIcon();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}


XS(XS_Object_SetIcon); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetIcon) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetIcon(THIS, uint32 icon)");  // @categories Objects
	{
		Object *THIS;
		uint32 icon = (uint32) SvUV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetIcon(icon);
	}
	XSRETURN_EMPTY;
}


XS(XS_Object_GetItemID); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetItemID) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetItemID(THIS)");  // @categories Objects
	{
		Object *THIS;
		uint32 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetItemID();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}


XS(XS_Object_SetItemID); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetItemID) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetItemID(THIS, uint32 item_id)");  // @categories Objects
	{
		Object *THIS;
		uint32 itemid = (uint32) SvUV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetItemID(itemid);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_SetLocation); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetLocation) {
	dXSARGS;
	if (items != 4)
		Perl_croak(aTHX_ "Usage: Object::SetLocation(THIS, float x, float y, float z)");  // @categories Objects
	{
		Object *THIS;
		float  x = (float) SvNV(ST(1));
		float  y = (float) SvNV(ST(2));
		float  z = (float) SvNV(ST(3));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetLocation(x, y, z);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_SetX); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetX) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetX(THIS, float x)");  // @categories Objects
	{
		Object *THIS;
		float  pos = (float) SvNV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetX(pos);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_SetY); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetY) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetY(THIS, float y)");  // @categories Objects
	{
		Object *THIS;
		float  pos = (float) SvNV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetY(pos);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_SetZ); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetZ) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetZ(THIS, float z)");  // @categories Objects
	{
		Object *THIS;
		float  pos = (float) SvNV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetZ(pos);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_SetHeading); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetHeading) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetHeading(THIS, float heading)");  // @categories Objects
	{
		Object *THIS;
		float  heading = (float) SvNV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetHeading(heading);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_SetModelName); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetModelName) {
	dXSARGS;
	if (items < 1 || items > 2)
		Perl_croak(aTHX_ "Usage: Object::SetModelName(THIS, string name)");  // @categories Objects
	{
		Object *THIS;
		char   *name = nullptr;
		VALIDATE_THIS_IS_OBJECT;
		if (items > 1) { name = (char *) SvPV_nolen(ST(1)); }

		THIS->SetModelName(name);
	}
	XSRETURN_EMPTY;
}
XS(XS_Object_GetModelName); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetModelName) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetModelName(THIS)");  // @categories Objects
	{
		Object     *THIS;
		Const_char *RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetModelName();
		sv_setpv(TARG, RETVAL);
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Object_Repop); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_Repop) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::Repop(THIS)");  // @categories Objects
	{
		Object *THIS;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;		THIS->Repop();
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_Depop); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_Depop) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::Depop(THIS)");  // @categories Objects
	{
		Object *THIS;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;		THIS->Depop();
	}
	XSRETURN_EMPTY;
}


XS(XS_Object_GetEntityVariable); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetEntityVariable) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::GetEntityVariable(THIS, string key)");  // @categories Objects
	{
		Object     *THIS;
		Const_char *id = SvPV_nolen(ST(1));
		Const_char *RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetEntityVariable(id);
		sv_setpv(TARG, RETVAL);
		XSprePUSH;
		PUSHTARG;
	}
	XSRETURN(1);
}

XS(XS_Object_EntityVariableExists); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_EntityVariableExists) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::EntityVariableExists(THIS, string key)");  // @categories Objects
	{
		Object     *THIS;
		Const_char *id = SvPV_nolen(ST(1));
		bool       RETVAL;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->EntityVariableExists(id);
		ST(0) = boolSV(RETVAL);
		sv_2mortal(ST(0));
	}
	XSRETURN(1);
}

XS(XS_Object_SetEntityVariable); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetEntityVariable) {
	dXSARGS;
	if (items != 3)
		Perl_croak(aTHX_ "Usage: Object::SetEntityVariable(THIS, string key, string var)");  // @categories Objects
	{
		Object     *THIS;
		Const_char *id  = SvPV_nolen(ST(1));
		const char *var = (const char *) SvPV_nolen(ST(2));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetEntityVariable(id, var);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_GetSolidType); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetSolidType) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetSolidType(THIS)");  // @categories Objects
	{
		Object *THIS;
		uint16 RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetSolidType();
		XSprePUSH;
		PUSHu((UV) RETVAL);
	}
	XSRETURN(1);
}


XS(XS_Object_SetSolidType); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetSolidType) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetSolidType(THIS, uint16 type)");  // @categories Objects
	{
		Object *THIS;
		uint16 type = (uint16) SvUV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetSolidType(type);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_GetSize); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetSize) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetSize(THIS)");  // @categories Objects
	{
		Object *THIS;
		float  RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetSize();
		XSprePUSH;
		PUSHn((double) RETVAL);
	}
	XSRETURN(1);
}


XS(XS_Object_SetSize); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetSize) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetSize(THIS, float size)");  // @categories Objects
	{
		Object *THIS;
		float  size = (float) SvNV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetSize(size);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_SetTiltX); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetTiltX) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetTiltX(THIS, float tilt_x)");  // @categories Objects
	{
		Object *THIS;
		float  pos = (float) SvNV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetTiltX(pos);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_SetTiltY); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_SetTiltY) {
	dXSARGS;
	if (items != 2)
		Perl_croak(aTHX_ "Usage: Object::SetTiltY(THIS, float tilt_y)");  // @categories Objects
	{
		Object *THIS;
		float  pos = (float) SvNV(ST(1));
		VALIDATE_THIS_IS_OBJECT;
		THIS->SetTiltY(pos);
	}
	XSRETURN_EMPTY;
}

XS(XS_Object_GetTiltX); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetTiltX) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetTiltX(THIS)"); // @categories Objects
	{
		Object *THIS;
		float  RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetTiltX();
		XSprePUSH;
		PUSHn((double) RETVAL);
	}
	XSRETURN(1);
}

XS(XS_Object_GetTiltY); /* prototype to pass -Wmissing-prototypes */
XS(XS_Object_GetTiltY) {
	dXSARGS;
	if (items != 1)
		Perl_croak(aTHX_ "Usage: Object::GetTiltY(THIS)"); // @categories Objects
	{
		Object *THIS;
		float  RETVAL;
		dXSTARG;
		VALIDATE_THIS_IS_OBJECT;
		RETVAL = THIS->GetTiltY();
		XSprePUSH;
		PUSHn((double) RETVAL);
	}
	XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_Object); /* prototype to pass -Wmissing-prototypes */
XS(boot_Object) {
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;

	if (items != 1)
		fprintf(stderr, "boot_quest does not take any arguments.");
	char buf[128];

	//add the strcpy stuff to get rid of const warnings....

	XS_VERSION_BOOTCHECK;
	newXSproto(strcpy(buf, "ClearUser"), XS_Object_ClearUser, file, "$");
	newXSproto(strcpy(buf, "Close"), XS_Object_Close, file, "$");
	newXSproto(strcpy(buf, "Delete"), XS_Object_Delete, file, "$$");
	newXSproto(strcpy(buf, "DeleteItem"), XS_Object_DeleteItem, file, "$$");
	newXSproto(strcpy(buf, "Depop"), XS_Object_Depop, file, "$");
	newXSproto(strcpy(buf, "EntityVariableExists"), XS_Object_EntityVariableExists, file, "$$");
	newXSproto(strcpy(buf, "GetDBID"), XS_Object_GetDBID, file, "$");
	newXSproto(strcpy(buf, "GetEntityVariable"), XS_Object_GetEntityVariable, file, "$$");
	newXSproto(strcpy(buf, "GetHeading"), XS_Object_GetHeading, file, "$");
	newXSproto(strcpy(buf, "GetID"), XS_Object_GetID, file, "$");
	newXSproto(strcpy(buf, "GetIcon"), XS_Object_GetIcon, file, "$");
	newXSproto(strcpy(buf, "GetItemID"), XS_Object_GetItemID, file, "$");
	newXSproto(strcpy(buf, "GetModelName"), XS_Object_GetModelName, file, "$");
	newXSproto(strcpy(buf, "GetSize"), XS_Object_GetSize, file, "$");
	newXSproto(strcpy(buf, "GetSolidType"), XS_Object_GetSolidType, file, "$");
	newXSproto(strcpy(buf, "GetTiltX"), XS_Object_GetTiltX, file, "$$");
	newXSproto(strcpy(buf, "GetTiltY"), XS_Object_GetTiltY, file, "$");
	newXSproto(strcpy(buf, "GetType"), XS_Object_GetType, file, "$");
	newXSproto(strcpy(buf, "GetX"), XS_Object_GetX, file, "$");
	newXSproto(strcpy(buf, "GetY"), XS_Object_GetY, file, "$");
	newXSproto(strcpy(buf, "GetZ"), XS_Object_GetZ, file, "$");
	newXSproto(strcpy(buf, "IsGroundSpawn"), XS_Object_IsGroundSpawn, file, "$");
	newXSproto(strcpy(buf, "Repop"), XS_Object_Repop, file, "$");
	newXSproto(strcpy(buf, "Save"), XS_Object_Save, file, "$");
	newXSproto(strcpy(buf, "SetEntityVariable"), XS_Object_SetEntityVariable, file, "$$$");
	newXSproto(strcpy(buf, "SetHeading"), XS_Object_SetHeading, file, "$$");
	newXSproto(strcpy(buf, "SetID"), XS_Object_SetID, file, "$$");
	newXSproto(strcpy(buf, "SetIcon"), XS_Object_SetIcon, file, "$$");
	newXSproto(strcpy(buf, "SetItemID"), XS_Object_SetItemID, file, "$$");
	newXSproto(strcpy(buf, "SetLocation"), XS_Object_SetLocation, file, "$$$$");
	newXSproto(strcpy(buf, "SetModelName"), XS_Object_SetModelName, file, "$$");
	newXSproto(strcpy(buf, "SetSize"), XS_Object_SetSize, file, "$$");
	newXSproto(strcpy(buf, "SetSolidType"), XS_Object_SetSolidType, file, "$$");
	newXSproto(strcpy(buf, "SetTiltX"), XS_Object_SetTiltX, file, "$$");
	newXSproto(strcpy(buf, "SetTiltY"), XS_Object_SetTiltY, file, "$");
	newXSproto(strcpy(buf, "SetType"), XS_Object_SetType, file, "$$");
	newXSproto(strcpy(buf, "SetX"), XS_Object_SetX, file, "$$");
	newXSproto(strcpy(buf, "SetY"), XS_Object_SetY, file, "$$");
	newXSproto(strcpy(buf, "SetZ"), XS_Object_SetZ, file, "$$");
	newXSproto(strcpy(buf, "StartDecay"), XS_Object_StartDecay, file, "$$");
	newXSproto(strcpy(buf, "VarSave"), XS_Object_VarSave, file, "$");
	XSRETURN_YES;
}
#endif //EMBPERL_XS_CLASSES
