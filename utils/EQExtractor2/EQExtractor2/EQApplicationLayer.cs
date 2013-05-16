//
// Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net). Distributed under GPL version 2.
//
//
// The code in here should all be independent of the version of the EQ client.
//
// The EQStreamProcessor class will call the relevant Patch Specific decoder to return the required data in order to build SQL
//


using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using SharpPcap;
using zlib;
using MyUtils;
using EQExtractor2.InternalTypes;
using EQExtractor2.Patches;

using EQPacket;

namespace EQApplicationLayer
{
    public delegate void LogHandler(string Message);

    class EQStreamProcessor
    {
        // PatchSpecificDecoder is the base, Dummy' class from which the actual supported patch version decoders inherit.
        // Setting PatchDecoder to this is a failsafe in case the stream isn't identified.
        public PatchSpecficDecoder PatchDecoder = new PatchSpecficDecoder();

        List<ZonePoint> ZonePointList;

        // The PacketManager decodes the raw stream into EQApplictionPackets and holds the list of decoded packets for us to process
        public PacketManager Packets;

        // The PatchList is a list of each client version specific decoder
        List<PatchSpecficDecoder> PatchList;

        // EQStreamProcessor is the class that generates the SQL. It calls the relevant patch specific decoder to decode the packets
        // and return the relevant data in a standardized internal format.

        public EQStreamProcessor()
        {
            ZonePointList = null;

            Packets = new PacketManager();

            // Tell the PacketManager to call our Identify method to identify the packet stream. We will then call the different
            // patch specific Identifiers to identify the patch version.

            Packets.SetVersionIdentifierMethod(Identify);

            // Put our supported patch version decoders into the list.

            PatchList = new List<PatchSpecficDecoder>();

            PatchList.Add(new PatchMay122010Decoder());

            PatchList.Add(new PatchJuly132010Decoder());

            PatchList.Add(new PatchTestSep012010Decoder());

            PatchList.Add(new PatchTestSep222010Decoder());

            PatchList.Add(new PatchOct202010Decoder());

            PatchList.Add(new PatchDec072010Decoder());

            PatchList.Add(new PatchFeb082011Decoder());

            PatchList.Add(new PatchMarch152011Decoder());

            PatchList.Add(new PatchMay122011Decoder());

            PatchList.Add(new PatchMay242011Decoder());

            PatchList.Add(new PatchAug042011Decoder());

            PatchList.Add(new PatchNov172011Decoder());

            PatchList.Add(new PatchMar152012Decoder());

            PatchList.Add(new PatchJune252012Decoder());

            PatchList.Add(new PatchJuly132012Decoder());

            PatchList.Add(new PatchAugust152012Decoder());

            PatchList.Add(new PatchDecember102012Decoder());

            PatchList.Add(new PatchJanuary162013Decoder());

            PatchList.Add(new PatchTestServerJanuary162013Decoder());

            PatchList.Add(new PatchTestServerFebruary52013Decoder());

            PatchList.Add(new PatchFebruary112013Decoder());

            PatchList.Add(new PatchMarch132013Decoder());

            PatchList.Add(new PatchApril152013Decoder());

            PatchList.Add(new PatchSoD());

        }

        // This is called from the main form to tell us where the application was launched from (where to look for the .conf files)
        // and also gives us the LogHandler method so we can send debug messages to the main form.

        public bool Init(string ConfDirectory, LogHandler Logger)
        {
            // Pass the LogHandler on down to the PacketManager

            Packets.SetLogHandler(Logger);

            string ErrorMessage = "";

            // Here we init all the patch version specific decoders. The only reason one should fail to init is if it can't
            // find it's patch_XXXX.conf.
            //
            // If at least one initialises successfully, then return true  to the caller, otherwise return false
            bool AllDecodersFailed = true;

            PatchDecoder.Init(ConfDirectory, ref ErrorMessage);

            foreach (PatchSpecficDecoder p in PatchList)
            {
                Logger("Initialising patch " + p.GetVersion());
                if (!p.Init(ConfDirectory, ref ErrorMessage))
                    Logger(ErrorMessage);
                else
                    AllDecodersFailed = false;
            }

            return !AllDecodersFailed;
        }

        public string GetDecoderVersion()
        {
            // We don't need to check if PatchDecoder is null, because it is always initialised to an instance of the base
            // PatchSpecficDecoder class
            return PatchDecoder.GetVersion();
        }

        public bool StreamRecognised()
        {
            // Only the base PatchSpecficDecoder class returns false
            return !PatchDecoder.UnsupportedVersion();
        }

        public bool DumpPackets(string FileName, bool ShowTimeStamps)
        {
            return PatchDecoder.DumpPackets(FileName, ShowTimeStamps);
        }

        // This method is called by the PacketManager as it processes each packet in order to determine if the client patch
        // version is recognised.
        //
        // We call the Identify methods of each supported version decoder
        //
        // The decoders return No, if they cannot recognise the version from the given OpCode, size and direction,
        // 'Tentative' if they recognise it, but are not 100% sure it is their version, and Yes if they are 100%
        // sure the version has been identified.
        //
        public IdentificationStatus Identify(int OpCode, int Size, PacketDirection Direction)
        {
            IdentificationStatus Status = IdentificationStatus.No;

            foreach (PatchSpecficDecoder p in PatchList)
            {
                IdentificationStatus TempStatus = p.Identify(OpCode, Size, Direction);

                if (TempStatus == IdentificationStatus.Yes)
                {
                    // The version has been identified. Set PatchDecoder to point to this decoder
                    PatchDecoder = p;

                    return IdentificationStatus.Yes;
                }
                else if (TempStatus > Status)
                    Status = TempStatus;
            }
            return Status;
        }

        // This is called by the main form when all the packets have been processed. It prompts us to pass the packets down
        // to the decoder for this patch version.
        public void PCAPFileReadFinished()
        {
            PatchDecoder.GivePackets(Packets);
        }

        // The following methods are called by the main form and we just pass them on down to the decoder for the particular
        // patch version that the .pcap was produced with.

        public List<byte[]> GetPacketsOfType(string OpCodeName, PacketDirection Direction)
        {
            return PatchDecoder.GetPacketsOfType(OpCodeName, Direction);
        }

        public void ProcessPacket(System.Net.IPAddress srcIp, System.Net.IPAddress dstIp, ushort srcPort, ushort dstPort, byte[] Payload, DateTime PacketTime)
        {
            Packets.ProcessPacket(srcIp, dstIp, srcPort, dstPort, Payload, PacketTime, false, false);
        }

        public DateTime GetCaptureStartTime()
        {
            return PatchDecoder.GetCaptureStartTime();
        }

        public string GetZoneName()
        {
            return PatchDecoder.GetZoneName();
        }

        public string GetZoneLongName()
        {
            NewZoneStruct NewZone = PatchDecoder.GetZoneData();

            return NewZone.LongName;
        }

        public int VerifyPlayerProfile()
        {
            return PatchDecoder.VerifyPlayerProfile();
        }

        public UInt16 GetZoneNumber()
        {
            return PatchDecoder.GetZoneNumber();
        }

        //
        // The following are all the methods that actually generate the SQL. Again, we call the patch specific decoder's methods to
        // return the data we need in a version agnostic format.
        //

        public delegate void SQLDestination(string Message);

        public void GenerateDoorsSQL(string ZoneName, int DoorDBID, UInt32 SpawnVersion, SQLDestination SQLOut)
        {
            List<Door> DoorList = PatchDecoder.GetDoors();

            SQLOut("--");
            SQLOut("-- Doors");
            SQLOut("--");
            int UpperBound = (DoorDBID == 1 ? 998 : 999);
            SQLOut("DELETE from doors where zone = '" + ZoneName + "' and doorid >= @BaseDoorID and doorid <= @BaseDoorID + " + UpperBound + " and version = " + SpawnVersion + ";");

            foreach(Door d in DoorList)
            {
                if ((d.OpenType == 57) || (d.OpenType == 58))
                {
                    ZonePoint? zp = GetZonePointNumber(d.DoorParam);

                    if (zp != null)
                    {
                        d.DestZone = ZoneNumberToName(zp.Value.TargetZoneID);
                        d.DestX = zp.Value.TargetX;
                        d.DestY = zp.Value.TargetY;
                        d.DestZ = zp.Value.TargetZ;
                        d.DestHeading = zp.Value.Heading;

                    }
                }

                string DoorQuery = "INSERT INTO doors(`doorid`, `zone`, `version`, `name`, `pos_y`, `pos_x`, `pos_z`, `heading`, `opentype`, `doorisopen`, `door_param`, `dest_zone`, `dest_x`, `dest_y`, `dest_z`, `dest_heading`, `invert_state`, `incline`, `size`) VALUES(";
                DoorQuery += "@BaseDoorID + " + d.ID + ", '" + ZoneName + "', " + SpawnVersion + ", '" + d.Name + "', " + d.YPos + ", " + d.XPos + ", " + d.ZPos + ", " + d.Heading + ", " + d.OpenType + ", " + d.StateAtSpawn + ", " + d.DoorParam + ", '" + d.DestZone + "', " + d.DestX + ", " + d.DestY + ", " + d.DestZ + ", " + d.DestHeading + ", " + d.InvertState + ", " + d.Incline + ", " + d.Size + ");";

                SQLOut(DoorQuery);
            }
        }

