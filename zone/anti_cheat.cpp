#include "anti_cheat.h"
#include "client.h"
#include "quest_parser_collection.h"

void anti_cheat::set_client(Client* cli)
{
	m_target = cli;
}

void anti_cheat::set_exempt_status(ExemptionType type, bool v) {
	if (v == true)
	{
		movement_check();
	}
	m_exemption[type] = v;
}

bool anti_cheat::get_exempt_status(ExemptionType type) {
	return m_exemption[type];
}

void anti_cheat::cheat_detected(CheatTypes type, glm::vec3 position1, glm::vec3 position2) {
		switch (type) {
		case MQWarp:
			if (m_time_since_last_warp_detection.GetRemainingTime() == 0 && RuleB(Zone, EnableMQWarpDetector) &&
				((m_target->Admin() < RuleI(Zone, MQWarpExemptStatus) || (RuleI(Zone, MQWarpExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQWarp (large warp detection) with location from x [{:.2f}] y [{:.2f}] z [{:.2f}] to x [{:.2f}] y [{:.2f}] z [{:.2f}] Distance [{:.2f}]",
					position1.x,
					position1.y,
					position1.z,
					position2.x,
					position2.y,
					position2.z,
					Distance(position1, position2));
				database.SetMQDetectionFlag(m_target->AccountName(), m_target->GetName(), message.c_str(), zone->GetShortName());
				LogCheat(message.c_str());
				std::string export_string = fmt::format("{} {} {}", position1.x, position1.y, position1.z);
				parse->EventPlayer(EVENT_WARP, m_target, export_string, 0);
			}
			break;
		case MQWarpAbsolute:
			if (RuleB(Zone, EnableMQWarpDetector) &&
				((m_target->Admin() < RuleI(Zone, MQWarpExemptStatus) || (RuleI(Zone, MQWarpExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQWarp (Absolute) with location from x [{:.2f}] y [{:.2f}] z [{:.2f}] to x [{:.2f}] y [{:.2f}] z [{:.2f}] Distance [{:.2f}]",
					position1.x,
					position1.y,
					position1.z,
					position2.x,
					position2.y,
					position2.z,
					Distance(position1, position2));
				database.SetMQDetectionFlag(m_target->AccountName(), m_target->GetName(), message.c_str(), zone->GetShortName());
				LogCheat(message.c_str());
				std::string export_string = fmt::format("{} {} {}", position1.x, position1.y, position1.z);
				parse->EventPlayer(EVENT_WARP, m_target, export_string, 0);
				m_time_since_last_warp_detection.Start(2500);
			}
			break;
		case MQWarpShadowStep:
			if (RuleB(Zone, EnableMQWarpDetector) &&
				((m_target->Admin() < RuleI(Zone, MQWarpExemptStatus) || (RuleI(Zone, MQWarpExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQWarp(ShadowStep) with location from x [{:.2f}] y [{:.2f}] z [{:.2f}] the target was shadow step exempt but we still found this suspicious.",
					position1.x,
					position1.y,
					position1.z
				);
				database.SetMQDetectionFlag(m_target->AccountName(), m_target->GetName(), message.c_str(), zone->GetShortName());
				LogCheat(message.c_str());
			}
			break;
		case MQWarpKnockBack:
			if (RuleB(Zone, EnableMQWarpDetector) &&
				((m_target->Admin() < RuleI(Zone, MQWarpExemptStatus) || (RuleI(Zone, MQWarpExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQWarp(Knockback) with location from x [{:.2f}] y [{:.2f}] z [{:.2f}] the target was Knock Back exempt but we still found this suspicious.",
					position1.x,
					position1.y,
					position1.z
				);
				database.SetMQDetectionFlag(m_target->AccountName(), m_target->GetName(), message.c_str(), zone->GetShortName());
				LogCheat(message.c_str());
			}
			break;

		case MQWarpLight:
			if (RuleB(Zone, EnableMQWarpDetector) &&
				((m_target->Admin() < RuleI(Zone, MQWarpExemptStatus) || (RuleI(Zone, MQWarpExemptStatus)) == -1))) {
				if (RuleB(Zone, MarkMQWarpLT)) {
					std::string message = fmt::format(
						"/MQWarp(Knockback) with location from x [{:.2f}] y [{:.2f}] z [{:.2f}] running fast but not fast enough to get killed, possibly: small warp, speed hack, excessive lag, marked as suspicious.",
						position1.x,
						position1.y,
						position1.z
					);
					database.SetMQDetectionFlag(m_target->AccountName(), m_target->GetName(), message.c_str(), zone->GetShortName());
					LogCheat(message.c_str());
				}
			}
			break;

		case MQZone:
			if (RuleB(Zone, EnableMQZoneDetector) &&
				((m_target->Admin() < RuleI(Zone, MQZoneExemptStatus) || (RuleI(Zone, MQZoneExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQZone used at x [{:.2f}] y [{:.2f}] z [{:.2f}]",
					position1.x,
					position1.y,
					position1.z
				);
				database.SetMQDetectionFlag(m_target->AccountName(), m_target->GetName(), message.c_str(), zone->GetShortName());
				LogCheat(message.c_str());
			}
			break;
		case MQZoneUnknownDest:
			if (RuleB(Zone, EnableMQZoneDetector) &&
				((m_target->Admin() < RuleI(Zone, MQZoneExemptStatus) || (RuleI(Zone, MQZoneExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQZone used at x [{:.2f}] y [{:.2f}] z [{:.2f}] with Unknown Destination",
					position1.x,
					position1.y,
					position1.z
				);
				database.SetMQDetectionFlag(m_target->AccountName(), m_target->GetName(), message.c_str(), zone->GetShortName());
				LogCheat(message.c_str());
			}
			break;
		case MQGate:
			if (RuleB(Zone, EnableMQGateDetector) &&
				((m_target->Admin() < RuleI(Zone, MQGateExemptStatus) || (RuleI(Zone, MQGateExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQGate used at x [{:.2f}] y [{:.2f}] z [{:.2f}]",
					position1.x,
					position1.y,
					position1.z
				);
				database.SetMQDetectionFlag(m_target->AccountName(), m_target->GetName(), message.c_str(), zone->GetShortName());
				LogCheat(message.c_str());
			}
			break;
		case MQGhost:
			if (RuleB(Zone, EnableMQGhostDetector) &&
				((m_target->Admin() < RuleI(Zone, MQGhostExemptStatus) || (RuleI(Zone, MQGhostExemptStatus)) == -1))) {
				database.SetMQDetectionFlag(m_target->AccountName(), m_target->GetName(), "/MQGhost", zone->GetShortName());
				LogCheat("/MQGhost");
			}
			break;
		case MQFastMem:
			if (RuleB(Zone, EnableMQFastMemDetector) &&
				((m_target->Admin() < RuleI(Zone, MQFastMemExemptStatus) || (RuleI(Zone, MQFastMemExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQFastMem used at x [{:.2f}] y [{:.2f}] z [{:.2f}]",
					position1.x,
					position1.y,
					position1.z
				);
				database.SetMQDetectionFlag(m_target->AccountName(), m_target->GetName(), message.c_str(), zone->GetShortName());
				LogCheat(message.c_str());
			}
			break;
		default:
			std::string message = fmt::format(
				"Unhandled HackerDetection flag with location from x [{:.2f}] y [{:.2f}] z [{:.2f}]",
				position1.x,
				position1.y,
				position1.z
			);
			database.SetMQDetectionFlag(m_target->AccountName(), m_target->GetName(), message.c_str(), zone->GetShortName());
			LogCheat(message.c_str());
			break;
		}
}

void anti_cheat::movement_check(glm::vec3 updated_position) {
	float dist = DistanceNoZ(m_target->GetPosition(), updated_position);
	uint32 cur_time = Timer::GetCurrentTime();
	if (dist == 0) {
		if (m_distance_since_last_position_check > 0.0f) {
			movement_check(0);
		}
		else {
			m_time_since_last_position_check = cur_time;
			m_cheat_detect_moved = false;
		}
	}
	else {
		m_distance_since_last_position_check += dist;
		m_cheat_detect_moved = true;
		if (m_time_since_last_position_check == 0) {
			m_time_since_last_position_check = cur_time;
		}
		else {
			movement_check(2500);
		}
	}
}

void anti_cheat::movement_check(uint32 time_between_checks) {
	uint32 cur_time = Timer::GetCurrentTime();
	if ((cur_time - m_time_since_last_position_check) > time_between_checks) {
		float speed = (m_distance_since_last_position_check * 100) / (float)(cur_time - m_time_since_last_position_check);
		int runs = m_target->GetRunspeed() / std::min(RuleR(Zone, MQWarpDetectionDistanceFactor), 1.0f);
		if (speed > runs) {
			if (!m_target->GetGMSpeed() && (runs >= m_target->GetBaseRunspeed() || (speed > (m_target->GetBaseRunspeed() / std::min(RuleR(Zone, MQWarpDetectionDistanceFactor), 1.0f))))) {
				if (get_exempt_status(ShadowStep)) {
					if (m_distance_since_last_position_check > 800) {
						cheat_detected(MQWarpShadowStep, glm::vec3(m_target->GetX(), m_target->GetY(), m_target->GetZ()));
					}
				}
				else if (get_exempt_status(KnockBack)) {
					if (speed > 30.0f) {
						cheat_detected(MQWarpKnockBack, glm::vec3(m_target->GetX(), m_target->GetY(), m_target->GetZ()));
					}
				}
				else if (!get_exempt_status(Port)) {
					if (speed > (runs * 1.5)) {
						cheat_detected(MQWarp, glm::vec3(m_target->GetX(), m_target->GetY(), m_target->GetZ()));
						m_time_since_last_position_check = cur_time;
						m_distance_since_last_position_check = 0.0f;
					}
					else {
						cheat_detected(MQWarpLight, glm::vec3(m_target->GetX(), m_target->GetY(), m_target->GetZ()));
					}
				}
			}
		}
		if (time_between_checks != 1000) {
			set_exempt_status(ShadowStep, false);
			set_exempt_status(KnockBack, false);
			set_exempt_status(Port, false);
		}
		m_time_since_last_position_check = cur_time;
		m_distance_since_last_position_check = 0.0f;
	}
}

void anti_cheat::start_mem_check() {
	m_time_since_last_memorization = Timer::GetCurrentTime();
}

void anti_cheat::restart_mem_check() {
	if (m_target == nullptr)
		return;
	if (m_time_since_last_memorization - Timer::GetCurrentTime() <= 1) {
		glm::vec3 pos = m_target->GetPosition();
		cheat_detected(MQFastMem, pos);
	}
	m_time_since_last_memorization = Timer::GetCurrentTime();
}

void anti_cheat::process_movement_history(const EQApplicationPacket* app) {
	if (get_exempt_status(Port))
		return;
	UpdateMovementEntry* m_MovementHistory = (UpdateMovementEntry*)app->pBuffer;
	for (int index = 0; index < (app->size) / sizeof(UpdateMovementEntry); index++) {
		switch (m_MovementHistory[index].type) {
		case UpdateMovementType::ZoneLine:
			set_exempt_status(Port, true);
			// break from the switch case
			break;
		case UpdateMovementType::TeleportA:
			if (index != 0)
				cheat_detected(MQWarpAbsolute,
					glm::vec3(m_MovementHistory[index - 1].X, m_MovementHistory[index - 1].Y, m_MovementHistory[index - 1].Z),
					glm::vec3(m_MovementHistory[index].X, m_MovementHistory[index].Y, m_MovementHistory[index].Z)
				);
			set_exempt_status(Port, false);
			break;
		}
	}
}

void anti_cheat::client_process() {
	if (!m_cheat_detect_moved) {
		m_time_since_last_position_check = Timer::GetCurrentTime();
	}
}
