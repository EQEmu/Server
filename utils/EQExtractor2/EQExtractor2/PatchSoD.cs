//
// Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net). Distributed under GPL version 2.
//
//
//
// IMPORTANT NOTE: This decoder for SoD was developed purely as a cursory test of the multi-patch support and is not guaranteed
// to be 100% correct.
//
//
using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchSoD : PatchSpecficDecoder
    {
        public PatchSoD()
        {
            Version = "Seeds of Destruction";

            PatchConfFileName = "patch_SoD.conf";

            ExpectedPPLength = 23488;

            PPZoneIDOffset = 16452;
        }

        override public bool Init(string ConfDirectory, ref string ErrorMessage)
        {
            OpManager = new OpCodeManager();

            if (!OpManager.Init(ConfDirectory + "\\" + PatchConfFileName, ref ErrorMessage))
                return false;

            RegisterExplorers();

            return true;
        }

        override public IdentificationStatus Identify(int OpCode, int Size, PacketDirection Direction)
        {
            if ((OpCode == OpManager.OpCodeNameToNumber("OP_ZoneEntry")) && (Direction == PacketDirection.ClientToServer))
                return IdentificationStatus.Tentative;

            if ((OpCode == OpManager.OpCodeNameToNumber("OP_PlayerProfile")) && (Direction == PacketDirection.ServerToClient) &&
                (Size == ExpectedPPLength))
                return IdentificationStatus.Yes;


            return IdentificationStatus.No;
        }

        override public int VerifyPlayerProfile()
        {
            List<byte[]> PlayerProfilePacket = GetPacketsOfType("OP_PlayerProfile", PacketDirection.ServerToClient);

            if (PlayerProfilePacket.Count == 0)
            {
                return 0;
            }
            else
            {
                if (PlayerProfilePacket[0].Length != ExpectedPPLength)
                {
                    return 0;
                }
            }

            return ExpectedPPLength;
        }

        override public UInt16 GetZoneNumber()
        {
            List<byte[]> PlayerProfilePacket = GetPacketsOfType("OP_PlayerProfile", PacketDirection.ServerToClient);

            if (PlayerProfilePacket.Count == 0)
            {
                return 0;
            }
            else
            {
                if (PlayerProfilePacket[0].Length != ExpectedPPLength)
                {
                    return 0;
                }
            }

            return BitConverter.ToUInt16(PlayerProfilePacket[0], PPZoneIDOffset);
        }

        override public List<Door> GetDoors()
        {
            List<Door> DoorList = new List<Door>();

            List<byte[]> SpawnDoorPacket = GetPacketsOfType("OP_SpawnDoor", PacketDirection.ServerToClient);

            if ((SpawnDoorPacket.Count == 0) || (SpawnDoorPacket[0].Length == 0))
                return DoorList;

            int DoorCount = SpawnDoorPacket[0].Length / 92;

            ByteStream Buffer = new ByteStream(SpawnDoorPacket[0]);

            for (int d = 0; d < DoorCount; ++d)
            {
                string DoorName = Buffer.ReadFixedLengthString(32, false);

                float YPos = Buffer.ReadSingle();

                float XPos = Buffer.ReadSingle();

                float ZPos = Buffer.ReadSingle();

                float Heading = Buffer.ReadSingle();

                UInt32 Incline = Buffer.ReadUInt32();

                Int32 Size = Buffer.ReadInt32();

                Buffer.SkipBytes(4); // Skip Unknown

                Byte DoorID = Buffer.ReadByte();

                Byte OpenType = Buffer.ReadByte();

                Byte StateAtSpawn = Buffer.ReadByte();

                Byte InvertState = Buffer.ReadByte();

                Int32 DoorParam = Buffer.ReadInt32();

                // Skip past the trailing unknowns in the door struct, moving to the next door in the packet.

                Buffer.SkipBytes(24);

                string DestZone = "NONE";

                Door NewDoor = new Door(DoorName, YPos, XPos, ZPos, Heading, Incline, Size, DoorID, OpenType, StateAtSpawn, InvertState,
                                        DoorParam, DestZone, 0, 0, 0, 0);

                DoorList.Add(NewDoor);

            }
            return DoorList;
        }

        override public MerchantManager GetMerchantData(NPCSpawnList NPCSL)
        {
            List<EQApplicationPacket> PacketList = Packets.PacketList;

            UInt32 OP_ShopRequest = OpManager.OpCodeNameToNumber("OP_ShopRequest");

            UInt32 OP_ShopEnd = OpManager.OpCodeNameToNumber("OP_ShopEnd");

            UInt32 OP_ItemPacket = OpManager.OpCodeNameToNumber("OP_ItemPacket");

            MerchantManager mm = new MerchantManager();

            for (int i = 0; i < PacketList.Count; ++i)
            {
                EQApplicationPacket p = PacketList[i];

                if ((p.Direction == PacketDirection.ServerToClient) && (p.OpCode == OP_ShopRequest))
                {
                    ByteStream Buffer = new ByteStream(p.Buffer);

                    UInt32 MerchantSpawnID = Buffer.ReadUInt32();

                    NPCSpawn npc = NPCSL.GetNPC(MerchantSpawnID);

                    UInt32 NPCTypeID;

                    if (npc != null)
                        NPCTypeID = npc.NPCTypeID;
                    else
                        NPCTypeID = 0;

                    mm.AddMerchant(MerchantSpawnID);

                    for (int j = i + 1; j < PacketList.Count; ++j)
                    {
                        p = PacketList[j];

                        if (p.OpCode == OP_ShopEnd)
                            break;


                        if (p.OpCode == OP_ItemPacket)
                        {
                            Buffer = new ByteStream(p.Buffer);

                            UInt32 StackSize = Buffer.ReadUInt32();

                            Buffer.SkipBytes(4);

                            UInt32 Slot = Buffer.ReadUInt32();

                            UInt32 MerchantSlot = Buffer.ReadUInt32();

                            UInt32 Price = Buffer.ReadUInt32();

                            Int32 Quantity = Buffer.ReadInt32();

                            Buffer.SetPosition(68); // Point to item name

                            string ItemName = Buffer.ReadString(true);

                            string Lore = Buffer.ReadString(true);

                            string IDFile = Buffer.ReadString(true);


                            UInt32 ItemID = Buffer.ReadUInt32();

                            //if (Quantity == -1)
                            mm.AddMerchantItem(MerchantSpawnID, ItemID, ItemName, MerchantSlot, Quantity);
                        }
                    }
                }
            }

            return mm;
        }

        override public List<ZonePoint> GetZonePointList()
        {
            List<ZonePoint> ZonePointList = new List<ZonePoint>();

            List<byte[]> ZonePointPackets = GetPacketsOfType("OP_SendZonepoints", PacketDirection.ServerToClient);

            if (ZonePointPackets.Count < 1)
            {
                ZonePointList = null;

                return ZonePointList;
            }

            // Assume there is only 1 packet and process the first one.

            ByteStream Buffer = new ByteStream(ZonePointPackets[0]);

            UInt32 Entries = Buffer.ReadUInt32();

            if (Entries == 0)
                return ZonePointList;

            float x, y, z, Heading;

            UInt32 Number;

            UInt16 ZoneID, Instance;

            ZonePointList = new List<ZonePoint>();

            for (int i = 0; i < Entries; ++i)
            {
                Number = Buffer.ReadUInt32();

                y = Buffer.ReadSingle();

                x = Buffer.ReadSingle();

                z = Buffer.ReadSingle();

                Heading = Buffer.ReadSingle();

                if (Heading != 999)
                    Heading = Heading / 2;

                ZoneID = Buffer.ReadUInt16();

                Instance = Buffer.ReadUInt16();

                //Buffer.SkipBytes(4);    // Skip the last UInt32

                ZonePoint NewZonePoint = new ZonePoint(Number, ZoneID, Instance, x, y, z, x, y, z, Heading, ZoneID);

                ZonePointList.Add(NewZonePoint);
            }

            return ZonePointList;
        }

        override public NewZoneStruct GetZoneData()
        {
            NewZoneStruct NewZone = new NewZoneStruct();

            List<byte[]> ZonePackets = GetPacketsOfType("OP_NewZone", PacketDirection.ServerToClient);

            if (ZonePackets.Count < 1)
                return NewZone;

            // Assume there is only 1 packet and process the first one.

            ByteStream Buffer = new ByteStream(ZonePackets[0]);

            string CharName = Buffer.ReadFixedLengthString(64, false);

            NewZone.ShortName = Buffer.ReadFixedLengthString(32, false);

            Buffer.SkipBytes(96);   // Skip Unknown

            NewZone.LongName = Buffer.ReadFixedLengthString(278, true);

            NewZone.Type = Buffer.ReadByte();

            NewZone.FogRed = Buffer.ReadBytes(4);

            NewZone.FogGreen = Buffer.ReadBytes(4);

            NewZone.FogBlue = Buffer.ReadBytes(4);

            Buffer.SkipBytes(1);   // Unknown

            for (int i = 0; i < 4; ++i)
                NewZone.FogMinClip[i] = Buffer.ReadSingle();

            for (int i = 0; i < 4; ++i)
                NewZone.FogMaxClip[i] = Buffer.ReadSingle();

            NewZone.Gravity = Buffer.ReadSingle();

            NewZone.TimeType = Buffer.ReadByte();

            Buffer.SkipBytes(49);   // Unknown

            NewZone.Sky = Buffer.ReadByte();

            Buffer.SkipBytes(13);   // Unknown

            NewZone.ZEM = Buffer.ReadSingle();

            NewZone.SafeY = Buffer.ReadSingle();

            NewZone.SafeX = Buffer.ReadSingle();

            NewZone.SafeZ = Buffer.ReadSingle();

            NewZone.MinZ = Buffer.ReadSingle();

            NewZone.MaxZ = Buffer.ReadSingle();

            NewZone.UnderWorld = Buffer.ReadSingle();

            NewZone.MinClip = Buffer.ReadSingle();

            NewZone.MaxClip = Buffer.ReadSingle();

            Buffer.SkipBytes(84);   // Unknown

            NewZone.ShortName2 = Buffer.ReadFixedLengthString(96, false);

            Buffer.SkipBytes(52);   // Unknown

            NewZone.ZoneID = Buffer.ReadUInt16();

            NewZone.InstanceID = Buffer.ReadUInt16();

            Buffer.SkipBytes(38);   // Unknown

            NewZone.FallDamage = Buffer.ReadByte();

            Buffer.SkipBytes(21);   // Unknown

            NewZone.FogDensity = Buffer.ReadSingle();

            // Everything else after this point in the packet is unknown.

            return NewZone;
        }

        override public List<ZoneEntryStruct> GetSpawns()
        {
            List<ZoneEntryStruct> ZoneSpawns = new List<ZoneEntryStruct>();

            List<byte[]> SpawnPackets = GetPacketsOfType("OP_ZoneEntry", PacketDirection.ServerToClient);

            foreach (byte[] SpawnPacket in SpawnPackets)
            {
                ZoneEntryStruct NewSpawn = new ZoneEntryStruct();

                ByteStream Buffer = new ByteStream(SpawnPacket);

                NewSpawn.SpawnName = Buffer.ReadString(true);

                NewSpawn.SpawnName = Utils.MakeCleanName(NewSpawn.SpawnName);

                NewSpawn.SpawnID = Buffer.ReadUInt32();

                NewSpawn.Level = Buffer.ReadByte();

                float UnkSize = Buffer.ReadSingle();

                NewSpawn.IsNPC = Buffer.ReadByte();

                UInt32 Bitfield = Buffer.ReadUInt32();

                NewSpawn.Showname = (Bitfield >> 28) & 1;
                NewSpawn.TargetableWithHotkey = (Bitfield >> 27) & 1;
                NewSpawn.Targetable = (Bitfield >> 26) & 1;

                NewSpawn.ShowHelm = (Bitfield >> 24) & 1;
                NewSpawn.Gender = (Bitfield >> 20) & 3;

                Byte OtherData = Buffer.ReadByte();

                Buffer.SkipBytes(8);    // Skip 8 unknown bytes

                NewSpawn.DestructableString1 = "";
                NewSpawn.DestructableString2 = "";
                NewSpawn.DestructableString3 = "";

                if ((OtherData & 1) > 0)
                {
                    //SQLOut(String.Format("-- OBJECT FOUND SpawnID {0}", SpawnID.ToString("x")));
                    //Console.WriteLine("Otherdata is {0} Skipping Stuff.", OtherData.ToString("x"));
                    NewSpawn.DestructableString1 = Buffer.ReadString(false);

                    NewSpawn.DestructableString2 = Buffer.ReadString(false);

                    NewSpawn.DestructableString3 = Buffer.ReadString(false);

                    NewSpawn.DestructableUnk1 = Buffer.ReadUInt32();

                    NewSpawn.DestructableUnk2 = Buffer.ReadUInt32();

                    NewSpawn.DestructableID1 = Buffer.ReadUInt32();

                    NewSpawn.DestructableID2 = Buffer.ReadUInt32();

                    NewSpawn.DestructableID3 = Buffer.ReadUInt32();

                    NewSpawn.DestructableID4 = Buffer.ReadUInt32();

                    NewSpawn.DestructableUnk3 = Buffer.ReadUInt32();

                    NewSpawn.DestructableUnk4 = Buffer.ReadUInt32();

                    NewSpawn.DestructableUnk5 = Buffer.ReadUInt32();

                    NewSpawn.DestructableUnk6 = Buffer.ReadUInt32();

                    NewSpawn.DestructableUnk7 = Buffer.ReadUInt32();

                    NewSpawn.DestructableUnk8 = Buffer.ReadUInt32();

                    NewSpawn.DestructableUnk9 = Buffer.ReadUInt32();

                    NewSpawn.DestructableByte = Buffer.ReadByte();

                    //SQLOut(String.Format("-- DES: {0,8:x} {1,8:x} {2,8:d} {3,8:d} {4,8:d} {5,8:d} {6,8:x} {7,8:x} {8,8:x} {9,8:x} {10,8:x} {11,8:x} {12,8:x} {13,2:x} {14} {15} {16}",
                    //          DestructableUnk1, DestructableUnk2, DestructableID1, DestructableID2, DestructableID3, DestructableID4,
                    //          DestructableUnk3, DestructableUnk4, DestructableUnk5, DestructableUnk6, DestructableUnk7, DestructableUnk8,
                    //          DestructableUnk9, DestructableByte, DestructableString1, DestructableString2, DestructableString3));
                }

                NewSpawn.Size = Buffer.ReadSingle();

                NewSpawn.Face = Buffer.ReadByte();

                NewSpawn.WalkSpeed = Buffer.ReadSingle();

                NewSpawn.RunSpeed = Buffer.ReadSingle();

                NewSpawn.Race = Buffer.ReadUInt32();

                NewSpawn.PropCount = Buffer.ReadByte();

                NewSpawn.BodyType = 0;

                if (NewSpawn.PropCount >= 1)
                {
                    NewSpawn.BodyType = Buffer.ReadUInt32();

                    for (int j = 1; j < NewSpawn.PropCount; ++j)
                        Buffer.SkipBytes(4);
                }

                Buffer.SkipBytes(1);   // Skip HP %

                NewSpawn.HairColor = Buffer.ReadByte();
                NewSpawn.BeardColor = Buffer.ReadByte();
                NewSpawn.EyeColor1 = Buffer.ReadByte();
                NewSpawn.EyeColor2 = Buffer.ReadByte();
                NewSpawn.HairStyle = Buffer.ReadByte();
                NewSpawn.Beard = Buffer.ReadByte();

                NewSpawn.DrakkinHeritage = Buffer.ReadUInt32();

                NewSpawn.DrakkinTattoo = Buffer.ReadUInt32();

                NewSpawn.DrakkinDetails = Buffer.ReadUInt32();

                Buffer.SkipBytes(1);   // Skip Holding

                NewSpawn.Deity = Buffer.ReadUInt32();

                Buffer.SkipBytes(8);    // Skip GuildID and GuildRank

                NewSpawn.Class = Buffer.ReadByte();

                Buffer.SkipBytes(4);     // Skip PVP, Standstate, Light, Flymode

                NewSpawn.EquipChest2 = Buffer.ReadByte();

                bool UseWorn = (NewSpawn.EquipChest2 == 255);

                Buffer.SkipBytes(2);    // 2 Unknown bytes;

                NewSpawn.Helm = Buffer.ReadByte();

                NewSpawn.LastName = Buffer.ReadString(false);

                Buffer.SkipBytes(5);    // AATitle + unknown byte

                NewSpawn.PetOwnerID = Buffer.ReadUInt32();

                Buffer.SkipBytes(25);   // Unknown byte + 6 unknown uint32

                UInt32 Position1 = Buffer.ReadUInt32();

                UInt32 Position2 = Buffer.ReadUInt32();

                UInt32 Position3 = Buffer.ReadUInt32();

                UInt32 Position4 = Buffer.ReadUInt32();

                UInt32 Position5 = Buffer.ReadUInt32();

                NewSpawn.YPos = Utils.EQ19ToFloat((Int32)(Position3 & 0x7FFFF));

                NewSpawn.Heading = Utils.EQ19ToFloat((Int32)(Position4 & 0xFFF));

                NewSpawn.XPos = Utils.EQ19ToFloat((Int32)(Position4 >> 12) & 0x7FFFF);

                NewSpawn.ZPos = Utils.EQ19ToFloat((Int32)(Position5 & 0x7FFFF));

                for (int ColourSlot = 0; ColourSlot < 9; ++ColourSlot)
                    NewSpawn.SlotColour[ColourSlot] = Buffer.ReadUInt32();

                NewSpawn.MeleeTexture1 = 0;
                NewSpawn.MeleeTexture2 = 0;

                if (NPCType.IsPlayableRace(NewSpawn.Race))
                {
                    for (int i = 0; i < 9; ++i)
                    {
                        NewSpawn.Equipment[i] = Buffer.ReadUInt32();

                        UInt32 Equip1 = Buffer.ReadUInt32();

                        UInt32 Equip0 = Buffer.ReadUInt32();
                    }

                    if (NewSpawn.Equipment[Constants.MATERIAL_CHEST] > 0)
                    {
                        NewSpawn.EquipChest2 = (byte)NewSpawn.Equipment[Constants.MATERIAL_CHEST];

                    }

                    NewSpawn.ArmorTintRed = (byte)((NewSpawn.SlotColour[Constants.MATERIAL_CHEST] >> 16) & 0xff);

                    NewSpawn.ArmorTintGreen = (byte)((NewSpawn.SlotColour[Constants.MATERIAL_CHEST] >> 8) & 0xff);

                    NewSpawn.ArmorTintBlue = (byte)(NewSpawn.SlotColour[Constants.MATERIAL_CHEST] & 0xff);

                    if (NewSpawn.Equipment[Constants.MATERIAL_PRIMARY] > 0)
                        NewSpawn.MeleeTexture1 = NewSpawn.Equipment[Constants.MATERIAL_PRIMARY];

                    if (NewSpawn.Equipment[Constants.MATERIAL_SECONDARY] > 0)
                        NewSpawn.MeleeTexture2 = NewSpawn.Equipment[Constants.MATERIAL_SECONDARY];

                    if (UseWorn)
                        NewSpawn.Helm = (byte)NewSpawn.Equipment[Constants.MATERIAL_HEAD];
                    else
                        NewSpawn.Helm = 0;

                }
                else
                {
                    // Non playable race
                    NewSpawn.MeleeTexture1 = NewSpawn.SlotColour[3];
                    NewSpawn.MeleeTexture2 = NewSpawn.SlotColour[6];

                }

                if (NewSpawn.EquipChest2 == 255)
                    NewSpawn.EquipChest2 = 0;

                if (NewSpawn.Helm == 255)
                    NewSpawn.Helm = 0;

                if ((OtherData & 4) > 0)
                {
                    NewSpawn.Title = Buffer.ReadString(false);
                }

                if ((OtherData & 8) > 0)
                {
                    NewSpawn.Suffix = Buffer.ReadString(false);
                }

                // unknowns
                Buffer.SkipBytes(8);

                NewSpawn.IsMercenary = Buffer.ReadByte();

                ZoneSpawns.Add(NewSpawn);
            }

            return ZoneSpawns;
        }

        override public List<PositionUpdate> GetHighResolutionMovementUpdates()
        {
            List<PositionUpdate> Updates = new List<PositionUpdate>();

            List<byte[]> UpdatePackets = GetPacketsOfType("OP_NPCMoveUpdate", PacketDirection.ServerToClient);

            foreach (byte[] UpdatePacket in UpdatePackets)
            {
                PositionUpdate PosUpdate = new PositionUpdate();

                BitStream bs = new BitStream(UpdatePacket, 13);

                PosUpdate.SpawnID = bs.readUInt(16);

                UInt32 VFlags = bs.readUInt(6);

                PosUpdate.p.y = (float)bs.readInt(19) / (float)(1 << 3);

                PosUpdate.p.x = (float)bs.readInt(19) / (float)(1 << 3);

                PosUpdate.p.z = (float)bs.readInt(19) / (float)(1 << 3);

                PosUpdate.p.heading = (float)bs.readInt(12) / (float)(1 << 3);

                Updates.Add(PosUpdate);
            }

            return Updates;
        }

        override public List<PositionUpdate> GetLowResolutionMovementUpdates()
        {
            List<PositionUpdate> Updates = new List<PositionUpdate>();

            List<byte[]> UpdatePackets = GetPacketsOfType("OP_MobUpdate", PacketDirection.ServerToClient);

            foreach (byte[] MobUpdatePacket in UpdatePackets)
            {
                PositionUpdate PosUpdate = new PositionUpdate();

                ByteStream Buffer = new ByteStream(MobUpdatePacket);

                PosUpdate.SpawnID = Buffer.ReadUInt16();

                UInt32 Word1 = Buffer.ReadUInt32();

                UInt32 Word2 = Buffer.ReadUInt32();

                UInt16 Word3 = Buffer.ReadUInt16();

                PosUpdate.p.y = Utils.EQ19ToFloat((Int32)(Word1 & 0x7FFFF));

                // Z is in the top 13 bits of Word1 and the bottom 6 of Word2

                UInt32 ZPart1 = Word1 >> 19;    // ZPart1 now has low order bits of Z in bottom 13 bits
                UInt32 ZPart2 = Word2 & 0x3F;   // ZPart2 now has low order bits of Z in bottom 6 bits

                ZPart2 = ZPart2 << 13;

                PosUpdate.p.z = Utils.EQ19ToFloat((Int32)(ZPart1 | ZPart2));

                PosUpdate.p.x = Utils.EQ19ToFloat((Int32)(Word2 >> 6) & 0x7FFFF);

                PosUpdate.p.heading = Utils.EQ19ToFloat((Int32)(Word3 & 0xFFF));

                Updates.Add(PosUpdate);
            }

            return Updates;
        }

        override public List<GroundSpawnStruct> GetGroundSpawns()
        {
            List<GroundSpawnStruct> GroundSpawns = new List<GroundSpawnStruct>();

            List<byte[]> GroundSpawnPackets = GetPacketsOfType("OP_GroundSpawn", PacketDirection.ServerToClient);

            foreach (byte[] GroundSpawnPacket in GroundSpawnPackets)
            {
                GroundSpawnStruct GroundSpawn = new GroundSpawnStruct();

                ByteStream Buffer = new ByteStream(GroundSpawnPacket);

                Buffer.SkipBytes(12);

                GroundSpawn.DropID = Buffer.ReadUInt32();

                GroundSpawn.ZoneID = Buffer.ReadUInt16();

                GroundSpawn.InstanceID = Buffer.ReadUInt16();

                Buffer.SkipBytes(8);

                GroundSpawn.Heading = Buffer.ReadSingle();

                Buffer.SkipBytes(12);

                GroundSpawn.z = Buffer.ReadSingle();

                GroundSpawn.x = Buffer.ReadSingle();

                GroundSpawn.y = Buffer.ReadSingle();

                GroundSpawn.Name = Buffer.ReadFixedLengthString(16, false);

                Buffer.SkipBytes(20);

                GroundSpawn.ObjectType = Buffer.ReadUInt32();

                GroundSpawns.Add(GroundSpawn);
            }
            return GroundSpawns;
        }

        override public List<UInt32> GetFindableSpawns()
        {
            List<UInt32> FindableSpawnList = new List<UInt32>();

            List<byte[]> FindablePackets = GetPacketsOfType("OP_SendFindableNPCs", PacketDirection.ServerToClient);

            if (FindablePackets.Count < 1)
                return FindableSpawnList;

            foreach (byte[] Packet in FindablePackets)
            {
                if (BitConverter.ToUInt32(Packet, 0) == 0)
                    FindableSpawnList.Add(BitConverter.ToUInt32(Packet, 4));
            }

            return FindableSpawnList;
        }

        override public string GetZoneName()
        {
            List<byte[]> NewZonePacket = GetPacketsOfType("OP_NewZone", PacketDirection.ServerToClient);

            if (NewZonePacket.Count != 1)
                return "";

            return Utils.ReadNullTerminatedString(NewZonePacket[0], 704, 96, false);
        }

        public override void RegisterExplorers()
        {
            OpManager.RegisterExplorer("OP_ZoneEntry", ExploreZoneEntry);
            //OpManager.RegisterExplorer("OP_RespawnWindow", ExploreRespawnWindow);
            //OpManager.RegisterExplorer("OP_ZonePlayerToBind", ExploreZonePlayerToBind);
            //OpManager.RegisterExplorer("OP_RequestClientZoneChange", ExploreRequestClientZoneChange);
            //OpManager.RegisterExplorer("OP_DeleteSpawn", ExploreDeleteSpawn);
            OpManager.RegisterExplorer("OP_HPUpdate", ExploreHPUpdate);

        }

        public void ExploreZoneEntry(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            if (Direction != PacketDirection.ServerToClient)
                return;

            string Name = Buffer.ReadString(false);
            UInt32 SpawnID = Buffer.ReadUInt32();
            byte Level = Buffer.ReadByte();
            Buffer.SkipBytes(4);
            bool IsNPC = (Buffer.ReadByte() != 0);
            UInt32 Bitfield = Buffer.ReadUInt32();

            string DestructableString1;
            string DestructableString2;
            string DestructableString3;
            UInt32 DestructableUnk1;
            UInt32 DestructableUnk2;
            UInt32 DestructableID1;
            UInt32 DestructableID2;
            UInt32 DestructableID3;
            UInt32 DestructableID4;
            UInt32 DestructableUnk3;
            UInt32 DestructableUnk4;
            UInt32 DestructableUnk5;
            UInt32 DestructableUnk6;
            UInt32 DestructableUnk7;
            UInt32 DestructableUnk8;
            UInt32 DestructableUnk9;
            byte DestructableByte;

            Byte OtherData = Buffer.ReadByte();

            Buffer.SkipBytes(8);    // Skip 8 unknown bytes

            DestructableString1 = "";
            DestructableString2 = "";
            DestructableString3 = "";

            OutputStream.WriteLine("Spawn Name: {0} ID: {1} Level: {2} {3}\r\n", Name, SpawnID, Level, IsNPC ? "NPC" : "Player");

            if ((OtherData & 1) > 0)
            {
                // Destructable Objects.

                DestructableString1 = Buffer.ReadString(false);

                DestructableString2 = Buffer.ReadString(false);

                DestructableString3 = Buffer.ReadString(false);

                DestructableUnk1 = Buffer.ReadUInt32();

                DestructableUnk2 = Buffer.ReadUInt32();

                DestructableID1 = Buffer.ReadUInt32();

                DestructableID2 = Buffer.ReadUInt32();

                DestructableID3 = Buffer.ReadUInt32();

                DestructableID4 = Buffer.ReadUInt32();

                DestructableUnk3 = Buffer.ReadUInt32();

                DestructableUnk4 = Buffer.ReadUInt32();

                DestructableUnk5 = Buffer.ReadUInt32();

                DestructableUnk6 = Buffer.ReadUInt32();

                DestructableUnk7 = Buffer.ReadUInt32();

                DestructableUnk8 = Buffer.ReadUInt32();

                DestructableUnk9 = Buffer.ReadUInt32();

                DestructableByte = Buffer.ReadByte();

                OutputStream.WriteLine("DESTRUCTABLE OBJECT:\r\n");
                OutputStream.WriteLine(" String1: {0}", DestructableString1);
                OutputStream.WriteLine(" String2: {0}", DestructableString2);
                OutputStream.WriteLine(" String3: {0}\r\n", DestructableString3);

                OutputStream.WriteLine(" Unk1: {0,8:x} Unk2: {1,8:x}\r\n ID1 : {2,8:x} ID2 : {3,8:x} ID3 : {4,8:x} ID4 : {5,8:x}\r\n Unk3: {6,8:x} Unk4: {7,8:x} Unk5: {8,8:x} Unk6: {9,8:x}\r\n Unk7: {10,8:x} Unk8: {11,8:x} Unk9: {12,8:x}\r\n UnkByte:    {13,2:x}",
                          DestructableUnk1, DestructableUnk2, DestructableID1, DestructableID2, DestructableID3, DestructableID4,
                          DestructableUnk3, DestructableUnk4, DestructableUnk5, DestructableUnk6, DestructableUnk7, DestructableUnk8,
                          DestructableUnk9, DestructableByte);
            }

            Buffer.SkipBytes(17);

            byte PropCount = Buffer.ReadByte();

            if (PropCount >= 1)
            {
                Buffer.SkipBytes(4);

                for (int j = 1; j < PropCount; ++j)
                    Buffer.SkipBytes(4);
            }

            byte HP = Buffer.ReadByte();

            OutputStream.WriteLine("HP% is {0}\r\n", HP);

            AddExplorerSpawn(SpawnID, Name);
        }

        public void ExploreHPUpdate(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt32 CurrentHP = Buffer.ReadUInt32();
            Int32 MaxHP = Buffer.ReadInt32();
            UInt16 SpawnID = Buffer.ReadUInt16();

            string SpawnName = FindExplorerSpawn(SpawnID);

            OutputStream.WriteLine("Spawn {0} {1} Current HP: {2} Max HP: {3}", SpawnID, SpawnName, CurrentHP, MaxHP);

            OutputStream.WriteLine("");
        }

        override public bool DumpAAs(string FileName)
        {
            List<byte[]> AAPackets = GetPacketsOfType("OP_SendAATable", PacketDirection.ServerToClient);


            if (AAPackets.Count < 1)
                return false;

            StreamWriter OutputFile;

            try
            {
                OutputFile = new StreamWriter(FileName);
            }
            catch
            {
                return false;

            }

            OutputFile.WriteLine("-- There are " + AAPackets.Count + " OP_SendAATable packets.");
            OutputFile.WriteLine("");

            foreach (byte[] Packet in AAPackets)
            {
                UInt32 AAID = BitConverter.ToUInt32(Packet, 0);
                UInt32 Unknown004 = BitConverter.ToUInt32(Packet, 4);
                UInt32 HotKeySID = BitConverter.ToUInt32(Packet, 5);
                UInt32 HotKeySID2 = BitConverter.ToUInt32(Packet, 9);
                UInt32 TitleSID = BitConverter.ToUInt32(Packet, 13);
                UInt32 DescSID = BitConverter.ToUInt32(Packet, 17);
                UInt32 ClassType = BitConverter.ToUInt32(Packet, 21);
                UInt32 Cost = BitConverter.ToUInt32(Packet, 25);
                UInt32 Seq = BitConverter.ToUInt32(Packet, 29);
                UInt32 CurrentLevel = BitConverter.ToUInt32(Packet, 33);
                UInt32 PrereqSkill = BitConverter.ToUInt32(Packet, 37);
                UInt32 PrereqMinpoints = BitConverter.ToUInt32(Packet, 41);
                UInt32 Type = BitConverter.ToUInt32(Packet, 45);
                UInt32 SpellID = BitConverter.ToUInt32(Packet, 49);
                UInt32 SpellType = BitConverter.ToUInt32(Packet, 53);
                UInt32 SpellRefresh = BitConverter.ToUInt32(Packet, 57);
                UInt16 Classes = BitConverter.ToUInt16(Packet, 61);
                UInt16 Berserker = BitConverter.ToUInt16(Packet, 63);
                UInt32 MaxLevel = BitConverter.ToUInt32(Packet, 65);
                UInt32 LastID = BitConverter.ToUInt32(Packet, 69);
                UInt32 NextID = BitConverter.ToUInt32(Packet, 73);
                UInt32 Cost2 = BitConverter.ToUInt32(Packet, 77);
                UInt32 AAExpansion = BitConverter.ToUInt32(Packet, 88);
                UInt32 SpecialCategory = BitConverter.ToUInt32(Packet, 92);
                UInt32 TotalAbilities = BitConverter.ToUInt32(Packet, 100);

                OutputFile.WriteLine(String.Format("AAID: {0}", AAID));
                OutputFile.WriteLine(" Unknown004:\t" + Unknown004);
                OutputFile.WriteLine(" HotkeySID:\t" + HotKeySID);
                OutputFile.WriteLine(" HotkeySID2:\t" + HotKeySID2);
                OutputFile.WriteLine(" TitleSID:\t" + TitleSID);
                OutputFile.WriteLine(" DescSID:\t" + DescSID);
                OutputFile.WriteLine(" ClassType:\t" + ClassType);
                OutputFile.WriteLine(" Cost:\t\t" + Cost);
                OutputFile.WriteLine(" Seq:\t\t" + Seq);
                OutputFile.WriteLine(" CurrentLevel:\t" + CurrentLevel);
                OutputFile.WriteLine(" PrereqSkill:\t" + PrereqSkill);
                OutputFile.WriteLine(" PrereqMinPt:\t" + PrereqMinpoints);
                OutputFile.WriteLine(" Type:\t\t" + Type);
                OutputFile.WriteLine(" SpellID:\t" + SpellID);
                OutputFile.WriteLine(" SpellType:\t" + SpellType);
                OutputFile.WriteLine(" SpellRefresh:\t" + SpellRefresh);
                OutputFile.WriteLine(" Classes:\t" + Classes);
                OutputFile.WriteLine(" Berserker:\t" + Berserker);
                OutputFile.WriteLine(" MaxLevel:\t" + MaxLevel);
                OutputFile.WriteLine(" LastID:\t" + LastID);
                OutputFile.WriteLine(" NextID:\t" + NextID);
                OutputFile.WriteLine(" Cost2:\t\t" + Cost2);
                OutputFile.WriteLine(" AAExpansion:\t" + AAExpansion);
                OutputFile.WriteLine(" SpecialCat:\t" + SpecialCategory);
                OutputFile.WriteLine("");
                OutputFile.WriteLine(" TotalAbilities:\t" + TotalAbilities);
                OutputFile.WriteLine("");

                for (int i = 0; i < TotalAbilities; ++i)
                {
                    UInt32 Ability = BitConverter.ToUInt32(Packet, 104 + (i * 16));
                    Int32 Base1 = BitConverter.ToInt32(Packet, 108 + (i * 16));
                    Int32 Base2 = BitConverter.ToInt32(Packet, 112 + (i * 16));
                    UInt32 Slot = BitConverter.ToUInt32(Packet, 116 + (i * 16));

                    OutputFile.WriteLine(String.Format("    Ability:\t{0}\tBase1:\t{1}\tBase2:\t{2}\tSlot:\t{3}", Ability, Base1, Base2, Slot));

                }
                OutputFile.WriteLine("");

            }

            OutputFile.Close();

            return true;
        }
    }
}