        public void GenerateSpawnSQL(bool GenerateSpawns, bool GenerateGrids, bool GenerateMerchants,
                                            string ZoneName, UInt32 ZoneID, UInt32 SpawnVersion,
                                            bool UpdateExistingNPCTypes, bool UseNPCTypesTint, string SpawnNameFilter,
                                            bool CoalesceWaypoints, bool IncludeInvisibleMen, SQLDestination SQLOut)
        {
            UInt32 NPCTypeDBID = 0;
            UInt32 SpawnGroupID = 0;
            UInt32 SpawnEntryID = 0;
            UInt32 Spawn2ID = 0;
            UInt32 GridDBID = 0;
            UInt32 MerchantDBID = 0;

            List<ZoneEntryStruct> ZoneSpawns = PatchDecoder.GetSpawns();

            List<UInt32> FindableEntities = PatchDecoder.GetFindableSpawns();

            if (GenerateSpawns)
            {
                SQLOut("--");
                SQLOut("-- Spawns");
                SQLOut("--");
                if (SpawnVersion == 0)
                {
                    SQLOut("DELETE from npc_types where id >= @StartingNPCTypeID and id <= @StartingNPCTypeID + 999 and version = " + SpawnVersion + ";");

                    if(UseNPCTypesTint)
                        SQLOut("DELETE from npc_types_tint where id >= @StartingNPCTypeID and id <= @StartingNPCTypeID + 999;");

                    SQLOut("DELETE from spawngroup where id >= @StartingSpawnGroupID and id <= @StartingSpawnGroupID + 999;");
                    SQLOut("DELETE from spawnentry where spawngroupID >= @StartingSpawnEntryID and spawngroupID <= @StartingSpawnEntryID + 999;");
                    SQLOut("DELETE from spawn2 where id >= @StartingSpawn2ID and id <= @StartingSpawn2ID + 999 and version = " + SpawnVersion + ";");
                }
                else
                {
                    SQLOut("DELETE from npc_types where id >= @StartingNPCTypeID and id <= @StartingNPCTypeID + 99 and version = " + SpawnVersion + ";");
                    SQLOut("DELETE from spawngroup where id >= @StartingSpawnGroupID and id <= @StartingSpawnGroupID + 99;");
                    SQLOut("DELETE from spawnentry where spawngroupID >= @StartingSpawnEntryID and spawngroupID <= @StartingSpawnEntryID + 99;");
                    SQLOut("DELETE from spawn2 where id >= @StartingSpawn2ID and id <= @StartingSpawn2ID + 99 and version = " + SpawnVersion + ";");
                }

            }

            NPCTypeList NPCTL = new NPCTypeList();

            NPCSpawnList NPCSL = new NPCSpawnList();

            foreach(ZoneEntryStruct Spawn in ZoneSpawns)
            {
                if (NPCType.IsMount(Spawn.SpawnName))
                    continue;

                if (!IncludeInvisibleMen && (Spawn.Race == 127))
                    continue;

                Spawn.Findable = (FindableEntities.IndexOf(Spawn.SpawnID) >= 0);

                if (Spawn.IsNPC != 1)
                    continue;

                if (Spawn.PetOwnerID > 0)
                    continue;

                if ((SpawnNameFilter.Length > 0) && (Spawn.SpawnName.IndexOf(SpawnNameFilter) == -1))
                    continue;

                bool ColoursInUse = false;

                for (int ColourSlot = 0; ColourSlot < 9; ++ColourSlot)
                {
                    if (((Spawn.SlotColour[ColourSlot] & 0x00ffffff) != 0) && UseNPCTypesTint)
                        ColoursInUse = true;
                }

                if (Spawn.IsMercenary > 0)
                    continue;

                UInt32 ExistingDBID = NPCTL.FindNPCType(Spawn.SpawnName, Spawn.Level, Spawn.Gender, Spawn.Size, Spawn.Face, Spawn.WalkSpeed, Spawn.RunSpeed, Spawn.Race,
                       Spawn.BodyType, Spawn.HairColor, Spawn.BeardColor, Spawn.EyeColor1, Spawn.EyeColor2, Spawn.HairStyle, Spawn.Beard,
                       Spawn.DrakkinHeritage, Spawn.DrakkinTattoo, Spawn.DrakkinDetails, Spawn.Deity, Spawn.Class, Spawn.EquipChest2,
                       Spawn.Helm, Spawn.LastName);


                if (ExistingDBID == 0)
                {
                    NPCType NewNPCType = new NPCType(NPCTypeDBID, Spawn.SpawnName, Spawn.Level, Spawn.Gender, Spawn.Size, Spawn.Face, Spawn.WalkSpeed, Spawn.RunSpeed, Spawn.Race,
                       Spawn.BodyType, Spawn.HairColor, Spawn.BeardColor, Spawn.EyeColor1, Spawn.EyeColor2, Spawn.HairStyle, Spawn.Beard,
                       Spawn.DrakkinHeritage, Spawn.DrakkinTattoo, Spawn.DrakkinDetails, Spawn.Deity, Spawn.Class, Spawn.EquipChest2,
                       Spawn.Helm, Spawn.LastName, Spawn.Findable, Spawn.MeleeTexture1, Spawn.MeleeTexture2, Spawn.ArmorTintRed, Spawn.ArmorTintGreen, Spawn.ArmorTintBlue, Spawn.SlotColour);

                    NPCTL.AddNPCType(NewNPCType);

                    ExistingDBID = NPCTypeDBID++;

                    UInt32 ArmorTintID = 0;

                    if (ColoursInUse)
                    {
                        ArmorTintID = ExistingDBID;
                        Spawn.ArmorTintRed = 0;
                        Spawn.ArmorTintGreen = 0;
                        Spawn.ArmorTintBlue = 0;
                    }

                    string NPCTypesQuery = "INSERT INTO npc_types(`id`, `name`, `lastname`, `level`, `gender`, `size`, `runspeed`,`race`, `class`, `bodytype`, `hp`, `texture`, `helmtexture`, `face`, `luclin_hairstyle`, `luclin_haircolor`, `luclin_eyecolor`, `luclin_eyecolor2`,`luclin_beard`, `luclin_beardcolor`, `findable`, `version`, `d_meele_texture1`, `d_meele_texture2`, `armortint_red`, `armortint_green`, `armortint_blue`, `drakkin_heritage`, `drakkin_tattoo`, `drakkin_details`) VALUES(";

                    NPCTypesQuery += "@StartingNPCTypeID + " + ExistingDBID + ", '" + Spawn.SpawnName + "', " + "'" + Spawn.LastName + "', " + Spawn.Level + ", " + Spawn.Gender + ", " + Spawn.Size + ", ";
                    NPCTypesQuery += Spawn.RunSpeed + ", " + Spawn.Race + ", " + Spawn.Class + ", " + Spawn.BodyType + ", " + Spawn.Level * (10 + Spawn.Level) + ", ";
                    NPCTypesQuery += Spawn.EquipChest2 + ", " + Spawn.Helm + ", " + Spawn.Face + ", " + Spawn.HairStyle + ", " + Spawn.HairColor + ", " + Spawn.EyeColor1 + ", ";
                    NPCTypesQuery += Spawn.EyeColor2 + ", " + Spawn.Beard + ", " + Spawn.BeardColor + ", " + (Spawn.Findable ? 1 : 0) + ", " + SpawnVersion + ", ";
                    NPCTypesQuery += Spawn.MeleeTexture1 + ", " + Spawn.MeleeTexture2 + ", " + Spawn.ArmorTintRed + ", " + Spawn.ArmorTintGreen + ", " + Spawn.ArmorTintBlue + ", ";
                    NPCTypesQuery += Spawn.DrakkinHeritage + ", " + Spawn.DrakkinTattoo + ", " + Spawn.DrakkinDetails + ");";

                    if (GenerateSpawns)
                        SQLOut(NPCTypesQuery);

                    if (GenerateSpawns && ColoursInUse && NPCType.IsPlayableRace(Spawn.Race))
                    {
                        string TintQuery = "REPLACE INTO npc_types_tint(id, tint_set_name, ";
                        TintQuery += "red1h, grn1h, blu1h, ";
                        TintQuery += "red2c, grn2c, blu2c, ";
                        TintQuery += "red3a, grn3a, blu3a, ";
                        TintQuery += "red4b, grn4b, blu4b, ";
                        TintQuery += "red5g, grn5g, blu5g, ";
                        TintQuery += "red6l, grn6l, blu6l, ";
                        TintQuery += "red7f, grn7f, blu7f, ";
                        TintQuery += "red8x, grn8x, blu8x, ";
                        TintQuery += "red9x, grn9x, blu9x) values(@StartingNPCTypeID + " + ExistingDBID + ", '" + Spawn.SpawnName + "'";

                        for (int sc = 0; sc < 9; ++sc)
                        {
                            TintQuery += String.Format(", {0}, {1}, {2}", (Spawn.SlotColour[sc] >> 16) & 0xff,
                                                                           (Spawn.SlotColour[sc] >> 8) & 0xff,
                                                                           (Spawn.SlotColour[sc] & 0xff));
                        }

                        TintQuery += ");";
                        SQLOut(TintQuery);
                        SQLOut("UPDATE npc_types SET armortint_id = @StartingNPCTypeID + " + ArmorTintID + " WHERE id = @StartingNPCTypeID + " + ExistingDBID + " LIMIT 1;");
                    }

                }

                NPCSL.AddNPCSpawn(Spawn.SpawnID, Spawn2ID, ExistingDBID, Spawn.SpawnName);

                Position p = new Position(Spawn.XPos, Spawn.YPos, Spawn.ZPos, Spawn.Heading, DateTime.MinValue);

                NPCSL.AddWaypoint(Spawn.SpawnID, p, false);

                string SpawnGroupQuery = "INSERT INTO spawngroup(`id`, `name`, `spawn_limit`, `dist`, `max_x`, `min_x`, `max_y`, `min_y`, `delay`) VALUES(";
                SpawnGroupQuery += "@StartingSpawnGroupID + " + SpawnGroupID + ", CONCAT('" + ZoneName + "', @StartingSpawnGroupID + " + SpawnGroupID + "), 0, 0, 0, 0, 0, 0, 0);";

                string SpawnEntryQuery = "INSERT INTO spawnentry(`spawngroupID`, `npcID`, `chance`) VALUES(";
                SpawnEntryQuery += "@StartingSpawnEntryID + " + SpawnEntryID + ", @StartingNPCTypeID + " + ExistingDBID + ", " + "100);";

                string Spawn2EntryQuery = "INSERT INTO spawn2(`id`, `spawngroupID`, `zone`, `version`, `x`, `y`, `z`, `heading`, `respawntime`, `variance`, `pathgrid`, `_condition`, `cond_value`, `enabled`) VALUES(";
                Spawn2EntryQuery += "@StartingSpawn2ID + " + Spawn2ID + ", @StartingSpawnGroupID + " + SpawnGroupID + ", '" + ZoneName + "', " + SpawnVersion + ", " + Spawn.XPos + ", " + Spawn.YPos + ", " + Spawn.ZPos + ", ";
                Spawn2EntryQuery += Spawn.Heading + ", 640, 0, 0, 0, 1, 1);";

                SpawnGroupID++;
                SpawnEntryID++;
                Spawn2ID++;

                if (GenerateSpawns)
                {
                    SQLOut(SpawnGroupQuery);
                    SQLOut(SpawnEntryQuery);
                    SQLOut(Spawn2EntryQuery);
                }
            }

            if (UpdateExistingNPCTypes)
            {
                List<NPCType> UniqueSpawns = NPCTL.GetUniqueSpawns();

                foreach (NPCType n in UniqueSpawns)
                {
                    string UpdateQuery = "UPDATE npc_types set texture = {0}, helmtexture = {1}, size = {2}, face = {3}, luclin_hairstyle = {4}, ";
                    UpdateQuery += "luclin_haircolor = {5}, luclin_eyecolor = {6}, luclin_eyecolor2 = {7}, luclin_beardcolor = {8}, ";
                    UpdateQuery += "luclin_beard = {9}, drakkin_heritage = {10}, drakkin_tattoo = {11}, drakkin_details = {12}, ";
                    UpdateQuery += "armortint_red = {13}, armortint_green = {14}, armortint_blue = {15}, d_meele_texture1 = {16}, ";
                    UpdateQuery += "d_meele_texture2 = {17}, findable = {18}, gender = {20} where name = '{19}' and id >= @StartingNPCTypeID and id <= @StartingNPCTypeID + 999 and version = {21};";

                    SQLOut(String.Format(UpdateQuery, n.EquipChest2, n.Helm, n.Size, n.Face, n.HairStyle, n.HairColor, n.EyeColor1, n.EyeColor2, n.BeardColor,
                                                      n.Beard, n.DrakkinHeritage, n.DrakkinTattoo, n.DrakkinDetails, n.ArmorTintRed, n.ArmorTintGreen,
                                                      n.ArmorTintBlue, n.MeleeTexture1, n.MeleeTexture2, n.Findable, n.Name, n.Gender, SpawnVersion));

                    if(!NPCType.IsPlayableRace(n.Race))
                        continue;

                    bool ColoursInUse = false;

                    for (int ColourSlot = 0; ColourSlot < 9; ++ColourSlot)
                    {
                        if (((n.SlotColour[ColourSlot] & 0x00ffffff) != 0) && UseNPCTypesTint)
                            ColoursInUse = true;
                    }

                    if (ColoursInUse)
                    {
                        string TintQuery = "REPLACE INTO npc_types_tint(id, tint_set_name, ";
                        TintQuery += "red1h, grn1h, blu1h, ";
                        TintQuery += "red2c, grn2c, blu2c, ";
                        TintQuery += "red3a, grn3a, blu3a, ";
                        TintQuery += "red4b, grn4b, blu4b, ";
                        TintQuery += "red5g, grn5g, blu5g, ";
                        TintQuery += "red6l, grn6l, blu6l, ";
                        TintQuery += "red7f, grn7f, blu7f, ";
                        TintQuery += "red8x, grn8x, blu8x, ";
                        TintQuery += "red9x, grn9x, blu9x) SELECT id, '" + n.Name + "'";

                        for (int sc = 0; sc < 9; ++sc)
                        {
                            TintQuery += String.Format(", {0}, {1}, {2}", (n.SlotColour[sc] >> 16) & 0xff,
                                                                           (n.SlotColour[sc] >> 8) & 0xff,
                                                                           (n.SlotColour[sc] & 0xff));
                        }

                        TintQuery += " from npc_types where name = '" + n.Name + "' and id >= @StartingNPCTypeID and id <= @StartingNPCTypeID + 999 and version = " + SpawnVersion + " LIMIT 1;";
                        SQLOut(TintQuery);
                        SQLOut(String.Format("UPDATE npc_types set armortint_id = (SELECT id from npc_types_tint WHERE tint_set_name = '{0}' and id >= @StartingNPCTypeID and id <= @StartingNPCTypeID + 999 and version = {1} LIMIT 1), armortint_red = 0, armortint_green = 0, armortint_blue = 0 WHERE name = '{0}' and id >= @StartingNPCTypeID and id <= @StartingNPCTypeID + 999 and version = {1} LIMIT 1;",
                            n.Name, SpawnVersion));
                    }
                }
                return;
            }

            if (GenerateGrids)
            {
                List<PositionUpdate> AllMovementUpdates = PatchDecoder.GetAllMovementUpdates();

                foreach (PositionUpdate Update in AllMovementUpdates)
                    NPCSL.AddWaypoint(Update.SpawnID, Update.p, Update.HighRes);

                SQLOut("--");
                SQLOut("-- Grids");
                SQLOut("--");

                SQLOut("DELETE from grid WHERE id >= @StartingGridID AND id <= @StartingGridID + 999;");
                SQLOut("DELETE from grid_entries WHERE gridid >= @StartingGridID AND gridid <= @StartingGridID + 999;");
                foreach (NPCSpawn ns in NPCSL._NPCSpawnList)
                {
                    if (ns.Waypoints.Count > 1)
                    {
                        bool AllWaypointsTheSame = true;

                        for (int WPNumber = 0; WPNumber < ns.Waypoints.Count; ++WPNumber)
                        {
                            if (WPNumber == 0)
                                continue;
                            if ((ns.Waypoints[WPNumber].x != ns.Waypoints[WPNumber - 1].x) ||
                               (ns.Waypoints[WPNumber].y != ns.Waypoints[WPNumber - 1].y) ||
                               (ns.Waypoints[WPNumber].z != ns.Waypoints[WPNumber - 1].z))
                            {
                                AllWaypointsTheSame = false;
                            }
                        }

                        if (AllWaypointsTheSame)
                            continue;

                        int WaypointsInserted = 0;

                        int WPNum = 1;

                        int Pause = 10;

                        int FirstUsableWaypoint = 0;

                        if (ns.DoesHaveHighResWaypoints())
                            FirstUsableWaypoint = 1;

                        for (int WPNumber = FirstUsableWaypoint; WPNumber < ns.Waypoints.Count; ++WPNumber)
                        {
                            Position p = ns.Waypoints[WPNumber];

                            if (CoalesceWaypoints)
                            {
                                if ((WPNumber > FirstUsableWaypoint) && (WPNumber < (ns.Waypoints.Count - 2)))
                                {
                                    Position np = ns.Waypoints[WPNumber + 1];

                                    if ((Math.Abs(p.heading - np.heading) < 1) || (Math.Abs(p.heading - np.heading) > 255))
                                    {
                                        // Skipping waypoint as heading is the same as next.
                                        continue;
                                    }
                                    if ((Math.Abs(p.heading - np.heading) < 5) || (Math.Abs(p.heading - np.heading) > 251))
                                    {
                                        // Setting pause to 0 because headings are similar
                                        Pause = 0;
                                    }
                                    else if ((p.x == np.x) && (p.y == np.y) && (p.z == np.z))
                                    {
                                        // Skipping waypoint as same as next");
                                        continue;
                                    }
                                    else
                                        Pause = 10;
                                }
                            }

                            // If this is the last waypoint, and we haven't inserted any of the previous ones, then don't bother
                            // with this one either.
                            if ((WPNumber == (ns.Waypoints.Count - 1)) && (WaypointsInserted == 0))
                                continue;

                            SQLOut("INSERT into grid_entries (`gridid`, `zoneid`, `number`, `x`, `y`, `z`, `heading`, `pause`) VALUES(@StartingGridID + " + GridDBID + ", " + ZoneID + ", " + (WPNum++) + ", " + p.x + ", " + p.y + ", " + p.z + ", " + p.heading + ", " + Pause + ");");

                            ++WaypointsInserted;
                        }
                        if (WaypointsInserted > 1)
                        {
                            SQLOut("INSERT into grid(`id`, `zoneid`, `type`, `type2`) VALUES(@StartingGridID + " + GridDBID + ", " + ZoneID + ", 3, 2); -- " + ns.Name);

                            SQLOut("UPDATE spawn2 set pathgrid = @StartingGridID + " + GridDBID + " WHERE id = @StartingSpawn2ID + " + ns.Spawn2DBID + ";");

                            if (ns.DoesHaveHighResWaypoints())
                                SQLOut("UPDATE spawn2 set x = " + ns.Waypoints[1].x + ", y = " + ns.Waypoints[1].y + ", z = " + ns.Waypoints[1].z + ", heading = " + ns.Waypoints[1].heading + " WHERE id = @StartingSpawn2ID + " + ns.Spawn2DBID + ";");

                            ++GridDBID;
                        }
                    }
                }
            }
            if(GenerateMerchants)
                GenerateMerchantSQL(NPCSL, MerchantDBID, GenerateSpawns, SQLOut);
        }

