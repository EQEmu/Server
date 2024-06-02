#ifndef PETS_H
#define PETS_H

class Mob;
struct NPCType;

class Pet : public NPC {
public:
	Pet(NPCType* type_data, Mob* owner, uint8_t type, uint16 spell_id, int16 power);
	virtual bool CheckSpellLevelRestriction(Mob* caster, uint16 spell_id);
};

namespace Pets {
	namespace Button {
		constexpr uint32 Sit         = 0;
		constexpr uint32 Stop        = 1;
		constexpr uint32 Regroup     = 2;
		constexpr uint32 Follow      = 3;
		constexpr uint32 Guard       = 4;
		constexpr uint32 Taunt       = 5;
		constexpr uint32 Hold        = 6;
		constexpr uint32 GreaterHold = 7;
		constexpr uint32 Focus       = 8;
		constexpr uint32 SpellHold   = 9;

		constexpr uint32 Off = 0;
		constexpr uint32 On  = 1;
	}

	namespace Command {
		constexpr uint32 HealthReport   = 0;   // /pet health or Pet Window
		constexpr uint32 Leader         = 1;   // /pet leader or Pet Window
		constexpr uint32 Attack         = 2;   // /pet attack or Pet Window
		constexpr uint32 QueueAttack    = 3;   // /pet qattack or Pet Window
		constexpr uint32 Follow         = 4;   // /pet follow or Pet Window
		constexpr uint32 Guard          = 5;   // /pet guard or Pet Window
		constexpr uint32 Sit            = 6;   // /pet sit or Pet Window
		constexpr uint32 SitOn          = 7;   // /pet sit on
		constexpr uint32 SitOff         = 8;   // /pet sit off
		constexpr uint32 Stop           = 9;   // /pet stop or Pet Window
		constexpr uint32 StopOn         = 10;  // /pet stop on - Not implemented
		constexpr uint32 StopOff        = 11;  // /pet stop off - Not implemented
		constexpr uint32 Taunt          = 12;  // /pet taunt or Pet Window
		constexpr uint32 TauntOn        = 13;  // /pet taunt on
		constexpr uint32 TauntOff       = 14;  // /pet taunt off
		constexpr uint32 Hold           = 15;  // /pet hold or Pet Window, won't add to hate list unless attacking
		constexpr uint32 HoldOn         = 16;  // /pet hold on
		constexpr uint32 HoldOff        = 17;  // /pet hold off
		constexpr uint32 GreaterHold    = 18;  // /pet ghold, will never add to hate list unless told to
		constexpr uint32 GreaterHoldOn  = 19;  // /pet ghold on
		constexpr uint32 GreaterHoldOff = 20;  // /pet ghold off
		constexpr uint32 SpellHold      = 21;  // /pet no cast or /pet spellhold or Pet Window
		constexpr uint32 SpellHoldOn    = 22;  // /pet spellhold on
		constexpr uint32 SpellHoldOff   = 23;  // /pet spellhold off
		constexpr uint32 Focus          = 24;  // /pet focus or Pet Window
		constexpr uint32 FocusOn        = 25;  // /pet focus on
		constexpr uint32 FocusOff       = 26;  // /pet focus off
		constexpr uint32 FeignDeath     = 27;  // /pet feign
		constexpr uint32 BackOff        = 28;  // /pet back off
		constexpr uint32 GetLost        = 29;  // /pet get lost
		constexpr uint32 GuardMe        = 30;  // Same as /pet follow, but different message in older clients - define not from client /pet target in modern clients but doesn't send packet
		constexpr uint32 Regroup        = 31;  // /pet regroup, acts like classic hold. Stops attack and moves back to guard/you but doesn't clear hate list
		constexpr uint32 RegroupOn      = 32;  // /pet regroup on, turns on regroup
		constexpr uint32 RegroupOff     = 33;  // /pet regroup off, turns off regroup
		constexpr uint32 Max            = RegroupOff + 1;

		std::string GetName(uint32 pet_command);
		bool IsValid(uint32 pet_command);
	}

	namespace Type {
		constexpr uint8 Familiar   = 0; // only listens to /pet get lost
		constexpr uint8 Animation  = 1; // does not listen to any commands
		constexpr uint8 Normal     = 2;
		constexpr uint8 Charmed    = 3;
		constexpr uint8 NPCFollow  = 4;
		constexpr uint8 TargetLock = 5; // remain active as long as something is on the hatelist. Don't listen to any commands
		constexpr uint8 None       = std::numeric_limits<uint8_t>::max();// not a pet

		std::string GetName(uint8 pet_type);
		bool IsValid(uint8 pet_type);
	}
};

static std::map<uint32, std::string> pet_commands = {
	{ Pets::Command::HealthReport,   "Health Report" },
	{ Pets::Command::Leader,         "Leader" },
	{ Pets::Command::Attack,         "Attack" },
	{ Pets::Command::QueueAttack,    "Queue Attack" },
	{ Pets::Command::Follow,         "Follow" },
	{ Pets::Command::Guard,          "Guard" },
	{ Pets::Command::Sit,            "Sit" },
	{ Pets::Command::SitOn,          "Sit On" },
	{ Pets::Command::SitOff,         "Sit Off" },
	{ Pets::Command::Stop,           "Stop" },
	{ Pets::Command::StopOn,         "Stop On" },
	{ Pets::Command::StopOff,        "Stop Off" },
	{ Pets::Command::Taunt,          "Taunt" },
	{ Pets::Command::TauntOn,        "Taunt On" },
	{ Pets::Command::TauntOff,       "Taunt Off" },
	{ Pets::Command::Hold,           "Hold" },
	{ Pets::Command::HoldOn,         "Hold On" },
	{ Pets::Command::HoldOff,        "Hold Off" },
	{ Pets::Command::GreaterHold,    "Greater Hold" },
	{ Pets::Command::GreaterHoldOn,  "Greater Hold On" },
	{ Pets::Command::GreaterHoldOff, "Greater Hold Off" },
	{ Pets::Command::SpellHold,      "Spell Hold" },
	{ Pets::Command::SpellHoldOn,    "Spell Hold On" },
	{ Pets::Command::SpellHoldOff,   "Spell Hold Off" },
	{ Pets::Command::Focus,          "Focus" },
	{ Pets::Command::FocusOn,        "Focus On" },
	{ Pets::Command::FocusOff,       "Focus Off" },
	{ Pets::Command::FeignDeath,     "Feign Death" },
	{ Pets::Command::BackOff,        "Back Off" },
	{ Pets::Command::GetLost,        "Get Lost" },
	{ Pets::Command::GuardMe,        "Guard Me" },
	{ Pets::Command::Regroup,        "Regroup" },
	{ Pets::Command::RegroupOn,      "Regroup On" },
	{ Pets::Command::RegroupOff,     "Regroup Off" },
	{ Pets::Command::Max,            "Max" }
};

static std::map<uint8, std::string> pet_types = {
	{ Pets::Type::Familiar,   "Familiar" },
	{ Pets::Type::Animation,  "Animation" },
	{ Pets::Type::Normal,     "Normal" },
	{ Pets::Type::Charmed,    "Charmed" },
	{ Pets::Type::NPCFollow,  "NPCFollow" },
	{ Pets::Type::TargetLock, "TargetLock" },
	{ Pets::Type::None,       "None" },
};

#endif

