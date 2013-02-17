CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(sharedmem_src
    DLLMain.cpp
    Doors.cpp
    Items.cpp
    Loot.cpp
    MMF.cpp
    MMFMutex.cpp
    NPCFactionLists.cpp
    Opcodes.cpp
    SkillCaps.cpp
    Spells.cpp
)

SET(sharedmem_headers
    Doors.h
    Items.h
    Loot.h
    MMF.h
    MMFMutex.h
    NPCFactionLists.h
    Opcodes.h
    SkillCaps.h
    Spells.h
)

SET(EQEMU_MAX_ITEMS 300000 CACHE STRING "Maxium number of items to load into memory. Make sure this is bigger than the total number of items in the server database")
SET(EQEMU_MAX_DOORS 30000 CACHE STRING "Maxium number of doors to load into memory. Make sure this is bigger than the total number of doors in the server database")
SET(EQEMU_MAX_FACTIONLIST_IDS 50000 CACHE STRING "Maxium number of FactionList IDs to load into memory. Make sure this is bigger than the total number of FactionList IDs in the server database")

ADD_DEFINITIONS(-DMMF_EQMAX_ITEMS=${EQEMU_MAX_ITEMS})
ADD_DEFINITIONS(-DMMF_MAX_Door_ID=${EQEMU_MAX_DOORS})
ADD_DEFINITIONS(-DMMF_MAX_NPCFactionList_ID=${EQEMU_MAX_FACTIONLIST_IDS})

ADD_LIBRARY(EMuShareMem SHARED ${sharedmem_src} ${sharedmem_headers})
TARGET_LINK_LIBRARIES(EMuShareMem Common)

IF(UNIX)
    TARGET_LINK_LIBRARIES(EMuShareMem "dl")
    TARGET_LINK_LIBRARIES(EMuShareMem "m")
    TARGET_LINK_LIBRARIES(EMuShareMem "rt")
    TARGET_LINK_LIBRARIES(EMuShareMem "pthread")
ENDIF(UNIX)

SET(LIBRARY_OUTPUT_PATH ../Bin)