        public void GenerateMerchantSQL(NPCSpawnList NPCSL, UInt32 MerchantDBID, bool GenerateSpawns, SQLDestination SQLOut)
        {
            MerchantManager mm = PatchDecoder.GetMerchantData(NPCSL);

            if(GenerateSpawns)
                SQLOut("DELETE from merchantlist where merchantid >= @StartingMerchantID and merchantid <= @StartingMerchantID + 999;");

            SQLOut("--");
            SQLOut("-- Merchant Lists");
            SQLOut("-- ");

            foreach (Merchant m in mm.MerchantList)
            {
                UInt32 MerchantSpawnID = m.SpawnID;

                NPCSpawn npc = NPCSL.GetNPC(MerchantSpawnID);

                if(npc == null)
                    continue;

                UInt32 MerchantNPCTypeID = npc.NPCTypeID;

                SQLOut("--");
                SQLOut("-- " + npc.Name);
                SQLOut("-- ");

                bool StartOfPlayerSoldItems = false;

                foreach (MerchantItem mi in m.Items)
                {
                    string Insert = "";

                    if (mi.Quantity >= 0)
                    {
                        if (!StartOfPlayerSoldItems)
                        {
                            StartOfPlayerSoldItems = true;
                            SQLOut("--");
                            SQLOut("-- The items below were more than likely sold to " + npc.Name + " by players. Uncomment them if you want.");
                            SQLOut("--");
                        }

                        Insert += "-- ";
                    }

                    Insert += "INSERT into merchantlist(`merchantid`, `slot`, `item`) VALUES(";

                    Insert += "@StartingMerchantID + " + MerchantDBID + ", " + mi.Slot + ", " + mi.ItemID + "); -- " + mi.Name;

                    SQLOut(Insert);
                }

                if(GenerateSpawns)
                    SQLOut("UPDATE npc_types SET merchant_id = @StartingMerchantID + " + MerchantDBID + " WHERE id = @StartingNPCTypeID + " + MerchantNPCTypeID + ";");

                ++MerchantDBID;
            }
        }

