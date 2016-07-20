#ifndef PETS_H
#define PETS_H

// Defines based on the RoF2 Client
#define PET_HEALTHREPORT	0	// 0x00 - /pet health or Pet Window
#define PET_LEADER			1   // 0x01 - /pet leader or Pet Window
#define PET_ATTACK			2   // 0x02 - /pet attack or Pet Window
#define PET_QATTACK			3	// 0x03 - /pet qattack or Pet Window
#define PET_FOLLOWME		4   // 0x04 - /pet follow or Pet Window
#define PET_GUARDHERE		5   // 0x05 - /pet guard or Pet Window
#define PET_SIT				6   // 0x06 - /pet sit or Pet Window
#define PET_SITDOWN			7   // 0x07 - /pet sit on
#define PET_STANDUP			8   // 0x08 - /pet sit off
#define PET_STOP			9	// 0x09 - /pet stop or Pet Window - Not implemented
#define PET_STOP_ON			10	// 0x0a - /pet stop on - Not implemented
#define PET_STOP_OFF		11	// 0x0b - /pet stop off - Not implemented
#define PET_TAUNT			12  // 0x0c - /pet taunt or Pet Window
#define PET_TAUNT_ON		13  // 0x0d - /pet taunt on
#define PET_TAUNT_OFF		14	// 0x0e - /pet taunt off
#define PET_HOLD			15  // 0x0f - /pet hold or Pet Window
#define PET_HOLD_ON			16  // 0x10 - /pet hold on
#define PET_HOLD_OFF		17  // 0x11 - /pet hold off
#define	PET_SLUMBER			18	// 0x12 - What activates this? - define guessed
#define	PET_SLUMBER_ON		19	// 0x13 - What activates this? - define guessed
#define	PET_SLUMBER_OFF		20	// 0x14 - What activates this? - define guessed
#define	PET_SPELLHOLD		21  // 0x15 - /pet no cast or /pet spellhold or Pet Window
#define	PET_SPELLHOLD_ON	22  // 0x16 - /pet spellhold on
#define	PET_SPELLHOLD_OFF	23  // 0x17 - /pet spellhold off
#define	PET_FOCUS			24  // 0x18 - /pet focus or Pet Window
#define	PET_FOCUS_ON		25  // 0x19 - /pet focus on
#define	PET_FOCUS_OFF		26  // 0x1a - /pet focus off
#define PET_BACKOFF			28  // 0x1c - /pet back off
#define PET_GETLOST			29  // 0x1d - /pet get lost
#define PET_GUARDME			30	// 0x1e - Same as /pet follow, but different message in older clients - define not from client

class Mob;
struct NPCType;

class Pet : public NPC {
	public:
		Pet(NPCType *type_data, Mob *owner, PetType type, uint16 spell_id, int16 power);
		virtual void SetTarget(Mob *mob);
		virtual bool CheckSpellLevelRestriction(uint16 spell_id);

	};

#endif

