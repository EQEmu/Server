#include "cheat_manager.h"
#include "client.h"
#include "quest_parser_collection.h"

void CheatManager::SetClient(Client *cli)
{
	m_target = cli;
}

void CheatManager::SetExemptStatus(ExemptionType type, bool v)
{
	if (v == true) {
		MovementCheck();
	}
	m_exemption[type] = v;
}

bool CheatManager::GetExemptStatus(ExemptionType type)
{
	return m_exemption[type];
}

void CheatManager::CheatDetected(CheatTypes type, glm::vec3 position1, glm::vec3 position2)
{
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
					Distance(position1, position2)
				);
				database.SetMQDetectionFlag(
					m_target->AccountName(),
					m_target->GetName(),
					message.c_str(),
					zone->GetShortName()
				);
				LogCheat(message);
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
					Distance(position1, position2)
				);
				database.SetMQDetectionFlag(
					m_target->AccountName(),
					m_target->GetName(),
					message.c_str(),
					zone->GetShortName()
				);
				LogCheat(message);
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
				database.SetMQDetectionFlag(
					m_target->AccountName(),
					m_target->GetName(),
					message.c_str(),
					zone->GetShortName()
				);
				LogCheat(message);
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
				database.SetMQDetectionFlag(
					m_target->AccountName(),
					m_target->GetName(),
					message.c_str(),
					zone->GetShortName()
				);
				LogCheat(message);
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
					database.SetMQDetectionFlag(
						m_target->AccountName(),
						m_target->GetName(),
						message.c_str(),
						zone->GetShortName()
					);
					LogCheat(message);
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
				database.SetMQDetectionFlag(
					m_target->AccountName(),
					m_target->GetName(),
					message.c_str(),
					zone->GetShortName()
				);
				LogCheat(message);
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
				database.SetMQDetectionFlag(
					m_target->AccountName(),
					m_target->GetName(),
					message.c_str(),
					zone->GetShortName());
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
				database.SetMQDetectionFlag(
					m_target->AccountName(),
					m_target->GetName(),
					message.c_str(),
					zone->GetShortName()
				);
				LogCheat(message);
			}
			break;
		case MQGhost:
			if (RuleB(Zone, EnableMQGhostDetector) &&
				((m_target->Admin() < RuleI(Zone, MQGhostExemptStatus) || (RuleI(Zone, MQGhostExemptStatus)) == -1))) {
				database.SetMQDetectionFlag(
					m_target->AccountName(),
					m_target->GetName(),
					"/MQGhost",
					zone->GetShortName());
				LogCheat("/MQGhost");
			}
			break;
		case MQFastMem:
			if (RuleB(Zone, EnableMQFastMemDetector) &&
				((m_target->Admin() < RuleI(Zone, MQFastMemExemptStatus) ||
				  (RuleI(Zone, MQFastMemExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQFastMem used at x [{:.2f}] y [{:.2f}] z [{:.2f}]",
					position1.x,
					position1.y,
					position1.z
				);
				database.SetMQDetectionFlag(
					m_target->AccountName(),
					m_target->GetName(),
					message.c_str(),
					zone->GetShortName()
				);
				LogCheat(message);
			}
			break;
		default:
			std::string message = fmt::format(
				"Unhandled HackerDetection flag with location from x [{:.2f}] y [{:.2f}] z [{:.2f}]",
				position1.x,
				position1.y,
				position1.z
			);
			database.SetMQDetectionFlag(
				m_target->AccountName(),
				m_target->GetName(),
				message.c_str(),
				zone->GetShortName()
			);
			LogCheat(message);
			break;
	}
}

void CheatManager::MovementCheck(glm::vec3 updated_position)
{
	if (m_time_since_last_movement_history.GetRemainingTime() == 0) {
		CheatDetected(MQWarp, updated_position);
	}    // someone is spoofing.

	float  dist     = DistanceNoZ(m_target->GetPosition(), updated_position);
	uint32 cur_time = Timer::GetCurrentTime();
	if (dist == 0) {
		if (m_distance_since_last_position_check > 0.0f) {
			MovementCheck(0);
		}
		else {
			m_time_since_last_position_check = cur_time;
			m_cheat_detect_moved             = false;
		}
	}
	else {
		m_distance_since_last_position_check += dist;
		m_cheat_detect_moved = true;
		if (m_time_since_last_position_check == 0) {
			m_time_since_last_position_check = cur_time;
		}
		else {
			MovementCheck(2500);
		}
	}
}

void CheatManager::MovementCheck(uint32 time_between_checks)
{
	uint32 cur_time = Timer::GetCurrentTime();
	if ((cur_time - m_time_since_last_position_check) > time_between_checks) {
		float speed =
				  (m_distance_since_last_position_check * 100) / (float) (cur_time - m_time_since_last_position_check);
		int   runs  = m_target->GetRunspeed() / std::min(RuleR(Zone, MQWarpDetectionDistanceFactor), 1.0f);
		if (speed > runs) {
			if (!m_target->GetGMSpeed() && (runs >= m_target->GetBaseRunspeed() || (speed >
																					(m_target->GetBaseRunspeed() /
																					 std::min(
																						 RuleR(Zone,
																							   MQWarpDetectionDistanceFactor),
																						 1.0f
																					 ))))) {
				if (GetExemptStatus(ShadowStep)) {
					if (m_distance_since_last_position_check > 800) {
						CheatDetected(
							MQWarpShadowStep,
							glm::vec3(
								m_target->GetX(),
								m_target->GetY(),
								m_target->GetZ()
							)
						);
					}
				}
				else if (GetExemptStatus(KnockBack)) {
					if (speed > 30.0f) {
						CheatDetected(MQWarpKnockBack, glm::vec3(m_target->GetX(), m_target->GetY(), m_target->GetZ()));
					}
				}
				else if (!GetExemptStatus(Port)) {
					if (speed > (runs * 1.5)) {
						CheatDetected(MQWarp, glm::vec3(m_target->GetX(), m_target->GetY(), m_target->GetZ()));
						m_time_since_last_position_check     = cur_time;
						m_distance_since_last_position_check = 0.0f;
					}
					else {
						CheatDetected(MQWarpLight, glm::vec3(m_target->GetX(), m_target->GetY(), m_target->GetZ()));
					}
				}
			}
		}
		if (time_between_checks != 1000) {
			SetExemptStatus(ShadowStep, false);
			SetExemptStatus(KnockBack, false);
			SetExemptStatus(Port, false);
		}
		m_time_since_last_position_check     = cur_time;
		m_distance_since_last_position_check = 0.0f;
	}
}

void CheatManager::CheckMemTimer()
{
	if (m_target == nullptr) {
		return;
	}
	if (m_time_since_last_memorization - Timer::GetCurrentTime() <= 1) {
		glm::vec3 pos = m_target->GetPosition();
		CheatDetected(MQFastMem, pos);
	}
	m_time_since_last_memorization = Timer::GetCurrentTime();
}

void CheatManager::ProcessMovementHistory(const EQApplicationPacket *app)
{
	// if they haven't sent sent the packet within this time... they are probably spoofing...
	// linux users reported that they don't send this packet at all but i can't prove they don't so i'm not sure if thats a fake or not.
	m_time_since_last_movement_history.Start(70000);
	if (GetExemptStatus(Port)) {
		return;
	}
	auto *m_MovementHistory = (UpdateMovementEntry *) app->pBuffer;
	if (app->size < sizeof(UpdateMovementEntry))
	{
		LogDebug("Size mismatch in OP_MovementHistoryList, expected {}, got [{}]", sizeof(UpdateMovementEntry), app->size);
		DumpPacket(app);
		return;
	}

	for (int index = 0; index < (app->size) / sizeof(UpdateMovementEntry); index++) {
		glm::vec3 to = glm::vec3(m_MovementHistory[index].X, m_MovementHistory[index].Y, m_MovementHistory[index].Z);
		switch (m_MovementHistory[index].type) {
			case UpdateMovementType::ZoneLine:
				SetExemptStatus(Port, true);
				break;
			case UpdateMovementType::TeleportA:
				if (index != 0) {
					glm::vec3 from = glm::vec3(
						m_MovementHistory[index - 1].X,
						m_MovementHistory[index - 1].Y,
						m_MovementHistory[index - 1].Z
					);
					CheatDetected(MQWarpAbsolute, from, to);
				}
				SetExemptStatus(Port, false);
				break;
		}
	}
}

void CheatManager::ProcessSpawnApperance(uint16 spawn_id, uint16 type, uint32 parameter)
{
	if (type == AT_Anim && parameter == ANIM_SIT) {
		m_time_since_last_memorization = Timer::GetCurrentTime();
	}
	else if (spawn_id == 0 && type == AT_AntiCheat) {
		m_time_since_last_action = parameter;
	}
}

void CheatManager::ProcessItemVerifyRequest(int32 slot_id, uint32 target_id)
{
	if (slot_id == -1 && m_warp_counter != target_id) {
		m_warp_counter = target_id;
	}
}

void CheatManager::ClientProcess()
{
	if (!m_cheat_detect_moved) {
		m_time_since_last_position_check = Timer::GetCurrentTime();
	}
}
