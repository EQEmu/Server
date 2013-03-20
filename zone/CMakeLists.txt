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
    doors.cpp
    effects.cpp
    embparser.cpp
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
    net.cpp
    npc.cpp
    NpcAI.cpp
    Object.cpp
    parser.cpp
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
    perlparser.cpp
    petitions.cpp
    pets.cpp
    PlayerCorpse.cpp
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
    updatemgr.cpp
    watermap.cpp
    waypoints.cpp
    worldserver.cpp
    zone.cpp
    zone_logsys.cpp
    zone_profile.cpp
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
    parser.h
    pathing.h
    perlpacket.h
    perlparser.h
    petitions.h
    pets.h
    PlayerCorpse.h
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
    updatemgr.h
    watermap.h
    worldserver.h
    zone.h
    zone_profile.h
    ZoneConfig.h
    zonedb.h
    zonedbasync.h
    zonedump.h
)

ADD_EXECUTABLE(zone ${zone_sources} ${zone_headers})

ADD_DEFINITIONS(-DZONE)

TARGET_LINK_LIBRARIES(zone Common ${PERL_LIBRARY} debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE} ${ZLIB_LIBRARY} ${VLD_LIBRARY_VLD})

IF(MSVC)
    SET_TARGET_PROPERTIES(zone PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
    TARGET_LINK_LIBRARIES(zone "Ws2_32.lib")
ENDIF(MSVC)
SET(VLD_FOUND FALSE)

## VLD works only in Microsoft Visual Studio on Windows:
IF(WIN32)
IF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv)")

FIND_PATH(VLD_INCLUDE_DIR vld.h
 $ENV{VLD_HOME}/include
 $ENV{VLD_HOME}
 "$ENV{ProgramFiles}/Microsoft Visual Studio .NET 2003/Vc7/include"
 )
#MESSAGE("VLD_INCLUDE_DIR=${VLD_INCLUDE_DIR}")

## this is just required for runtim enabling/disabling: 
FIND_PATH(VLD_INCLUDE_DIR_API vldapi.h
 $ENV{VLD_HOME}/include
 $ENV{VLD_HOME}
 "$ENV{ProgramFiles}/Microsoft Visual Studio .NET 2003/Vc7/include"
)
#MESSAGE("VLD_INCLUDE_DIR_API=${VLD_INCLUDE_DIR}")

FIND_LIBRARY(VLD_LIBRARY_VLD
 NAMES vld
 PATHS 
 $ENV{VLD_HOME}/lib
 $ENV{VLD_HOME}
 "$ENV{ProgramFiles}/Microsoft Visual Studio .NET 2003/Vc7/lib"
 DOC "Visual Leak Debugger multithreaded library"
 )
#MESSAGE("VLD_LIBRARY_VLD=${VLD_LIBRARY_VLD}")

FIND_LIBRARY(VLD_LIBRARY_VLDMT
 NAMES vldmt
 PATHS 
 $ENV{VLD_HOME}/lib
 $ENV{VLD_HOME}
 "$ENV{ProgramFiles}/Microsoft Visual Studio .NET 2003/Vc7/lib"
 DOC "Visual Leak Debugger multithreaded library"
 )
#MESSAGE("VLD_LIBRARY_VLDMT=${VLD_LIBRARY_VLDMT}")

FIND_LIBRARY(VLD_LIBRARY_VLDMTDLL
 NAMES vldmtdll
 PATHS 
 $ENV{VLD_HOME}/lib
 $ENV{VLD_HOME}
 "$ENV{ProgramFiles}/Microsoft Visual Studio .NET 2003/Vc7/lib"
 DOC "Visual Leak Debugger multithreaded library"
 )
#MESSAGE("VLD_LIBRARY_VLDMTDLL=${VLD_LIBRARY_VLDMTDLL}")

IF(VLD_INCLUDE_DIR)
 IF (VLD_LIBRARY_VLD OR VLD_LIBRARY_VLDMT OR VLD_LIBRARY_VLDMTDLL)
   ## do NOT add LIBRARIES because dependencies are handled through pragmas.
   SET(VLD_FOUND TRUE)
   # MESSAGE("found VLD.")
 ENDIF (VLD_LIBRARY_VLD OR VLD_LIBRARY_VLDMT OR VLD_LIBRARY_VLDMTDLL)
ENDIF(VLD_INCLUDE_DIR)

MARK_AS_ADVANCED(
 VLD_INCLUDE_DIR
 VLD_INCLUDE_DIR_API
 VLD_LIBRARY_VLD
 VLD_LIBRARY_VLDMT
 VLD_LIBRARY_VLDMTDLL
 VLD_LIBRARIES
)

ENDIF(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv)")
ENDIF(WIN32)

IF(MINGW)
    TARGET_LINK_LIBRARIES(zone "WS2_32")
ENDIF(MINGW)

IF(UNIX)
    TARGET_LINK_LIBRARIES(zone "dl")
    TARGET_LINK_LIBRARIES(zone "z")
    TARGET_LINK_LIBRARIES(zone "m")
    TARGET_LINK_LIBRARIES(zone "rt")
    TARGET_LINK_LIBRARIES(zone "pthread")
    ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

INCLUDE_DIRECTORIES(${VLD_INCLUDE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ../Bin)
