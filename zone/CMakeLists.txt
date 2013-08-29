CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(zone_sources
	AA.cpp
	aggro.cpp
	attack.cpp
	beacon.cpp
	bonuses.cpp
	bot.cpp
	botspellsai.cpp
	client.cpp
	client_logs.cpp
	client_mods.cpp
	client_packet.cpp
	client_process.cpp
	command.cpp
	corpse.cpp
	doors.cpp
	effects.cpp
	embparser.cpp
	embparser_api.cpp
	embperl.cpp
	embxs.cpp
	entity.cpp
	exp.cpp
	fearpath.cpp
	forage.cpp
	groups.cpp
	guild.cpp
	guild_mgr.cpp
	hate_list.cpp
	horse.cpp
	inventory.cpp
	loottables.cpp
	lua_bit.cpp
	lua_corpse.cpp
	lua_client.cpp
	lua_door.cpp
	lua_entity.cpp
	lua_entity_list.cpp
	lua_general.cpp
	lua_group.cpp
	lua_hate_list.cpp
	lua_inventory.cpp
	lua_item.cpp
	lua_iteminst.cpp
	lua_mob.cpp
	lua_npc.cpp
	lua_object.cpp
	lua_packet.cpp
	lua_parser.cpp
	lua_parser_events.cpp
	lua_raid.cpp
	lua_spawn.cpp
	lua_spell.cpp
	embperl.cpp
	embxs.cpp
	entity.cpp
	exp.cpp
	fearpath.cpp
	forage.cpp
	groups.cpp
	guild.cpp
	guild_mgr.cpp
	hate_list.cpp
	horse.cpp
	inventory.cpp
	loottables.cpp
	Map.cpp
	merc.cpp
	mob.cpp
	MobAI.cpp
	mod_functions.cpp
	net.cpp
	npc.cpp
	NpcAI.cpp
	Object.cpp
	pathing.cpp
	perl_client.cpp
	perl_doors.cpp
	perl_entity.cpp
	perl_groups.cpp
	perl_hateentry.cpp
	perl_mob.cpp
	perl_npc.cpp
	perl_object.cpp
	perl_perlpacket.cpp
	perl_PlayerCorpse.cpp
	perl_questitem.cpp
	perl_raids.cpp
	perlpacket.cpp
	petitions.cpp
	pets.cpp
	QGlobals.cpp
	questmgr.cpp
	QuestParserCollection.cpp
	raids.cpp
	spawn2.cpp
	spawn2.h
	spawngroup.cpp
	special_attacks.cpp
	spell_effects.cpp
	spells.cpp
	tasks.cpp
	titles.cpp
	tradeskills.cpp
	trading.cpp
	trap.cpp
	tribute.cpp
	watermap.cpp
	waypoints.cpp
	worldserver.cpp
	zone.cpp
	zone_logsys.cpp
	ZoneConfig.cpp
	zonedb.cpp
	zonedbasync.cpp
	zoning.cpp
)

SET(zone_headers
	AA.h
	basic_functions.h
	beacon.h
	bot.h
	botStructs.h
	client.h
	client_logs.h
	client_packet.h
	command.h
	common.h
	corpse.h
	doors.h
	embparser.h
	embperl.h
	embxs.h
	entity.h
	errmsg.h
	event_codes.h
	forage.h
	groups.h
	guild_mgr.h
	hate_list.h
	horse.h
	lua_bit.h
	lua_client.h
	lua_corpse.h
	lua_entity.h
	lua_entity_list.h
	lua_general.h
	lua_group.h
	lua_hate_list.h
	lua_inventory.h
	lua_item.h
	lua_iteminst.h
	lua_mob.h
	lua_npc.h
	lua_object.h
	lua_packet.h
	lua_parser.h
	lua_parser_events.h
	lua_ptr.h
	lua_raid.h
	lua_spawn.h
	lua_spell.h
	map.h
	masterentity.h
	maxskill.h
	message.h
	merc.h
	mob.h
	net.h
	npc.h
	NpcAI.h
	object.h
	pathing.h
	perlpacket.h
	petitions.h
	pets.h
	QGlobals.h
	QuestInterface.h
	questmgr.h
	QuestParserCollection.h
	raid.h
	raids.h
	skills.h
	spawn2.cpp
	spawn2.h
	spawngroup.h
	StringIDs.h
	tasks.h
	titles.h
	trap.h
	watermap.h
	worldserver.h
	zone.h
	ZoneConfig.h
	zonedb.h
	zonedbasync.h
	zonedump.h
)

ADD_EXECUTABLE(zone ${zone_sources} ${zone_headers})

INSTALL(TARGETS zone RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

ADD_DEFINITIONS(-DZONE)

TARGET_LINK_LIBRARIES(zone Common ${PERL_LIBRARY} debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY} ${LUA_LIBRARY})

IF(EQEMU_BUILD_LUA)
	TARGET_LINK_LIBRARIES(zone luabind)
ENDIF(EQEMU_BUILD_LUA)


IF(MSVC)
	SET_TARGET_PROPERTIES(zone PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
	TARGET_LINK_LIBRARIES(zone "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
	TARGET_LINK_LIBRARIES(zone "WS2_32")
ENDIF(MINGW)

IF(UNIX)
	IF(NOT FREEBSD)
		TARGET_LINK_LIBRARIES(zone "dl")
	ENDIF(NOT FREEBSD)
	TARGET_LINK_LIBRARIES(zone "z")
	TARGET_LINK_LIBRARIES(zone "m")
	TARGET_LINK_LIBRARIES(zone "rt")
	TARGET_LINK_LIBRARIES(zone "pthread")
	ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/Bin)