        public void GenerateZonePointSQL(string ZoneName, SQLDestination SQLOut)
        {
            foreach (ZonePoint zp in ZonePointList)
            {
                string Insert = String.Format("REPLACE into zone_points(`zone`, `number`, `y`, `x`, `z`, `heading`, `target_y`, `target_x`, `target_z`, `target_heading`, `zoneinst`, `target_zone_id`, `buffer`) VALUES('{0}', {1}, 0, 0, 0, 0, {2}, {3}, {4}, {5}, {6}, {7}, 0);", ZoneName, zp.Number, zp.y, zp.x, zp.z, zp.Heading, zp.Instance, zp.ZoneID);

                SQLOut(Insert);
            }
        }

        public void GenerateZonePointList()
        {
            ZonePointList = PatchDecoder.GetZonePointList();
        }

        public ZonePoint? GetZonePointNumber(Int32 Number)
        {
            if (ZonePointList != null)
            {
                foreach (ZonePoint zp in ZonePointList)
                {
                    if (zp.Number == Number)
                        return zp;
                }
            }
            return null;
        }

        public UInt32 GenerateZoneSQL(SQLDestination SQLOut)
        {
            UInt16 ZoneID = PatchDecoder.GetZoneNumber();

            NewZoneStruct NewZone = PatchDecoder.GetZoneData();

            SQLOut("--");
            SQLOut("-- Zone Config");
            SQLOut("--");
            string InsertFormat = "UPDATE zone set `short_name` = '{0}', `file_name` = '', `long_name` = '{1}', `safe_x` = {2}, `safe_y` = {3}, `safe_z` = {4}, ";
            InsertFormat += "`underworld` = {6}, `minclip` = {7}, `maxclip` = {8}, `fog_minclip` = {9}, `fog_maxclip` = {10}, ";
            InsertFormat += "`fog_blue` = {11}, `fog_red` = {12}, `fog_green` = {13}, `sky` = {14}, `ztype` = {15}, `time_type` = {16}, ";
            InsertFormat += "`fog_red2` = {17}, `fog_green2` = {18}, `fog_blue2` = {19}, `fog_minclip2` = {20}, `fog_maxclip2` = {21}, ";
            InsertFormat += "`fog_red3` = {22}, `fog_green3` = {23}, `fog_blue3` = {24}, `fog_minclip3` = {25}, `fog_maxclip3` = {26}, ";
            InsertFormat += "`fog_red4` = {27}, `fog_green4` = {28}, `fog_blue4` = {29}, `fog_minclip4` = {30}, `fog_maxclip4` = {31} WHERE zoneidnumber = {5};";

            SQLOut(String.Format(InsertFormat, NewZone.ShortName2, NewZone.LongName, NewZone.SafeX, NewZone.SafeY, NewZone.SafeZ,
                                               ZoneID, NewZone.UnderWorld, NewZone.MinClip, NewZone.MaxClip, NewZone.FogMinClip[0], NewZone.FogMaxClip[0],
                                               NewZone.FogBlue[0], NewZone.FogRed[0], NewZone.FogGreen[0], NewZone.Sky, NewZone.Type, NewZone.TimeType,
                                               NewZone.FogRed[1], NewZone.FogGreen[1], NewZone.FogBlue[1], NewZone.FogMinClip[1], NewZone.FogMaxClip[1],
                                               NewZone.FogRed[2], NewZone.FogGreen[2], NewZone.FogBlue[2], NewZone.FogMinClip[2], NewZone.FogMaxClip[2],
                                               NewZone.FogRed[3], NewZone.FogGreen[3], NewZone.FogBlue[3], NewZone.FogMinClip[3], NewZone.FogMaxClip[3]));

            SQLOut(String.Format("UPDATE zone set fog_density = {0} WHERE zoneidnumber = {1};", NewZone.FogDensity, ZoneID));
            SQLOut("--");

            return ZoneID;
        }

