#ifndef LARION_STRUCTS_H_
#define LARION_STRUCTS_H_

namespace Larion {
	namespace structs {
		struct LoginInfo_Struct {
			/*000*/	char	login_info[64];
			/*064*/	uint8	unknown064[124];
			/*188*/	uint8	zoning;			// 01 if zoning, 00 if not
			/*189*/	uint8	unknown189[275];
			/*488*/
		};

		struct ClientZoneEntry_Struct {
			/*00*/ uint32	unknown00;	// ***Placeholder
			/*04*/ char	char_name[64];	// Player firstname [32]
			/*68*/ uint32	unknown68;
			/*72*/ uint32	unknown72;
		};
	};	//end namespace structs
};	//end namespace larion

#endif /*LARION_STRUCTS_H_*/










