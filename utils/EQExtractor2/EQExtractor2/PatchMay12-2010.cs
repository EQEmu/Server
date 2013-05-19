//
// Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net). Distributed under GPL version 2.
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
    class PatchMay122010Decoder : PatchSpecficDecoder
    {
        // The date specified in the version and patch conf file name should be the first build that this patch supports. If later
        // patches don't break anything, then there is no need to create a new PatchSpecificDecoder just because the client build
        // date has changed.

        // I expect in most cases if a new client patch is released that breaks the extractor, than a new decoder derived from the previous
        // version's decoder would be created, rather than creating a brand new one based off the base class, unless all of the previous
        // decoder's methods need replacing to support the new patch.
        //
        public PatchMay122010Decoder()
        {
            Version = "EQ Client Build Date May 12 2010. (Valid up to and including Build Date June 8 2010)";

            PatchConfFileName = "patch_May12-2010.conf";

            ExpectedPPLength = 26632;

            PPZoneIDOffset = 19396;
        }

        override public bool Init(string ConfDirectory, ref string ErrorMessage)
        {
            OpManager = new OpCodeManager();

            if (!OpManager.Init(ConfDirectory + "\\" + PatchConfFileName, ref ErrorMessage))
                return false;

            RegisterExplorers();

            return true;
        }

        // This method is used to identify a particular patch version
        //
        // The method should always return 'Tentative' for the Client->Server OP_ZoneEntry
        //
        // 'Yes' should only be returned when a packet uniquely identifying this particular patch version has been seen.
        //
        // When new patches are added, the Identify method of older patch decoders may need updating to ensure they identify a
        // patch uniquely.
        //
        //
        override public IdentificationStatus Identify(int OpCode, int Size, PacketDirection Direction)
        {
            if((OpCode == OpManager.OpCodeNameToNumber("OP_ZoneEntry")) && (Direction == PacketDirection.ClientToServer))
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
            // A return value of zero from this method should be intepreted as 'Unable to identify patch version'.

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

                        if ((p.OpCode == OP_ShopEnd) || (p.OpCode == OP_ShopRequest))
                            break;

                        if (p.OpCode == OP_ItemPacket)
                        {
                            Item NewItem = DecodeItemPacket(p.Buffer);

                            mm.AddMerchantItem(MerchantSpawnID, NewItem.ID, NewItem.Name, NewItem.MerchantSlot, NewItem.Quantity);
                        }
                    }
                }
            }

            return mm;
        }

        override public Item DecodeItemPacket(byte[] PacketBuffer)
        {
            ByteStream Buffer = new ByteStream(PacketBuffer);

            Item NewItem = new Item();

            NewItem.StackSize = Buffer.ReadUInt32();
            Buffer.SkipBytes(4);
            NewItem.Slot = Buffer.ReadUInt32();
            NewItem.MerchantSlot = Buffer.ReadUInt32();
            NewItem.Price = Buffer.ReadUInt32();
            NewItem.Quantity = Buffer.ReadInt32();
            Buffer.SetPosition(68);
            NewItem.Name = Buffer.ReadString(true);
            NewItem.Lore = Buffer.ReadString(true);
            NewItem.IDFile = Buffer.ReadString(true);
            NewItem.ID = Buffer.ReadUInt32();

            return NewItem;
        }

        override public List<ZonePoint> GetZonePointList()
        {
            List<ZonePoint> ZonePointList = new List<ZonePoint>();

            List<byte[]> ZonePointPackets = GetPacketsOfType("OP_SendZonepoints", PacketDirection.ServerToClient);

            if (ZonePointPackets.Count < 1)
            {
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

                Buffer.SkipBytes(4);    // Skip the last UInt32

                ZonePoint NewZonePoint = new ZonePoint(Number, ZoneID, Instance, x, y, z,  x, y, z, Heading, ZoneID);

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

                NewSpawn.Padding5 = (Bitfield >> 4) & 1;
                NewSpawn.Padding7 = (Bitfield >> 6) & 2047;
                NewSpawn.Padding26 = (Bitfield >> 25) & 1;

                Byte OtherData = Buffer.ReadByte();

                Buffer.SkipBytes(8);    // Skip 8 unknown bytes

                NewSpawn.DestructableString1 = "";
                NewSpawn.DestructableString2 = "";
                NewSpawn.DestructableString3 = "";

                if ((NewSpawn.IsNPC == 1) && ((OtherData & 3) > 0))
                {
                    // Destructable Objects. Not handled yet
                    //
                    //SQLOut(String.Format("-- OBJECT FOUND SpawnID {0}", SpawnID.ToString("x")));

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
                Updates.Add(Decode_OP_NPCMoveUpdate(UpdatePacket));

            return Updates;
        }

        override public  PositionUpdate Decode_OP_NPCMoveUpdate(byte[] UpdatePacket)
        {
            PositionUpdate PosUpdate = new PositionUpdate();

            BitStream bs = new BitStream(UpdatePacket, 13);

            PosUpdate.SpawnID = bs.readUInt(16);

            UInt32 VFlags = bs.readUInt(6);

            PosUpdate.p.y = (float)bs.readInt(19) / (float)(1 << 3);

            PosUpdate.p.x = (float)bs.readInt(19) / (float)(1 << 3);

            PosUpdate.p.z = (float)bs.readInt(19) / (float)(1 << 3);

            PosUpdate.p.heading = (float)bs.readInt(12) / (float)(1 << 3);

            PosUpdate.HighRes = true;

            return PosUpdate;
        }

        override public List<PositionUpdate> GetLowResolutionMovementUpdates()
        {
            List<PositionUpdate> Updates = new List<PositionUpdate>();

            List<byte[]> UpdatePackets = GetPacketsOfType("OP_MobUpdate", PacketDirection.ServerToClient);

            foreach (byte[] MobUpdatePacket in UpdatePackets)
                Updates.Add(Decode_OP_MobUpdate(MobUpdatePacket));

            return Updates;
        }

        override public PositionUpdate Decode_OP_MobUpdate(byte[] MobUpdatePacket)
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
            UInt32 ZPart2 = Word2 & 0x3F;   // ZPart2 now has high order bits of Z in bottom 6 bits

            ZPart2 = ZPart2 << 13;

            PosUpdate.p.z = Utils.EQ19ToFloat((Int32)(ZPart1 | ZPart2));

            PosUpdate.p.x = Utils.EQ19ToFloat((Int32)(Word2 >> 6) & 0x7FFFF);

            PosUpdate.p.heading = Utils.EQ19ToFloat((Int32)(Word3 & 0xFFF));

            PosUpdate.HighRes = false;

            return PosUpdate;
        }

        override public List<PositionUpdate> GetAllMovementUpdates()
        {
            PositionUpdate PosUpdate = new PositionUpdate();

            List<PositionUpdate> Updates = new List<PositionUpdate>();

            List<EQApplicationPacket> PacketList = Packets.PacketList;

            UInt32 OP_NPCMoveUpdate = OpManager.OpCodeNameToNumber("OP_NPCMoveUpdate");

            UInt32 OP_MobUpdate = OpManager.OpCodeNameToNumber("OP_MobUpdate");

            for (int i = 0; i < PacketList.Count; ++i)
            {
                EQApplicationPacket p = PacketList[i];

                if (p.Direction == PacketDirection.ServerToClient)
                {
                    if (p.OpCode == OP_NPCMoveUpdate)
                    {
                        PosUpdate = Decode_OP_NPCMoveUpdate(p.Buffer);
                        PosUpdate.p.TimeStamp = p.PacketTime;
                        Updates.Add(PosUpdate);
                    }
                    else if (p.OpCode == OP_MobUpdate)
                    {
                        PosUpdate = Decode_OP_MobUpdate(p.Buffer);
                        PosUpdate.p.TimeStamp = p.PacketTime;
                        Updates.Add(PosUpdate);
                    }
                }
            }

            return Updates;
        }

        override public List<PositionUpdate> GetClientMovementUpdates()
        {
            List<PositionUpdate> Updates = new List<PositionUpdate>();

            List<EQApplicationPacket> PacketList = Packets.PacketList;

            UInt32 OP_ClientUpdate = OpManager.OpCodeNameToNumber("OP_ClientUpdate");

            foreach (EQApplicationPacket UpdatePacket in PacketList)
            {
                if ((UpdatePacket.OpCode != OP_ClientUpdate) || (UpdatePacket.Direction != PacketDirection.ClientToServer))
                    continue;

                ByteStream Buffer = new ByteStream(UpdatePacket.Buffer);

                PositionUpdate PosUpdate = new PositionUpdate();

                PosUpdate.SpawnID = Buffer.ReadUInt16();
                Buffer.SkipBytes(6);
                PosUpdate.p.x = Buffer.ReadSingle();
                PosUpdate.p.y = Buffer.ReadSingle();
                Buffer.SkipBytes(12);
                PosUpdate.p.z = Buffer.ReadSingle();
                PosUpdate.p.TimeStamp = UpdatePacket.PacketTime;
                Buffer.SkipBytes(4);
                UInt32 Temp = Buffer.ReadUInt32();
                Temp = Temp & 0x3FFFFF;
                Temp = Temp >> 10;
                PosUpdate.p.heading = Utils.EQ19ToFloat((Int32)(Temp));


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

                GroundSpawn.DropID = Buffer.ReadUInt32();

                GroundSpawn.Name = Buffer.ReadString(false);

                GroundSpawn.ZoneID = Buffer.ReadUInt16();

                GroundSpawn.InstanceID = Buffer.ReadUInt16();

                Buffer.SkipBytes(8); // Two unknown uint32s

                GroundSpawn.Heading = Buffer.ReadSingle();

                Buffer.SkipBytes(12); // First float is 255 to make some groundspawns appear, second 4 bytes unknown, last is a float

                GroundSpawn.y = Buffer.ReadSingle();

                GroundSpawn.x = Buffer.ReadSingle();

                GroundSpawn.z = Buffer.ReadSingle();

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
            //OpManager.RegisterExplorer("OP_NewZone", ExploreNewZonePacket);
            //OpManager.RegisterExplorer("OP_ZoneEntry", ExploreZoneEntry);
            //OpManager.RegisterExplorer("OP_PlayerProfile", ExplorePlayerProfile);
            //OpManager.RegisterExplorer("OP_RespawnWindow", ExploreRespawnWindow);
            //OpManager.RegisterExplorer("OP_ZonePlayerToBind", ExploreZonePlayerToBind);
            //OpManager.RegisterExplorer("OP_RequestClientZoneChange", ExploreRequestClientZoneChange);
            //OpManager.RegisterExplorer("OP_DeleteSpawn", ExploreDeleteSpawn);
            //OpManager.RegisterExplorer("OP_SpawnAppearance", ExploreSpawnAppearance);
            //OpManager.RegisterExplorer("OP_HPUpdate", ExploreHPUpdate);
            //OpManager.RegisterExplorer("OP_Animation", ExploreAnimation);
            //OpManager.RegisterExplorer("OP_CharInventory", ExploreCharInventoryPacket);

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

        public void ExploreSpawnAppearance(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt16 SpawnID = Buffer.ReadUInt16();
            UInt16 Type = Buffer.ReadUInt16();
            UInt32 Param = Buffer.ReadUInt32();
            string SpawnName = FindExplorerSpawn(SpawnID);

            OutputStream.WriteLine("Spawn {0} {1} Appearance Change Type {2} Parameter {3}", SpawnID, SpawnName, Type, Param);

            OutputStream.WriteLine("");
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

        public void ExploreAnimation(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt16 SpawnID = Buffer.ReadUInt16();
            byte Action = Buffer.ReadByte();
            byte Value = Buffer.ReadByte();

            string SpawnName = FindExplorerSpawn(SpawnID);

            OutputStream.WriteLine("Spawn {0} {1} Action: {2} Value: {3}", SpawnID, SpawnName, Action, Value);

            OutputStream.WriteLine("");
        }

        public void ExploreNewZonePacket(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            Buffer.SetPosition(704);

            OutputStream.WriteLine("Zone name is {0}\r\n", Buffer.ReadString(false));
        }

        public void ExplorePlayerProfile(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            Buffer.SetPosition(12);

            UInt32 PlayerClass = Buffer.ReadUInt32();

            Buffer.SetPosition(56);

            byte PlayerLevel = Buffer.ReadByte();

            Buffer.SetPosition(18100);

            OutputStream.WriteLine("Name: {0} Class: {1} Level: {2}\r\n", Buffer.ReadString(false), PlayerClass, PlayerLevel);
        }

        public void ExploreRespawnWindow(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            if (Direction == PacketDirection.ServerToClient)
            {
                UInt32 Unknown000 = Buffer.ReadUInt32();
                UInt32 TimeRemaining = Buffer.ReadUInt32();
                UInt32 Unknown008 = Buffer.ReadUInt32();
                UInt32 NumBinds = Buffer.ReadUInt32();

                OutputStream.WriteLine("Unknown000: {0} Time: {1} Unknown008: {2} Num Binds: {3}\r\n", Unknown000, TimeRemaining, Unknown008, NumBinds);

                for (int i = 0; i < NumBinds; ++i)
                {
                    UInt32 BindNumber = Buffer.ReadUInt32();
                    UInt32 ZoneID = Buffer.ReadUInt32();
                    float X = Buffer.ReadSingle();
                    float Y = Buffer.ReadSingle();
                    float Z = Buffer.ReadSingle();
                    float Heading = Buffer.ReadSingle();
                    string ZoneName = Buffer.ReadString(false);
                    byte Valid = Buffer.ReadByte();

                    OutputStream.WriteLine("Bind Number: {0} Zone ID: {1} Zone Name: {2} Valid: {3}", BindNumber, ZoneID, ZoneName, Valid);
                }
            }

            OutputStream.WriteLine("");
        }

        public void ExploreZonePlayerToBind(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt32 ZoneID = Buffer.ReadUInt32();

            float X = Buffer.ReadSingle();
            float Y = Buffer.ReadSingle();
            float Z = Buffer.ReadSingle();
            float Heading = Buffer.ReadSingle();
            string ZoneName = Buffer.ReadString(false);

            byte Unknown021 = Buffer.ReadByte();
            UInt32 Unknown022 = Buffer.ReadUInt32();
            UInt32 Unknown023 = Buffer.ReadUInt32();
            UInt32 Unknown024 = Buffer.ReadUInt32();

            OutputStream.WriteLine("ZoneID: {0} Loc: ({1}, {2}, {3}) ZoneName: {4}", ZoneID, X, Y, Z, ZoneName);
            OutputStream.WriteLine("Unknowns: {0} {1} {2} {3}", Unknown021, Unknown022, Unknown023, Unknown024);


            OutputStream.WriteLine("");
        }

        public void ExploreRequestClientZoneChange(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt16 ZoneID = Buffer.ReadUInt16();
            UInt16 Instance = Buffer.ReadUInt16();
            float X = Buffer.ReadSingle();
            float Y = Buffer.ReadSingle();
            float Z = Buffer.ReadSingle();
            float Heading = Buffer.ReadSingle();
            UInt32 Type = Buffer.ReadUInt16();

            OutputStream.WriteLine("ZoneID: {0} Type: {1}", ZoneID, Type);
            OutputStream.WriteLine("");
        }

        public void ExploreDeleteSpawn(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt32 SpawnID = Buffer.ReadUInt32();
            byte Decay = 0; // Buffer.ReadByte();

            string SpawnName = FindExplorerSpawn(SpawnID);

            OutputStream.WriteLine("SpawnID: {0} {1} Decay: {2}\r\n", SpawnID, SpawnName, Decay);
        }

        public void ExploreCharInventoryPacket(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt32 ItemCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("There are {0} items in the inventory.\r\n", ItemCount);

            for (int i = 0; i < ItemCount; ++i)
            {
                ExploreSubItem(OutputStream, ref Buffer);
            }

            OutputStream.WriteLine("");
        }

        public void ExploreItemPacket(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            Buffer.SkipBytes(4);    // Skip type field.

            ExploreSubItem(OutputStream, ref Buffer);

            OutputStream.WriteLine("");
        }

        void ExploreSubItem(StreamWriter OutputStream, ref ByteStream Buffer)
        {
            UInt32 StackSize = Buffer.ReadUInt32();
            Buffer.SkipBytes(4);
            UInt32 Slot = Buffer.ReadUInt32();
            UInt32 Price = Buffer.ReadUInt32();
            UInt32 MerchantSlot = Buffer.ReadUInt32();
            Buffer.SkipBytes(16);
            Buffer.SkipBytes(28);
            string Name = Buffer.ReadString(true);
            Buffer.ReadString(true);    // Lore
            Buffer.ReadString(true);    // IDFile

            OutputStream.WriteLine("Item Name: {0}", Name);

            Buffer.SkipBytes(236);      // ItemBodyStruct
            Buffer.ReadString(true);    // CharmFile
            Buffer.SkipBytes(64);       // ItemSecondaryBodyStruct
            Buffer.ReadString(true);    // Filename

            Buffer.SkipBytes(76);       // ItemTertiaryBodyStruct

            //Buffer.SkipBytes(30);       // Click Effect Struct
            UInt32 Effect = Buffer.ReadUInt32();
            byte Level2 = Buffer.ReadByte();
            UInt32 Type = Buffer.ReadUInt32();
            byte Level = Buffer.ReadByte();
            UInt32 Unknown1 = Buffer.ReadUInt32();
            UInt32 Unknown2 = Buffer.ReadUInt32();
            UInt32 Unknown3 = Buffer.ReadUInt32();
            UInt32 Unknown4 = Buffer.ReadUInt32();
            UInt32 Unknown5 = Buffer.ReadUInt32();

            OutputStream.WriteLine("Buffer pos is {0}" + Buffer.GetPosition());
            string ClickName = Buffer.ReadString(true);    // Clickname
            OutputStream.WriteLine(" Click Name: {0}", ClickName);
            //Buffer.SkipBytes(4);        // Clickunk7
            UInt32 Unknown7 = Buffer.ReadUInt32();
            OutputStream.WriteLine("    Effect: {0} Level2: {1} Type {2} Level {3}", Effect, Level2, Type, Level);
            OutputStream.WriteLine("    Unks: {0} {1} {2} {3} {4} {5}", Unknown1, Unknown2, Unknown3, Unknown4, Unknown5, Unknown7);

            Buffer.SkipBytes(30);       // Proc Effect Struct
            Buffer.ReadString(true);    // Clickname
            Buffer.SkipBytes(4);        // Unknown5

            Buffer.SkipBytes(30);       // Worn Effect Struct
            Buffer.ReadString(true);    // Wornname
            Buffer.SkipBytes(4);        // Unknown6

            //Buffer.SkipBytes(30);       // Worn Effect Struct
            Effect = Buffer.ReadUInt32();
            Level2 = Buffer.ReadByte();
            Type = Buffer.ReadUInt32();
            Level = Buffer.ReadByte();
            Unknown1 = Buffer.ReadUInt32();
            Unknown2 = Buffer.ReadUInt32();
            Unknown3 = Buffer.ReadUInt32();
            Unknown4 = Buffer.ReadUInt32();
            Unknown5 = Buffer.ReadUInt32();
            string FocusName = Buffer.ReadString(true);    // Focusname
            OutputStream.WriteLine("   Focusname is {0}", FocusName);
            UInt32 Unknown6 = Buffer.ReadUInt32();
            OutputStream.WriteLine("    Effect: {0} Level2: {1} Type {2} Level {3}", Effect, Level2, Type, Level);
            OutputStream.WriteLine("    Unks: {0} {1} {2} {3} {4} {5}", Unknown1, Unknown2, Unknown3, Unknown4, Unknown5, Unknown6);
            //Buffer.SkipBytes(4);        // Unknown6

            Buffer.SkipBytes(30);       // Scroll Effect Struct
            Buffer.ReadString(true);    // Scrollname
            Buffer.SkipBytes(4);        // Unknown6

            Buffer.SkipBytes(30);       // Bard Effect Struct
            Buffer.ReadString(true);    // Wornname
            Buffer.SkipBytes(4);        // Unknown6

            Buffer.SkipBytes(103);      // Quaternarybodystruct - 4

            UInt32 SubLengths = Buffer.ReadUInt32();

            //return;

            for (int i = 0; i < SubLengths; ++i)
            {
                Buffer.SkipBytes(4);
                ExploreSubItem(OutputStream, ref Buffer);
            }

            return;

            //Buffer.SkipBytes(236);  // Item Body Struct

            UInt32 ID = Buffer.ReadUInt32();
            byte Weight = Buffer.ReadByte();
            byte NoRent = Buffer.ReadByte();
            byte NoDrop = Buffer.ReadByte();
            byte Attune = Buffer.ReadByte();
            byte Size = Buffer.ReadByte();

            OutputStream.WriteLine("   ID: {0} Weight: {1} NoRent: {2} NoDrop: {3} Attune {4} Size {5}", ID, Weight, NoRent, NoDrop, Attune, Size);

            UInt32 Slots = Buffer.ReadUInt32();
            //UInt32 Price = Buffer.ReadUInt32();
            UInt32 Icon = Buffer.ReadUInt32();
            Buffer.SkipBytes(2);
            UInt32 BenefitFlags = Buffer.ReadUInt32();
            byte Tradeskills = Buffer.ReadByte();

            OutputStream.WriteLine("   Slots: {0} Price: {1} Icon: {2} BenefitFlags {3} Tradeskills: {4}", Slots, Price, Icon, BenefitFlags, Tradeskills);

            byte CR = Buffer.ReadByte();
            byte DR = Buffer.ReadByte();
            byte PR = Buffer.ReadByte();
            byte MR = Buffer.ReadByte();
            byte FR = Buffer.ReadByte();
            byte SVC = Buffer.ReadByte();

            OutputStream.WriteLine("   CR: {0} DR: {1} PR: {2} MR: {3} FR: {4} SVC: {5}", CR, DR, PR, MR, FR, SVC);

            byte AStr = Buffer.ReadByte();
            byte ASta = Buffer.ReadByte();
            byte AAgi = Buffer.ReadByte();
            byte ADex = Buffer.ReadByte();
            byte ACha = Buffer.ReadByte();
            byte AInt = Buffer.ReadByte();
            byte AWis = Buffer.ReadByte();

            OutputStream.WriteLine("   AStr: {0} ASta: {1} AAgi: {2} ADex: {3} ACha: {4} AInt: {5} AWis: {6}", AStr, ASta, AAgi, ADex, ACha, AInt, AWis);

            Int32 HP = Buffer.ReadInt32();
            Int32 Mana = Buffer.ReadInt32();
            UInt32 Endurance = Buffer.ReadUInt32();
            Int32 AC = Buffer.ReadInt32();
            Int32 Regen = Buffer.ReadInt32();
            Int32 ManaRegen = Buffer.ReadInt32();
            Int32 EndRegen = Buffer.ReadInt32();
            UInt32 Classes = Buffer.ReadUInt32();
            UInt32 Races = Buffer.ReadUInt32();
            UInt32 Deity = Buffer.ReadUInt32();
            Int32 SkillModValue = Buffer.ReadInt32();
            Buffer.SkipBytes(4);
            UInt32 SkillModType = Buffer.ReadUInt32();
            UInt32 BaneDamageRace = Buffer.ReadUInt32();
            UInt32 BaneDamageBody = Buffer.ReadUInt32();
            UInt32 BaneDamageRaceAmount = Buffer.ReadUInt32();
            Int32 BaneDamageAmount = Buffer.ReadInt32();
            byte Magic = Buffer.ReadByte();
            Int32 CastTime = Buffer.ReadInt32();
            UInt32 ReqLevel = Buffer.ReadUInt32();
            UInt32 RecLevel = Buffer.ReadUInt32();
            UInt32 ReqSkill = Buffer.ReadUInt32();
            UInt32 BardType = Buffer.ReadUInt32();
            Int32 BardValue = Buffer.ReadInt32();
            byte Light = Buffer.ReadByte();
            byte Delay = Buffer.ReadByte();
            byte ElemDamageAmount = Buffer.ReadByte();
            byte ElemDamageType = Buffer.ReadByte();
            byte Range = Buffer.ReadByte();
            UInt32 Damage = Buffer.ReadUInt32();
            UInt32 Color = Buffer.ReadUInt32();
            byte ItemType = Buffer.ReadByte();
            UInt32 Material = Buffer.ReadUInt32();
            Buffer.SkipBytes(4);
            UInt32 EliteMaterial = Buffer.ReadUInt32();
            float SellRate = Buffer.ReadSingle();
            Int32 CombatEffects = Buffer.ReadInt32();
            Int32 Shielding = Buffer.ReadInt32();
            Int32 StunResist = Buffer.ReadInt32();
            Int32 StrikeThrough = Buffer.ReadInt32();
            Int32 ExtraDamageSkill = Buffer.ReadInt32();
            Int32 ExtraDamageAmount = Buffer.ReadInt32();
            Int32 SpellShield = Buffer.ReadInt32();
            Int32 Avoidance = Buffer.ReadInt32();
            Int32 Accuracy = Buffer.ReadInt32();
            UInt32 CharmFileID = Buffer.ReadUInt32();
            UInt32 FactionMod1 = Buffer.ReadUInt32();
            Int32 FactionAmount1 = Buffer.ReadInt32();
            UInt32 FactionMod2 = Buffer.ReadUInt32();
            Int32 FactionAmount2 = Buffer.ReadInt32();
            UInt32 FactionMod3 = Buffer.ReadUInt32();
            Int32 FactionAmount3 = Buffer.ReadInt32();
            UInt32 FactionMod4 = Buffer.ReadUInt32();
            Int32 FactionAmount4 = Buffer.ReadInt32();

            Buffer.ReadString(true);    // Charm File
            Buffer.SkipBytes(64);   // Item Secondary Body Struct
            Buffer.ReadString(true);    // Filename
            Buffer.SkipBytes(76);   // Item Tertiary Body Struct
            Buffer.SkipBytes(30);   // Click Effect Struct
            Buffer.ReadString(true);    // Clickname
            Buffer.SkipBytes(4);    // clickunk7
            Buffer.SkipBytes(30);   // Proc Effect Struct
            Buffer.ReadString(true);    // Proc Name
            Buffer.SkipBytes(4);    // unknown5
            Buffer.SkipBytes(30);   // Worn Effect Struct
            Buffer.ReadString(true);    // Worn Name
            Buffer.SkipBytes(4);    // unknown6
            Buffer.SkipBytes(30);   // Worn Effect Struct
            Buffer.ReadString(true);    // Worn Name
            Buffer.SkipBytes(4);    // unknown6
            Buffer.SkipBytes(30);   // Worn Effect Struct
            Buffer.ReadString(true);    // Worn Name
            Buffer.SkipBytes(4);    // unknown6
            Buffer.SkipBytes(30);   // Worn Effect Struct
            Buffer.ReadString(true);    // Worn Name
            Buffer.SkipBytes(4);    // unknown6
            Buffer.SkipBytes(103);   // Item Quaternary Body Struct - 4 (we want to read the SubLength field at the end)

            //UInt32 SubLengths = Buffer.ReadUInt32();

            for (int i = 0; i < SubLengths; ++i)
            {
                Buffer.SkipBytes(4);
                ExploreSubItem(OutputStream, ref Buffer);
            }
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