        public bool DumpAAs(string FileName)
        {
            return PatchDecoder.DumpAAs(FileName);
        }

        public void GenerateObjectSQL(bool DoGroundSpawns, bool DoObjects, UInt32 SpawnVersion, SQLDestination SQLOut)
        {
            List<GroundSpawnStruct> GroundSpawns = PatchDecoder.GetGroundSpawns();

            UInt32 GroundSpawnDBID = 0;
            UInt32 ObjectDBID = 0;

            SQLOut("--");
            SQLOut("-- Objects and Groundspawns");
            SQLOut("--");

            if(DoGroundSpawns)
                SQLOut("DELETE from ground_spawns where id >= @StartingGroundSpawnID and id <= @StartingGroundSpawnID + 999 and version = " + SpawnVersion + ";");

            if(DoObjects)
                SQLOut("DELETE from object where id >= @StartingObjectID and id <= @StartingObjectID + 999 and version = " + SpawnVersion + ";");

            foreach(GroundSpawnStruct GroundSpawn in GroundSpawns)
            {
                String Insert;

                if (IsGroundSpawn(GroundSpawn.ObjectType) && DoGroundSpawns)
                {
                    Insert = "INSERT into ground_spawns(`id`, `zoneid`, `version`, `max_x`, `max_y`, `max_z`, `min_x`, `min_y`, `heading`, `name`, `item`, `max_allowed`, `comment`, `respawn_timer`) VALUES(";

                    Insert += "@StartingGroundSpawnID + " + (GroundSpawnDBID++) + ", " + GroundSpawn.ZoneID + ", " + SpawnVersion + ", " + GroundSpawn.x + ", " + GroundSpawn.y + ", " + GroundSpawn.z + ", " + GroundSpawn.x + ", " + GroundSpawn.y + ", " + GroundSpawn.Heading + ", '" + GroundSpawn.Name + "', 1001, 1, 'Auto generated by Collector. FIX THE ITEMID!', 300000);";

                    SQLOut(Insert);
                }
                else if(!IsGroundSpawn(GroundSpawn.ObjectType) && DoObjects)
                {
                    GroundSpawn.ObjectType = ObjectNameToType(GroundSpawn.Name);

                    Insert = "INSERT into object(`id`, `zoneid`, `version`, `xpos`, `ypos`, `zpos`, `heading`, `itemid`, `charges`, `objectname`, `type`, `icon`) VALUES(";

                    Insert += "@StartingObjectID + " + (ObjectDBID++) + ", " + GroundSpawn.ZoneID + ", " + SpawnVersion + ", " + GroundSpawn.x + ", " + GroundSpawn.y + ", " + GroundSpawn.z + ", " + GroundSpawn.Heading + ", 0, 0, '" + GroundSpawn.Name + "', " + GroundSpawn.ObjectType + ", 0);";

                    SQLOut(Insert);
                }
            }
        }

        static bool IsGroundSpawn(UInt32 Type)
        {
            if ((Type == 1) || (Type == 8))
                return true;

            return false;
        }

        public static UInt32 ObjectNameToType(string Name)
        {
            switch (Name)
            {
                case "IT10511_ACTORDEF":
                    return 3;
                case "IT10512_ACTODEF":
                    return 1;
                case "IT10714_ACTORDEF":
                    return 53;
                case "IT10800_ACTORDEF":
                    return 21;
                case "IT10801_ACTORDEF":
                    return 22;
                case "IT10802_ACTORDEF":
                    return 16;
                case "IT10803_ACTORDEF":
                    return 15;
                case "IT10865_ACTORDEF":
                    return 15;
                case "IT128_ACTORDEF":
                    return 16;
                case "IT27_ACTORDEF":
                    return 0;
                case "IT403_ACTORDEF":
                    return 1;
                case "IT5_ACTORDEF":
                    return 25;
                case "IT63_ACTORDEF":
                    return 8;
                case "IT64_ACTORDEF":
                    return 30;
                case "IT69_ACTORDEF":
                    return 15;
                case "IT73_ACTORDEF":
                    return 22;
                case "IT74_ACTORDEF":
                    return 21;
                case "IT10805_ACTORDEF":
                case "IT70_ACTORDEF":
                    return 19;
                case "IT66_ACTORDEF":
                case "IT10804_ACTORDEF":
                case "IT10863_ACTORDEF":
                case "IT10864_ACTORDEF":
                    return 17;

                default:
                    return 255;
            }
        }

