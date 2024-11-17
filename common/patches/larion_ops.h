
//list of packets we need to encode on the way out:
E(OP_LogServer)
E(OP_SendMembership)
E(OP_SendMembershipDetails)
E(OP_SendMaxCharacters)
E(OP_SendCharInfo)
E(OP_ExpansionInfo)
E(OP_SpawnAppearance)
//E(OP_SendAATable)
E(OP_PlayerProfile)
E(OP_ZoneEntry)
E(OP_ZoneSpawns)
E(OP_CharInventory)
E(OP_NewZone)
E(OP_SpawnDoor)
E(OP_GroundSpawn)

//list of packets we need to decode on the way in:
D(OP_ZoneEntry)

#undef E
#undef D
