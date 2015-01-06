#ifndef PETS_H
#define PETS_H

// Defines based on the RoF2 Client
#define PET_HEALTHREPORT	0	// 0x00 - /pet health
#define PET_LEADER			1   // 0x01 - /pet leader
#define PET_ATTACK			2   // 0x02 - /pet attack or Pet Window
#define PET_GUARDME			3	// 0x03 - No longer exists? - define not from client
#define PET_FOLLOWME		4   // 0x04 - /pet follow or Pet Window
#define PET_GUARDHERE		5   // 0x05 - /pet guard or Pet Window
#define PET_SIT				6   // 0x06 - /pet sit or Pet Window
#define PET_SITDOWN			7   // 0x07 - /pet sit on
#define PET_STANDUP			8   // 0x08 - /pet sit off
#define PET_STOP			9	// 0x09 - Not implemented?
								// Guessing 10 and 11 are PET_STOP_ON and PET_STOP_OFF
#define PET_TAUNT			12  // 0x0c - /pet taunt or Pet Window
#define PET_TAUNT_ON		13  // 0x0d - /pet taunt on
#define PET_TAUNT_OFF		14	// 0x0e - /pet taunt off
#define PET_HOLD			15  // 0x0f - /pet hold
#define PET_HOLD_ON			16  // 0x10 - /pet hold on
#define PET_HOLD_OFF		17  // 0x11 - /pet hold off
#define	PET_SLUMBER			18	// 0x12 - What is this? - define not from client
								// Guessing 19 and 20 are PET_SLUMBER_ON and PET_SLUMBER_OFF
#define	PET_NOCAST			21  // 0x15 - /pet no cast
#define	PET_NOCAST_ON		22  // 0x16 - Pet Window No Cast
#define	PET_NOCAST_OFF		23  // 0x17 - Guessed
#define	PET_FOCUS			24  // 0x18 - /pet focus
#define	PET_FOCUS_ON		25  // 0x19 - /pet focus on
#define	PET_FOCUS_OFF		26  // 0x1a - /pet focus off
#define PET_BACKOFF			28  // 0x1c - /pet back off
#define PET_GETLOST			29  // 0x1d - /pet get lost

class Mob;
struct NPCType;

class Pet : public NPC {
	public:
		Pet(NPCType *type_data, Mob *owner, PetType type, uint16 spell_id, int16 power);

	};

#endif