        public static UInt32 ZoneNameToNumber(string ZoneName)
        {
            switch (ZoneName)
            {
                case "qeynos": return 1;
                case "qeynos2": return 2;
                case "qrg": return 3;
                case "qeytoqrg": return 4;
                case "highpass": return 5;
                case "highkeep": return 6;
                case "freportn": return 8;
                case "freportw": return 9;
                case "freporte": return 10;
                case "runnyeye": return 11;
                case "qey2hh1": return 12;
                case "northkarana": return 13;
                case "southkarana": return 14;
                case "eastkarana": return 15;
                case "beholder": return 16;
                case "blackburrow": return 17;
                case "paw": return 18;
                case "rivervale": return 19;
                case "kithicor": return 20;
                case "commons": return 21;
                case "ecommons": return 22;
                case "erudnint": return 23;
                case "erudnext": return 24;
                case "nektulos": return 25;
                case "cshome": return 26;
                case "lavastorm": return 27;
                case "nektropos": return 28;
                case "halas": return 29;
                case "everfrost": return 30;
                case "soldunga": return 31;
                case "soldungb": return 32;
                case "misty": return 33;
                case "nro": return 34;
                case "sro": return 35;
                case "befallen": return 36;
                case "oasis": return 37;
                case "tox": return 38;
                case "hole": return 39;
                case "neriaka": return 40;
                case "neriakb": return 41;
                case "neriakc": return 42;
                case "neriakd": return 43;
                case "najena": return 44;
                case "qcat": return 45;
                case "innothule": return 46;
                case "feerrott": return 47;
                case "cazicthule": return 48;
                case "oggok": return 49;
                case "rathemtn": return 50;
                case "lakerathe": return 51;
                case "grobb": return 52;
                case "aviak": return 53;
                case "gfaydark": return 54;
                case "akanon": return 55;
                case "steamfont": return 56;
                case "lfaydark": return 57;
                case "crushbone": return 58;
                case "mistmoore": return 59;
                case "kaladima": return 60;
                case "felwithea": return 61;
                case "felwitheb": return 62;
                case "unrest": return 63;
                case "kedge": return 64;
                case "guktop": return 65;
                case "gukbottom": return 66;
                case "kaladimb": return 67;
                case "butcher": return 68;
                case "oot": return 69;
                case "cauldron": return 70;
                case "airplane": return 71;
                case "fearplane": return 72;
                case "permafrost": return 73;
                case "kerraridge": return 74;
                case "paineel": return 75;
                case "hateplane": return 76;
                case "arena": return 77;
                case "fieldofbone": return 78;
                case "warslikswood": return 79;
                case "soltemple": return 80;
                case "droga": return 81;
                case "cabwest": return 82;
                case "swampofnohope": return 83;
                case "firiona": return 84;
                case "lakeofillomen": return 85;
                case "dreadlands": return 86;
                case "burningwood": return 87;
                case "kaesora": return 88;
                case "sebilis": return 89;
                case "citymist": return 90;
                case "skyfire": return 91;
                case "frontiermtns": return 92;
                case "overthere": return 93;
                case "emeraldjungle": return 94;
                case "trakanon": return 95;
                case "timorous": return 96;
                case "kurn": return 97;
                case "erudsxing": return 98;
                case "stonebrunt": return 100;
                case "warrens": return 101;
                case "karnor": return 102;
                case "chardok": return 103;
                case "dalnir": return 104;
                case "charasis": return 105;
                case "cabeast": return 106;
                case "nurga": return 107;
                case "veeshan": return 108;
                case "veksar": return 109;
                case "iceclad": return 110;
                case "frozenshadow": return 111;
                case "velketor": return 112;
                case "kael": return 113;
                case "skyshrine": return 114;
                case "thurgadina": return 115;
                case "eastwastes": return 116;
                case "cobaltscar": return 117;
                case "greatdivide": return 118;
                case "wakening": return 119;
                case "westwastes": return 120;
                case "crystal": return 121;
                case "necropolis": return 123;
                case "templeveeshan": return 124;
                case "sirens": return 125;
                case "mischiefplane": return 126;
                case "growthplane": return 127;
                case "sleeper": return 128;
                case "thurgadinb": return 129;
                case "erudsxing2": return 130;
                case "shadowhaven": return 150;
                case "bazaar": return 151;
                case "nexus": return 152;
                case "echo": return 153;
                case "acrylia": return 154;
                case "sharvahl": return 155;
                case "paludal": return 156;
                case "fungusgrove": return 157;
                case "vexthal": return 158;
                case "sseru": return 159;
                case "katta": return 160;
                case "netherbian": return 161;
                case "ssratemple": return 162;
                case "griegsend": return 163;
                case "thedeep": return 164;
                case "shadeweaver": return 165;
                case "hollowshade": return 166;
                case "grimling": return 167;
                case "mseru": return 168;
                case "letalis": return 169;
                case "twilight": return 170;
                case "thegrey": return 171;
                case "tenebrous": return 172;
                case "maiden": return 173;
                case "dawnshroud": return 174;
                case "scarlet": return 175;
                case "umbral": return 176;
                case "akheva": return 179;
                case "arena2": return 180;
                case "jaggedpine": return 181;
                case "nedaria": return 182;
                case "tutorial": return 183;
                case "load": return 184;
                case "load2": return 185;
                case "hateplaneb": return 186;
                case "shadowrest": return 187;
                case "tutoriala": return 188;
                case "tutorialb": return 189;
                case "clz": return 190;
                case "codecay": return 200;
                case "pojustice": return 201;
                case "poknowledge": return 202;
                case "potranquility": return 203;
                case "ponightmare": return 204;
                case "podisease": return 205;
                case "poinnovation": return 206;
                case "potorment": return 207;
                case "povalor": return 208;
                case "bothunder": return 209;
                case "postorms": return 210;
                case "hohonora": return 211;
                case "solrotower": return 212;
                case "powar": return 213;
                case "potactics": return 214;
                case "poair": return 215;
                case "powater": return 216;
                case "pofire": return 217;
                case "poeartha": return 218;
                case "potimea": return 219;
                case "hohonorb": return 220;
                case "nightmareb": return 221;
                case "poearthb": return 222;
                case "potimeb": return 223;
                case "gunthak": return 224;
                case "dulak": return 225;
                case "torgiran": return 226;
                case "nadox": return 227;
                case "hatesfury": return 228;
                case "guka": return 229;
                case "ruja": return 230;
                case "taka": return 231;
                case "mira": return 232;
                case "mmca": return 233;
                case "gukb": return 234;
                case "rujb": return 235;
                case "takb": return 236;
                case "mirb": return 237;
                case "mmcb": return 238;
                case "gukc": return 239;
                case "rujc": return 240;
                case "takc": return 241;
                case "mirc": return 242;
                case "mmcc": return 243;
                case "gukd": return 244;
                case "rujd": return 245;
                case "takd": return 246;
                case "mird": return 247;
                case "mmcd": return 248;
                case "guke": return 249;
                case "ruje": return 250;
                case "take": return 251;
                case "mire": return 252;
                case "mmce": return 253;
                case "gukf": return 254;
                case "rujf": return 255;
                case "takf": return 256;
                case "mirf": return 257;
                case "mmcf": return 258;
                case "gukg": return 259;
                case "rujg": return 260;
                case "takg": return 261;
                case "mirg": return 262;
                case "mmcg": return 263;
                case "gukh": return 264;
                case "rujh": return 265;
                case "takh": return 266;
                case "mirh": return 267;
                case "mmch": return 268;
                case "ruji": return 269;
                case "taki": return 270;
                case "miri": return 271;
                case "mmci": return 272;
                case "rujj": return 273;
                case "takj": return 274;
                case "mirj": return 275;
                case "mmcj": return 276;
                case "chardokb": return 277;
                case "soldungc": return 278;
                case "abysmal": return 279;
                case "natimbi": return 280;
                case "qinimi": return 281;
                case "riwwi": return 282;
                case "barindu": return 283;
                case "ferubi": return 284;
                case "snpool": return 285;
                case "snlair": return 286;
                case "snplant": return 287;
                case "sncrematory": return 288;
                case "tipt": return 289;
                case "vxed": return 290;
                case "yxtta": return 291;
                case "uqua": return 292;
                case "kodtaz": return 293;
                case "ikkinz": return 294;
                case "qvic": return 295;
                case "inktuta": return 296;
                case "txevu": return 297;
                case "tacvi": return 298;
                case "qvicb": return 299;
                case "wallofslaughter": return 300;
                case "bloodfields": return 301;
                case "draniksscar": return 302;
                case "causeway": return 303;
                case "chambersa": return 304;
                case "chambersb": return 305;
                case "chambersc": return 306;
                case "chambersd": return 307;
                case "chamberse": return 308;
                case "chambersf": return 309;
                case "provinggrounds": return 316;
                case "anguish": return 317;
                case "dranikhollowsa": return 318;
                case "dranikhollowsb": return 319;
                case "dranikhollowsc": return 320;
                case "dranikhollowsd": return 321;
                case "dranikhollowse": return 322;
                case "dranikhollowsf": return 323;
                case "dranikhollowsg": return 324;
                case "dranikhollowsh": return 325;
                case "dranikhollowsi": return 326;
                case "dranikhollowsj": return 327;
                case "dranikcatacombsa": return 328;
                case "dranikcatacombsb": return 329;
                case "dranikcatacombsc": return 330;
                case "draniksewersa": return 331;
                case "draniksewersb": return 332;
                case "draniksewersc": return 333;
                case "riftseekers": return 334;
                case "harbingers": return 335;
                case "dranik": return 336;
                case "broodlands": return 337;
                case "stillmoona": return 338;
                case "stillmoonb": return 339;
                case "thundercrest": return 340;
                case "delvea": return 341;
                case "delveb": return 342;
                case "thenest": return 343;
                case "guildlobby": return 344;
                case "guildhall": return 345;
                case "barter": return 346;
                case "illsalin": return 347;
                case "illsalina": return 348;
                case "illsalinb": return 349;
                case "illsalinc": return 350;
                case "dreadspire": return 351;
                case "drachnidhive": return 354;
                case "drachnidhivea": return 355;
                case "drachnidhiveb": return 356;
                case "drachnidhivec": return 357;
                case "westkorlach": return 358;
                case "westkorlacha": return 359;
                case "westkorlachb": return 360;
                case "westkorlachc": return 361;
                case "eastkorlach": return 362;
                case "eastkorlacha": return 363;
                case "shadowspine": return 364;
                case "corathus": return 365;
                case "corathusa": return 366;
                case "corathusb": return 367;
                case "nektulosa": return 368;
                case "arcstone": return 369;
                case "relic": return 370;
                case "skylance": return 371;
                case "devastation": return 372;
                case "devastationa": return 373;
                case "rage": return 374;
                case "ragea": return 375;
                case "takishruins": return 376;
                case "takishruinsa": return 377;
                case "elddar": return 378;
                case "elddara": return 379;
                case "theater": return 380;
                case "theatera": return 381;
                case "freeporteast": return 382;
                case "freeportwest": return 383;
                case "freeportsewers": return 384;
                case "freeportacademy": return 385;
                case "freeporttemple": return 386;
                case "freeportmilitia": return 387;
                case "freeportarena": return 388;
                case "freeportcityhall": return 389;
                case "freeporttheater": return 390;
                case "freeporthall": return 391;
                case "northro": return 392;
                case "southro": return 393;
                case "crescent": return 394;
                case "moors": return 395;
                case "stonehive": return 396;
                case "mesa": return 397;
                case "roost": return 398;
                case "steppes": return 399;
                case "icefall": return 400;
                case "valdeholm": return 401;
                case "frostcrypt": return 402;
                case "sunderock": return 403;
                case "vergalid": return 404;
                case "direwind": return 405;
                case "ashengate": return 406;
                case "highpasshold": return 407;
                case "commonlands": return 408;
                case "oceanoftears": return 409;
                case "kithforest": return 410;
                case "befallenb": return 411;
                case "highpasskeep": return 412;
                case "innothuleb": return 413;
                case "toxxulia": return 414;
                case "mistythicket": return 415;
                case "kattacastrum": return 416;
                case "thalassius": return 417;
                case "atiiki": return 418;
                case "zhisza": return 419;
                case "silyssar": return 420;
                case "solteris": return 421;
                case "barren": return 422;
                case "buriedsea": return 423;
                case "jardelshook": return 424;
                case "monkeyrock": return 425;
                case "suncrest": return 426;
                case "deadbone": return 427;
                case "blacksail": return 428;
                case "maidensgrave": return 429;
                case "redfeather": return 430;
                case "shipmvp": return 431;
                case "shipmvu": return 432;
                case "shippvu": return 433;
                case "shipuvu": return 434;
                case "shipmvm": return 435;
                case "mechanotus": return 436;
                case "mansion": return 437;
                case "steamfactory": return 438;
                case "shipworkshop": return 439;
                case "gyrospireb": return 440;
                case "gyrospirez": return 441;
                case "dragonscale": return 442;
                case "lopingplains": return 443;
                case "hillsofshade": return 444;
                case "bloodmoon": return 445;
                case "crystallos": return 446;
                case "guardian": return 447;
                case "steamfontmts": return 448;
                case "cryptofshade": return 449;
                case "dragonscaleb": return 451;
                case "oldfieldofbone": return 452;
                case "oldkaesoraa": return 453;
                case "oldkaesorab": return 454;
                case "oldkurn": return 455;
                case "oldkithicor": return 456;
                case "oldcommons": return 457;
                case "oldhighpass": return 458;
                case "thevoida": return 459;
                case "thevoidb": return 460;
                case "thevoidc": return 461;
                case "thevoidd": return 462;
                case "thevoide": return 463;
                case "thevoidf": return 464;
                case "thevoidg": return 465;
                case "oceangreenhills": return 466;
                case "oceangreenvillag": return 467;
                case "oldblackburrow": return 468;
                case "bertoxtemple": return 469;
                case "discord": return 470;
                case "discordtower": return 471;
                case "oldbloodfield": return 472;
                case "precipiceofwar": return 473;
                case "olddranik": return 474;
                case "toskirakk": return 475;
                case "korascian": return 476;
                case "rathechamber": return 477;
                case "arttest": return 996;
                case "fhalls": return 998;
                case "apprentice": return 999;

            }

            return 0;
        }

