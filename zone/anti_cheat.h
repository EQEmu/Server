#ifndef ANTICHEAT_H
#define ANTICHEAT_H
class anti_cheat;
class Client;

#include "../common/timer.h"
#include "../common/rulesys.h"
#include <glm/ext/vector_float3.hpp>
#include "../common/eq_packet_structs.h"
#include "../common/eq_packet.h"

typedef enum {
	Collision = 1,
	TeleportB,
	TeleportA,
	ZoneLine,
	Unknown0x5,
	Unknown0x6,
	SpellA, // Titanium - UF
	Unknown0x8,
	SpellB // Used in RoF+
}UpdateMovementType;

typedef enum {
	ShadowStep,
	KnockBack,
	Port,
	Assist,
	Sense,
	MAX_EXEMPTIONS
} ExemptionType;

typedef enum {
	MQWarp,
	MQWarpShadowStep,
	MQWarpKnockBack,
	MQWarpLight,
	MQZone,
	MQZoneUnknownDest,
	MQGate,
	MQGhost,
	MQFastMem,
	MQWarpAbsolute
} CheatTypes;

class anti_cheat {
public:
	anti_cheat() {
		set_exempt_status(ShadowStep, false);
		set_exempt_status(KnockBack, false);
		set_exempt_status(Port, false);
		set_exempt_status(Assist, false);
		set_exempt_status(Sense, false);
		m_distance_since_last_position_check = 0.0f;
		m_cheat_detect_moved = false;
		m_target = nullptr;
		m_time_since_last_memorization = 0;
		m_time_since_last_position_check = 0;
		m_time_since_last_warp_detection.Start();
	}
	void set_client(Client* cli);
	void set_exempt_status(ExemptionType type, bool v);
	bool get_exempt_status(ExemptionType type);
	void cheat_detected(CheatTypes type, glm::vec3 position1, glm::vec3 position2 = glm::vec3(0, 0, 0));
	void movement_check(glm::vec3 updated_position);
	void movement_check(uint32 time_between_checks = 1000);
	void start_mem_check();
	void restart_mem_check();
	void process_movement_history(const EQApplicationPacket* app);
	void client_process();
private:
	bool m_exemption[ExemptionType::MAX_EXEMPTIONS];
	float m_distance_since_last_position_check;
	bool m_cheat_detect_moved;
	
	Client* m_target;
	uint32 m_time_since_last_position_check;
	uint32 m_time_since_last_memorization;
	Timer m_time_since_last_warp_detection;
};
#endif ANTICHEAT_H
