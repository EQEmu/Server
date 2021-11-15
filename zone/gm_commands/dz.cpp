#include "../client.h"
#include "../expedition.h"

void command_dz(Client *c, const Seperator *sep)
{
	if (!c || !zone) {
		return;
	}

	if (strcasecmp(sep->arg[1], "cache") == 0) {
		if (strcasecmp(sep->arg[2], "reload") == 0) {
			DynamicZone::CacheAllFromDatabase();
			Expedition::CacheAllFromDatabase();
			c->Message(
				Chat::White, fmt::format(
					"Reloaded [{}] dynamic zone(s) and [{}] expedition(s) from database",
					zone->dynamic_zone_cache.size(), zone->expedition_cache.size()
				).c_str());
		}
	}
	else if (strcasecmp(sep->arg[1], "expedition") == 0) {
		if (strcasecmp(sep->arg[2], "list") == 0) {
			std::vector<Expedition *> expeditions;
			for (const auto           &expedition : zone->expedition_cache) {
				expeditions.emplace_back(expedition.second.get());
			}

			std::sort(
				expeditions.begin(), expeditions.end(),
				[](const Expedition *lhs, const Expedition *rhs) {
					return lhs->GetID() < rhs->GetID();
				}
			);

			c->Message(Chat::White, fmt::format("Total Active Expeditions: [{}]", expeditions.size()).c_str());
			for (const auto &expedition : expeditions) {
				auto dz = expedition->GetDynamicZone();
				if (!dz) {
					LogExpeditions("Expedition [{}] has an invalid dz [{}] in cache",
								   expedition->GetID(),
								   expedition->GetDynamicZoneID());
					continue;
				}

				auto leader_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
					fmt::format(
						"#goto {}", expedition->GetLeaderName()), false, expedition->GetLeaderName());
				auto zone_saylink   = EQ::SayLinkEngine::GenerateQuestSaylink(
					fmt::format(
						"#zoneinstance {}", dz->GetInstanceID()), false, "zone"
				);

				auto seconds = dz->GetSecondsRemaining();

				c->Message(
					Chat::White, fmt::format(
						"expedition id: [{}] dz id: [{}] name: [{}] leader: [{}] {}: [{}]:[{}]:[{}]:[{}] members: [{}] remaining: [{:02}:{:02}:{:02}]",
						expedition->GetID(),
						expedition->GetDynamicZoneID(),
						expedition->GetName(),
						leader_saylink,
						zone_saylink,
						ZoneName(dz->GetZoneID()),
						dz->GetZoneID(),
						dz->GetInstanceID(),
						dz->GetZoneVersion(),
						dz->GetMemberCount(),
						seconds / 3600,      // hours
						(seconds / 60) % 60, // minutes
						seconds % 60         // seconds
					).c_str());
			}
		}
		else if (strcasecmp(sep->arg[2], "reload") == 0) {
			Expedition::CacheAllFromDatabase();
			c->Message(
				Chat::White, fmt::format(
					"Reloaded [{}] expeditions to cache from database.", zone->expedition_cache.size()
				).c_str());
		}
		else if (strcasecmp(sep->arg[2], "destroy") == 0 && sep->IsNumber(3)) {
			auto expedition_id = std::strtoul(sep->arg[3], nullptr, 10);
			auto expedition    = Expedition::FindCachedExpeditionByID(expedition_id);
			if (expedition) {
				c->Message(
					Chat::White, fmt::format(
						"Destroying expedition [{}] ({})",
						expedition_id, expedition->GetName()).c_str());
				expedition->GetDynamicZone()->RemoveAllMembers();
			}
			else {
				c->Message(Chat::Red, fmt::format("Failed to destroy expedition [{}]", sep->arg[3]).c_str());
			}
		}
		else if (strcasecmp(sep->arg[2], "unlock") == 0 && sep->IsNumber(3)) {
			auto expedition_id = std::strtoul(sep->arg[3], nullptr, 10);
			auto expedition    = Expedition::FindCachedExpeditionByID(expedition_id);
			if (expedition) {
				c->Message(Chat::White, fmt::format("Unlocking expedition [{}]", expedition_id).c_str());
				expedition->SetLocked(false, ExpeditionLockMessage::None, true);
			}
			else {
				c->Message(Chat::Red, fmt::format("Failed to find expedition [{}]", sep->arg[3]).c_str());
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "list") == 0) {
		c->Message(
			Chat::White,
			fmt::format("Total Dynamic Zones (cache): [{}]", zone->dynamic_zone_cache.size()).c_str());

		std::vector<DynamicZone *> dynamic_zones;
		for (const auto            &dz : zone->dynamic_zone_cache) {
			dynamic_zones.emplace_back(dz.second.get());
		}

		std::sort(
			dynamic_zones.begin(), dynamic_zones.end(),
			[](const DynamicZone *lhs, const DynamicZone *rhs) {
				return lhs->GetID() < rhs->GetID();
			}
		);

		for (const auto &dz : dynamic_zones) {
			auto seconds      = dz->GetSecondsRemaining();
			auto zone_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
				fmt::format("#zoneinstance {}", dz->GetInstanceID()), false, "zone"
			);

			std::string aligned_type = fmt::format(
				"[{}]",
				DynamicZone::GetDynamicZoneTypeName(static_cast<DynamicZoneType>(dz->GetType())));
			c->Message(
				Chat::White, fmt::format(
					"id: [{}] type: {:>10} {}: [{}]:[{}]:[{}] members: [{}] remaining: [{:02}:{:02}:{:02}]",
					dz->GetID(),
					aligned_type,
					zone_saylink,
					dz->GetZoneID(),
					dz->GetInstanceID(),
					dz->GetZoneVersion(),
					dz->GetMemberCount(),
					seconds / 3600,      // hours
					(seconds / 60) % 60, // minutes
					seconds % 60         // seconds
				).c_str());
		}
	}
	else if (strcasecmp(sep->arg[1], "listdb") == 0) {
		auto dz_list = DynamicZonesRepository::AllDzInstancePlayerCounts(database);
		c->Message(Chat::White, fmt::format("Total Dynamic Zones (database): [{}]", dz_list.size()).c_str());

		auto now = std::chrono::system_clock::now();

		for (const auto &dz : dz_list) {
			auto expire_time = std::chrono::system_clock::from_time_t(dz.start_time + dz.duration);
			auto remaining   = std::chrono::duration_cast<std::chrono::seconds>(expire_time - now);
			auto seconds     = std::max(0, static_cast<int>(remaining.count()));
			bool is_expired  = now > expire_time;

			if (!is_expired || strcasecmp(sep->arg[2], "all") == 0) {
				auto zone_saylink = is_expired ? "zone" : EQ::SayLinkEngine::GenerateQuestSaylink(
					fmt::format("#zoneinstance {}", dz.instance), false, "zone"
				);

				c->Message(
					Chat::White, fmt::format(
						"id: [{}] type: [{}] {}: [{}]:[{}]:[{}] members: [{}] remaining: [{:02}:{:02}:{:02}]",
						dz.id,
						DynamicZone::GetDynamicZoneTypeName(static_cast<DynamicZoneType>(dz.type)),
						zone_saylink,
						dz.zone,
						dz.instance,
						dz.version,
						dz.member_count,
						seconds / 3600,      // hours
						(seconds / 60) % 60, // minutes
						seconds % 60         // seconds
					).c_str());
			}
		}
	}
	else if (strcasecmp(sep->arg[1], "lockouts") == 0) {
		if (strcasecmp(sep->arg[2], "remove") == 0 && sep->arg[3][0] != '\0') {
			if (sep->arg[5][0] == '\0') {
				c->Message(
					Chat::White, fmt::format(
						"Removing [{}] lockouts on [{}].", sep->arg[4][0] ? sep->arg[4] : "all", sep->arg[3]
					).c_str());
			}
			else {
				c->Message(
					Chat::White, fmt::format(
						"Removing [{}]:[{}] lockout on [{}].", sep->arg[4], sep->arg[5], sep->arg[3]
					).c_str());
			}
			Expedition::RemoveLockoutsByCharacterName(sep->arg[3], sep->arg[4], sep->arg[5]);
		}
	}
	else if (strcasecmp(sep->arg[1], "makeleader") == 0 && sep->IsNumber(2) && sep->arg[3][0] != '\0') {
		auto expedition_id = std::strtoul(sep->arg[2], nullptr, 10);
		auto expedition    = Expedition::FindCachedExpeditionByID(expedition_id);
		if (expedition) {
			auto char_name = FormatName(sep->arg[3]);
			c->Message(
				Chat::White,
				fmt::format("Setting expedition [{}] leader to [{}]", expedition_id, char_name).c_str());
			expedition->SendWorldMakeLeaderRequest(c->CharacterID(), char_name);
		}
		else {
			c->Message(Chat::Red, fmt::format("Failed to find expedition [{}]", expedition_id).c_str());
		}
	}
	else {
		c->Message(Chat::White, "#dz usage:");
		c->Message(
			Chat::White,
			"#dz cache reload - reload the current zone cache from db (also reloads expedition cache dependency)"
		);
		c->Message(Chat::White, "#dz expedition list - list expeditions in current zone cache");
		c->Message(Chat::White, "#dz expedition reload - reload expedition zone cache from database");
		c->Message(
			Chat::White,
			"#dz expedition destroy <expedition_id> - destroy expedition globally (must be in cache)"
		);
		c->Message(Chat::White, "#dz expedition unlock <expedition_id> - unlock expedition");
		c->Message(Chat::White, "#dz list - list all dynamic zone instances from current zone cache");
		c->Message(
			Chat::White,
			"#dz listdb [all] - list dynamic zone instances from database -- 'all' includes expired"
		);
		c->Message(Chat::White, "#dz lockouts remove <char_name> - delete all of character's expedition lockouts");
		c->Message(
			Chat::White,
			"#dz lockouts remove <char_name> \"<expedition_name>\" - delete lockouts by expedition"
		);
		c->Message(
			Chat::White,
			"#dz lockouts remove <char_name> \"<expedition_name>\" \"<event_name>\" - delete lockout by expedition event"
		);
		c->Message(Chat::White, "#dz makeleader <expedition_id> <character_name> - set new expedition leader");
	}
}