        public static string ZoneNumberToName(UInt32 ZoneNumber)
        {
            switch (ZoneNumber)
            {
                case 1: return "qeynos";
                case 2: return "qeynos2";
                case 3: return "qrg";
                case 4: return "qeytoqrg";
                case 5: return "highpass";
                case 6: return "highkeep";
                case 8: return "freportn";
                case 9: return "freportw";
                case 10: return "freporte";
                case 11: return "runnyeye";
                case 12: return "qey2hh1";
                case 13: return "northkarana";
                case 14: return "southkarana";
                case 15: return "eastkarana";
                case 16: return "beholder";
                case 17: return "blackburrow";
                case 18: return "paw";
                case 19: return "rivervale";
                case 20: return "kithicor";
                case 21: return "commons";
                case 22: return "ecommons";
                case 23: return "erudnint";
                case 24: return "erudnext";
                case 25: return "nektulos";
                case 26: return "cshome";
                case 27: return "lavastorm";
                case 28: return "nektropos";
                case 29: return "halas";
                case 30: return "everfrost";
                case 31: return "soldunga";
                case 32: return "soldungb";
                case 33: return "misty";
                case 34: return "nro";
                case 35: return "sro";
                case 36: return "befallen";
                case 37: return "oasis";
                case 38: return "tox";
                case 39: return "hole";
                case 40: return "neriaka";
                case 41: return "neriakb";
                case 42: return "neriakc";
                case 43: return "neriakd";
                case 44: return "najena";
                case 45: return "qcat";
                case 46: return "innothule";
                case 47: return "feerrott";
                case 48: return "cazicthule";
                case 49: return "oggok";
                case 50: return "rathemtn";
                case 51: return "lakerathe";
                case 52: return "grobb";
                case 53: return "aviak";
                case 54: return "gfaydark";
                case 55: return "akanon";
                case 56: return "steamfont";
                case 57: return "lfaydark";
                case 58: return "crushbone";
                case 59: return "mistmoore";
                case 60: return "kaladima";
                case 61: return "felwithea";
                case 62: return "felwitheb";
                case 63: return "unrest";
                case 64: return "kedge";
                case 65: return "guktop";
                case 66: return "gukbottom";
                case 67: return "kaladimb";
                case 68: return "butcher";
                case 69: return "oot";
                case 70: return "cauldron";
                case 71: return "airplane";
                case 72: return "fearplane";
                case 73: return "permafrost";
                case 74: return "kerraridge";
                case 75: return "paineel";
                case 76: return "hateplane";
                case 77: return "arena";
                case 78: return "fieldofbone";
                case 79: return "warslikswood";
                case 80: return "soltemple";
                case 81: return "droga";
                case 82: return "cabwest";
                case 83: return "swampofnohope";
                case 84: return "firiona";
                case 85: return "lakeofillomen";
                case 86: return "dreadlands";
                case 87: return "burningwood";
                case 88: return "kaesora";
                case 89: return "sebilis";
                case 90: return "citymist";
                case 91: return "skyfire";
                case 92: return "frontiermtns";
                case 93: return "overthere";
                case 94: return "emeraldjungle";
                case 95: return "trakanon";
                case 96: return "timorous";
                case 97: return "kurn";
                case 98: return "erudsxing";
                case 100: return "stonebrunt";
                case 101: return "warrens";
                case 102: return "karnor";
                case 103: return "chardok";
                case 104: return "dalnir";
                case 105: return "charasis";
                case 106: return "cabeast";
                case 107: return "nurga";
                case 108: return "veeshan";
                case 109: return "veksar";
                case 110: return "iceclad";
                case 111: return "frozenshadow";
                case 112: return "velketor";
                case 113: return "kael";
                case 114: return "skyshrine";
                case 115: return "thurgadina";
                case 116: return "eastwastes";
                case 117: return "cobaltscar";
                case 118: return "greatdivide";
                case 119: return "wakening";
                case 120: return "westwastes";
                case 121: return "crystal";
                case 123: return "necropolis";
                case 124: return "templeveeshan";
                case 125: return "sirens";
                case 126: return "mischiefplane";
                case 127: return "growthplane";
                case 128: return "sleeper";
                case 129: return "thurgadinb";
                case 130: return "erudsxing2";
                case 150: return "shadowhaven";
                case 151: return "bazaar";
                case 152: return "nexus";
                case 153: return "echo";
                case 154: return "acrylia";
                case 155: return "sharvahl";
                case 156: return "paludal";
                case 157: return "fungusgrove";
                case 158: return "vexthal";
                case 159: return "sseru";
                case 160: return "katta";
                case 161: return "netherbian";
                case 162: return "ssratemple";
                case 163: return "griegsend";
                case 164: return "thedeep";
                case 165: return "shadeweaver";
                case 166: return "hollowshade";
                case 167: return "grimling";
                case 168: return "mseru";
                case 169: return "letalis";
                case 170: return "twilight";
                case 171: return "thegrey";
                case 172: return "tenebrous";
                case 173: return "maiden";
                case 174: return "dawnshroud";
                case 175: return "scarlet";
                case 176: return "umbral";
                case 179: return "akheva";
                case 180: return "arena2";
                case 181: return "jaggedpine";
                case 182: return "nedaria";
                case 183: return "tutorial";
                case 184: return "load";
                case 185: return "load2";
                case 186: return "hateplaneb";
                case 187: return "shadowrest";
                case 188: return "tutoriala";
                case 189: return "tutorialb";
                case 190: return "clz";
                case 200: return "codecay";
                case 201: return "pojustice";
                case 202: return "poknowledge";
                case 203: return "potranquility";
                case 204: return "ponightmare";
                case 205: return "podisease";
                case 206: return "poinnovation";
                case 207: return "potorment";
                case 208: return "povalor";
                case 209: return "bothunder";
                case 210: return "postorms";
                case 211: return "hohonora";
                case 212: return "solrotower";
                case 213: return "powar";
                case 214: return "potactics";
                case 215: return "poair";
                case 216: return "powater";
                case 217: return "pofire";
                case 218: return "poeartha";
                case 219: return "potimea";
                case 220: return "hohonorb";
                case 221: return "nightmareb";
                case 222: return "poearthb";
                case 223: return "potimeb";
                case 224: return "gunthak";
                case 225: return "dulak";
                case 226: return "torgiran";
                case 227: return "nadox";
                case 228: return "hatesfury";
                case 229: return "guka";
                case 230: return "ruja";
                case 231: return "taka";
                case 232: return "mira";
                case 233: return "mmca";
                case 234: return "gukb";
                case 235: return "rujb";
                case 236: return "takb";
                case 237: return "mirb";
                case 238: return "mmcb";
                case 239: return "gukc";
                case 240: return "rujc";
                case 241: return "takc";
                case 242: return "mirc";
                case 243: return "mmcc";
                case 244: return "gukd";
                case 245: return "rujd";
                case 246: return "takd";
                case 247: return "mird";
                case 248: return "mmcd";
                case 249: return "guke";
                case 250: return "ruje";
                case 251: return "take";
                case 252: return "mire";
                case 253: return "mmce";
                case 254: return "gukf";
                case 255: return "rujf";
                case 256: return "takf";
                case 257: return "mirf";
                case 258: return "mmcf";
                case 259: return "gukg";
                case 260: return "rujg";
                case 261: return "takg";
                case 262: return "mirg";
                case 263: return "mmcg";
                case 264: return "gukh";
                case 265: return "rujh";
                case 266: return "takh";
                case 267: return "mirh";
                case 268: return "mmch";
                case 269: return "ruji";
                case 270: return "taki";
                case 271: return "miri";
                case 272: return "mmci";
                case 273: return "rujj";
                case 274: return "takj";
                case 275: return "mirj";
                case 276: return "mmcj";
                case 277: return "chardokb";
                case 278: return "soldungc";
                case 279: return "abysmal";
                case 280: return "natimbi";
                case 281: return "qinimi";
                case 282: return "riwwi";
                case 283: return "barindu";
                case 284: return "ferubi";
                case 285: return "snpool";
                case 286: return "snlair";
                case 287: return "snplant";
                case 288: return "sncrematory";
                case 289: return "tipt";
                case 290: return "vxed";
                case 291: return "yxtta";
                case 292: return "uqua";
                case 293: return "kodtaz";
                case 294: return "ikkinz";
                case 295: return "qvic";
                case 296: return "inktuta";
                case 297: return "txevu";
                case 298: return "tacvi";
                case 299: return "qvicb";
                case 300: return "wallofslaughter";
                case 301: return "bloodfields";
                case 302: return "draniksscar";
                case 303: return "causeway";
                case 304: return "chambersa";
                case 305: return "chambersb";
                case 306: return "chambersc";
                case 307: return "chambersd";
                case 308: return "chamberse";
                case 309: return "chambersf";
                case 316: return "provinggrounds";
                case 317: return "anguish";
                case 318: return "dranikhollowsa";
                case 319: return "dranikhollowsb";
                case 320: return "dranikhollowsc";
                case 321: return "dranikhollowsd";
                case 322: return "dranikhollowse";
                case 323: return "dranikhollowsf";
                case 324: return "dranikhollowsg";
                case 325: return "dranikhollowsh";
                case 326: return "dranikhollowsi";
                case 327: return "dranikhollowsj";
                case 328: return "dranikcatacombsa";
                case 329: return "dranikcatacombsb";
                case 330: return "dranikcatacombsc";
                case 331: return "draniksewersa";
                case 332: return "draniksewersb";
                case 333: return "draniksewersc";
                case 334: return "riftseekers";
                case 335: return "harbingers";
                case 336: return "dranik";
                case 337: return "broodlands";
                case 338: return "stillmoona";
                case 339: return "stillmoonb";
                case 340: return "thundercrest";
                case 341: return "delvea";
                case 342: return "delveb";
                case 343: return "thenest";
                case 344: return "guildlobby";
                case 345: return "guildhall";
                case 346: return "barter";
                case 347: return "illsalin";
                case 348: return "illsalina";
                case 349: return "illsalinb";
                case 350: return "illsalinc";
                case 351: return "dreadspire";
                case 354: return "drachnidhive";
                case 355: return "drachnidhivea";
                case 356: return "drachnidhiveb";
                case 357: return "drachnidhivec";
                case 358: return "westkorlach";
                case 359: return "westkorlacha";
                case 360: return "westkorlachb";
                case 361: return "westkorlachc";
                case 362: return "eastkorlach";
                case 363: return "eastkorlacha";
                case 364: return "shadowspine";
                case 365: return "corathus";
                case 366: return "corathusa";
                case 367: return "corathusb";
                case 368: return "nektulosa";
                case 369: return "arcstone";
                case 370: return "relic";
                case 371: return "skylance";
                case 372: return "devastation";
                case 373: return "devastationa";
                case 374: return "rage";
                case 375: return "ragea";
                case 376: return "takishruins";
                case 377: return "takishruinsa";
                case 378: return "elddar";
                case 379: return "elddara";
                case 380: return "theater";
                case 381: return "theatera";
                case 382: return "freeporteast";
                case 383: return "freeportwest";
                case 384: return "freeportsewers";
                case 385: return "freeportacademy";
                case 386: return "freeporttemple";
                case 387: return "freeportmilitia";
                case 388: return "freeportarena";
                case 389: return "freeportcityhall";
                case 390: return "freeporttheater";
                case 391: return "freeporthall";
                case 392: return "northro";
                case 393: return "southro";
                case 394: return "crescent";
                case 395: return "moors";
                case 396: return "stonehive";
                case 397: return "mesa";
                case 398: return "roost";
                case 399: return "steppes";
                case 400: return "icefall";
                case 401: return "valdeholm";
                case 402: return "frostcrypt";
                case 403: return "sunderock";
                case 404: return "vergalid";
                case 405: return "direwind";
                case 406: return "ashengate";
                case 407: return "highpasshold";
                case 408: return "commonlands";
                case 409: return "oceanoftears";
                case 410: return "kithforest";
                case 411: return "befallenb";
                case 412: return "highpasskeep";
                case 413: return "innothuleb";
                case 414: return "toxxulia";
                case 415: return "mistythicket";
                case 416: return "kattacastrum";
                case 417: return "thalassius";
                case 418: return "atiiki";
                case 419: return "zhisza";
                case 420: return "silyssar";
                case 421: return "solteris";
                case 422: return "barren";
                case 423: return "buriedsea";
                case 424: return "jardelshook";
                case 425: return "monkeyrock";
                case 426: return "suncrest";
                case 427: return "deadbone";
                case 428: return "blacksail";
                case 429: return "maidensgrave";
                case 430: return "redfeather";
                case 431: return "shipmvp";
                case 432: return "shipmvu";
                case 433: return "shippvu";
                case 434: return "shipuvu";
                case 435: return "shipmvm";
                case 436: return "mechanotus";
                case 437: return "mansion";
                case 438: return "steamfactory";
                case 439: return "shipworkshop";
                case 440: return "gyrospireb";
                case 441: return "gyrospirez";
                case 442: return "dragonscale";
                case 443: return "lopingplains";
                case 444: return "hillsofshade";
                case 445: return "bloodmoon";
                case 446: return "crystallos";
                case 447: return "guardian";
                case 448: return "steamfontmts";
                case 449: return "cryptofshade";
                case 451: return "dragonscaleb";
                case 452: return "oldfieldofbone";
                case 453: return "oldkaesoraa";
                case 454: return "oldkaesorab";
                case 455: return "oldkurn";
                case 456: return "oldkithicor";
                case 457: return "oldcommons";
                case 458: return "oldhighpass";
                case 459: return "thevoida";
                case 460: return "thevoidb";
                case 461: return "thevoidc";
                case 462: return "thevoidd";
                case 463: return "thevoide";
                case 464: return "thevoidf";
                case 465: return "thevoidg";
                case 466: return "oceangreenhills";
                case 467: return "oceangreenvillag";
                case 468: return "oldblackburrow";
                case 469: return "bertoxtemple";
                case 470: return "discord";
                case 471: return "discordtower";
                case 472: return "oldbloodfield";
                case 473: return "precipiceofwar";
                case 474: return "olddranik";
                case 475: return "toskirakk";
                case 476: return "korascian";
                case 477: return "rathechamber";
                case 996: return "arttest";
                case 998: return "fhalls";
                case 999: return "apprentice";
            }
            return "UNKNOWNZONE";
        }

        public bool SupportsSQLGeneration()
        {
            if (PatchDecoder.SupportsSQLGeneration)
                return true;

            return false;
        }
    }
}
