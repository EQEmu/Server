using System;
using System.IO;
using System.Diagnostics;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchDecember102012Decoder : PatchAugust152012Decoder
    {
        public PatchDecember102012Decoder()
        {
            Version = "EQ Client Build Date December 10 2012.";

            ExpectedPPLength = -1;

            PPZoneIDOffset = -1;

            PatchConfFileName = "patch_Dec10-2012.conf";

            PacketsToMatch = new PacketToMatch[] {
                new PacketToMatch { OPCodeName = "OP_AckPacket", Direction = PacketDirection.ClientToServer, RequiredSize = 4, VersionMatched = false },
                new PacketToMatch { OPCodeName = "OP_ZoneEntry", Direction = PacketDirection.ClientToServer, RequiredSize = 76, VersionMatched = false },
                new PacketToMatch { OPCodeName = "OP_PlayerProfile", Direction = PacketDirection.ServerToClient, RequiredSize = -1, VersionMatched = true },
            };

            WaitingForPacket = 0;
        }

        override public IdentificationStatus Identify(int OpCode, int Size, PacketDirection Direction)
        {
            if ((OpCode == OpManager.OpCodeNameToNumber(PacketsToMatch[WaitingForPacket].OPCodeName)) &&
                (Direction == PacketsToMatch[WaitingForPacket].Direction))
            {
                if((PacketsToMatch[WaitingForPacket].RequiredSize >= 0) && (Size != PacketsToMatch[WaitingForPacket].RequiredSize))
                    return IdentificationStatus.No;

                if(PacketsToMatch[WaitingForPacket].VersionMatched)
                    return IdentificationStatus.Yes;

                WaitingForPacket++;

                return IdentificationStatus.Tentative;
            }

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
                // We should really verify the variable length PP here ...

                return -1;
            }

            return ExpectedPPLength;
        }

        override public PositionUpdate Decode_OP_MobUpdate(byte[] MobUpdatePacket)
        {
            PositionUpdate PosUpdate = new PositionUpdate();

            ByteStream Buffer = new ByteStream(MobUpdatePacket);

            PosUpdate.SpawnID = Buffer.ReadUInt16();

            Buffer.SkipBytes(2);

            UInt32 Word1 = Buffer.ReadUInt32();

            UInt32 Word2 = Buffer.ReadUInt32();

            UInt16 Word3 = Buffer.ReadUInt16();

            PosUpdate.p.y = Utils.EQ19ToFloat((Int32)(Word1 & 0x7FFFF));

            // Z is in the top 13 bits of Word1 and the bottom 6 of Word2

            UInt32 ZPart1 = Word1 >> 19;    // ZPart1 now has low order bits of Z in bottom 13 bits
            UInt32 ZPart2 = Word2 & 0x3F;   // ZPart2 now has high order bits of Z in bottom 6 bits

            ZPart2 = ZPart2 << 13;

            PosUpdate.p.z = Utils.EQ19ToFloat((Int32)(ZPart1 | ZPart2));

            PosUpdate.p.x = Utils.EQ19ToFloat((Int32)(Word2 >> 13) & 0x7FFFF);

            PosUpdate.p.heading = Utils.EQ19ToFloat((Int32)((Word3 >> 4) & 0xFFF));

            PosUpdate.HighRes = false;

            return PosUpdate;
        }

        override public UInt16 GetZoneNumber()
        {

            // A return value of zero from this method should be intepreted as 'Unable to identify patch version'.

            // Thanks to ShowEQ team for details on how to parse the variable length PP

            List<byte[]> PlayerProfilePacket = GetPacketsOfType("OP_PlayerProfile", PacketDirection.ServerToClient);

            if (PlayerProfilePacket.Count == 0)
            {
                return 0;
            }

            ByteStream Buffer = new ByteStream(PlayerProfilePacket[0]);

            Buffer.SkipBytes(24);

            UInt32 BindCount = Buffer.ReadUInt32();

            for (int i = 0; i < BindCount; ++i)
            {
                Buffer.SkipBytes(20);   // sizeof(Bind Struct)
            }
            Buffer.SkipBytes(8); // Deity, intoxication

            UInt32 SpellRefreshCount = Buffer.ReadUInt32();

            for (int i = 0; i < SpellRefreshCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            UInt32 EquipmentCount = Buffer.ReadUInt32();

            for (int i = 0; i < EquipmentCount; ++i)
            {
                Buffer.SkipBytes(20);
            }

            UInt32 SomethingCount = Buffer.ReadUInt32();

            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(20);
            }

            SomethingCount = Buffer.ReadUInt32();

            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            SomethingCount = Buffer.ReadUInt32();

            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            Buffer.SkipBytes(52);   // Per SEQ, this looks like face, haircolor, beardcolor etc.

            UInt32 Points = Buffer.ReadUInt32();
            UInt32 Mana = Buffer.ReadUInt32();
            UInt32 CurHP = Buffer.ReadUInt32();

            Buffer.SkipBytes(28);
            Buffer.SkipBytes(28);

            UInt32 AACount = Buffer.ReadUInt32();

            for (int i = 0; i < AACount; ++i)
            {
                Buffer.SkipBytes(12);
            }

            SomethingCount = Buffer.ReadUInt32();

            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(4);
            }
            SomethingCount = Buffer.ReadUInt32();

            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(4);
            }
            SomethingCount = Buffer.ReadUInt32();

            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            SomethingCount = Buffer.ReadUInt32();

            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            SomethingCount = Buffer.ReadUInt32();

            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            SomethingCount = Buffer.ReadUInt32();

            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            UInt32 SpellBookSlots = Buffer.ReadUInt32();

            for (int i = 0; i < SpellBookSlots; ++i)
            {
                Buffer.SkipBytes(4);
            }

            UInt32 SpellMemSlots = Buffer.ReadUInt32();

            for (int i = 0; i < SpellMemSlots; ++i)
            {
                Buffer.SkipBytes(4);
            }

            SomethingCount = Buffer.ReadUInt32();

            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            Buffer.SkipBytes(1);

            UInt32 BuffCount = Buffer.ReadUInt32();

            for (int i = 0; i < BuffCount; ++i)
            {
                Buffer.SkipBytes(80);
            }

            UInt32 Plat = Buffer.ReadUInt32();
            UInt32 Gold = Buffer.ReadUInt32();
            UInt32 Silver = Buffer.ReadUInt32();
            UInt32 Copper = Buffer.ReadUInt32();

            Buffer.SkipBytes(16); // Money on cursor

            Buffer.SkipBytes(20);

            UInt32 AASpent = Buffer.ReadUInt32();

            Buffer.SkipBytes(30);

            UInt32 BandolierCount = Buffer.ReadUInt32();

            for (int i = 0; i < BandolierCount; ++i)
            {
                Buffer.ReadString(false);

                Buffer.ReadString(false);
                Buffer.SkipBytes(8);

                Buffer.ReadString(false);
                Buffer.SkipBytes(8);

                Buffer.ReadString(false);
                Buffer.SkipBytes(8);

                Buffer.ReadString(false);
                Buffer.SkipBytes(8);
            }

            UInt32 PotionCount = Buffer.ReadUInt32();

            for (int i = 0; i < PotionCount; ++i)
            {
                Buffer.ReadString(false);
                Buffer.SkipBytes(8);
            }

            Buffer.SkipBytes(100);

            int CurrentPosition = Buffer.GetPosition();

            String PlayerName = Buffer.ReadString(false);

            Buffer.SetPosition(CurrentPosition + 64);

            Buffer.SkipBytes(96);

            // This is what I am after ...

            UInt16 ZoneID = Buffer.ReadUInt16();

            return ZoneID;

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

                NewSpawn.Gender = (Bitfield  & 3);

                Byte OtherData = Buffer.ReadByte();

                Buffer.SkipBytes(8);    // Skip 8 unknown bytes

                NewSpawn.DestructableString1 = "";
                NewSpawn.DestructableString2 = "";
                NewSpawn.DestructableString3 = "";

                if ((NewSpawn.IsNPC > 0) && ((OtherData & 1) > 0))
                {
                    // Destructable Objects
                    NewSpawn.DestructableString1 = Buffer.ReadString(false);
                    NewSpawn.DestructableString2 = Buffer.ReadString(false);
                    NewSpawn.DestructableString3 = Buffer.ReadString(false);
                    Buffer.SkipBytes(53);
                }

                if ((OtherData & 4) > 0)
                {
                    // Auras
                    Buffer.ReadString(false);
                    Buffer.ReadString(false);
                    Buffer.SkipBytes(54);
                }

                NewSpawn.PropCount = Buffer.ReadByte();

                if (NewSpawn.PropCount > 0)
                    NewSpawn.BodyType = Buffer.ReadUInt32();
                else
                    NewSpawn.BodyType = 0;


                for (int j = 1; j < NewSpawn.PropCount; ++j)
                        Buffer.SkipBytes(4);

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

                NewSpawn.EquipChest2 = Buffer.ReadByte();

                bool UseWorn = (NewSpawn.EquipChest2 == 255);

                Buffer.SkipBytes(2);    // 2 Unknown bytes;

                NewSpawn.Helm = Buffer.ReadByte();

                NewSpawn.Size = Buffer.ReadSingle();

                NewSpawn.Face = Buffer.ReadByte();

                NewSpawn.WalkSpeed = Buffer.ReadSingle();

                NewSpawn.RunSpeed = Buffer.ReadSingle();

                NewSpawn.Race = Buffer.ReadUInt32();

                Buffer.SkipBytes(1);   // Skip Holding

                NewSpawn.Deity = Buffer.ReadUInt32();

                Buffer.SkipBytes(8);    // Skip GuildID and GuildRank

                NewSpawn.Class = Buffer.ReadByte();

                Buffer.SkipBytes(4);     // Skip PVP, Standstate, Light, Flymode

                NewSpawn.LastName = Buffer.ReadString(true);

                Buffer.SkipBytes(6);

                NewSpawn.PetOwnerID = Buffer.ReadUInt32();

                Buffer.SkipBytes(25);

                NewSpawn.MeleeTexture1 = 0;
                NewSpawn.MeleeTexture2 = 0;

                if ( (NewSpawn.IsNPC == 0) || NPCType.IsPlayableRace(NewSpawn.Race))
                {
                    for (int ColourSlot = 0; ColourSlot < 9; ++ColourSlot)
                        NewSpawn.SlotColour[ColourSlot] = Buffer.ReadUInt32();

                    for (int i = 0; i < 9; ++i)
                    {
                        NewSpawn.Equipment[i] = Buffer.ReadUInt32();

                        UInt32 Equip3 = Buffer.ReadUInt32();

                        UInt32 Equip2 = Buffer.ReadUInt32();

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

                    Buffer.SkipBytes(20);

                    NewSpawn.MeleeTexture1 = Buffer.ReadUInt32();
                    Buffer.SkipBytes(16);
                    NewSpawn.MeleeTexture2 = Buffer.ReadUInt32();
                    Buffer.SkipBytes(16);
                }

                if (NewSpawn.EquipChest2 == 255)
                    NewSpawn.EquipChest2 = 0;

                if (NewSpawn.Helm == 255)
                    NewSpawn.Helm = 0;

                UInt32 Position1 = Buffer.ReadUInt32();

                UInt32 Position2 = Buffer.ReadUInt32();

                UInt32 Position3 = Buffer.ReadUInt32();

                UInt32 Position4 = Buffer.ReadUInt32();

                UInt32 Position5 = Buffer.ReadUInt32();

                NewSpawn.YPos = Utils.EQ19ToFloat((Int32)(Position1 >> 12));

                NewSpawn.ZPos = Utils.EQ19ToFloat((Int32)(Position3 >> 13) & 0x7FFFF);

                NewSpawn.XPos = Utils.EQ19ToFloat((Int32)(Position4) & 0x7FFFF);

                NewSpawn.Heading = Utils.EQ19ToFloat((Int32)(Position5) & 0x7FFFF);




                if ((OtherData & 16) > 0)
                {
                    NewSpawn.Title = Buffer.ReadString(false);
                }

                if ((OtherData & 32) > 0)
                {
                    NewSpawn.Suffix = Buffer.ReadString(false);
                }

                // unknowns
                Buffer.SkipBytes(8);

                NewSpawn.IsMercenary = Buffer.ReadByte();

                Buffer.SkipBytes(54);

                Debug.Assert(Buffer.GetPosition() == Buffer.Length(), "Length mismatch while parsing zone spawns");

                ZoneSpawns.Add(NewSpawn);
            }

            return ZoneSpawns;
        }


        public override void RegisterExplorers()
        {
            //OpManager.RegisterExplorer("OP_PlayerProfile", ExplorePlayerProfile);
            //OpManager.RegisterExplorer("OP_ZoneEntry", ExploreZoneEntry);
            //OpManager.RegisterExplorer("OP_RequestClientZoneChange", ExploreRequestClientZoneChange);
            //OpManager.RegisterExplorer("OP_NPCMoveUpdate", ExploreNPCMoveUpdate);
            //OpManager.RegisterExplorer("OP_MobUpdate", ExploreMobUpdate);
            //OpManager.RegisterExplorer("OP_ClientUpdate", ExploreClientUpdate);
            //OpManager.RegisterExplorer("OP_OpenNewTasksWindow", ExploreOpenNewTasksWindow);
            //OpManager.RegisterExplorer("OP_TaskDescription", ExploreTaskDescription);
            //OpManager.RegisterExplorer("OP_CharInventory", ExploreInventory);
        }

        public void DecodeItemPacket(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            String UnkString = Buffer.ReadString(false);
            //Buffer.SkipBytes(88);
            Buffer.SkipBytes(35);
            UInt32 RecastTimer = Buffer.ReadUInt32();
            Buffer.SkipBytes(49);
            String ItemName = Buffer.ReadString(false);
            String ItemLore = Buffer.ReadString(false);
            String ItemIDFile = Buffer.ReadString(false);
            Buffer.ReadString(false);

            UInt32 ItemID = Buffer.ReadUInt32();
            OutputStream.WriteLine("ItemName: {0}, IDFile: {1}", ItemName, ItemIDFile);
            OutputStream.WriteLine("Recast Time: {0:X}", RecastTimer);

            Buffer.SkipBytes(251);

            String CharmFile = Buffer.ReadString(false);

            OutputStream.WriteLine("CharmFile: {0}", CharmFile);

            Buffer.SkipBytes(74);   // Secondary BS

            String FileName = Buffer.ReadString(false);
            OutputStream.WriteLine("FileName: {0}", CharmFile);

            Buffer.SkipBytes(76);   // Tertiary BS

            UInt32 ClickEffect = Buffer.ReadUInt32();
            Buffer.SkipBytes(26);   // ClickEffect Struct
            String ClickName = Buffer.ReadString(false);
            OutputStream.WriteLine("ClickEffect = {0}, ClickName = {1}", ClickEffect, ClickName);
            Buffer.ReadUInt32();

            ClickEffect = Buffer.ReadUInt32();
            Buffer.SkipBytes(26);   // ClickEffect Struct
            ClickName = Buffer.ReadString(false);
            OutputStream.WriteLine("ClickEffect = {0}, ClickName = {1}", ClickEffect, ClickName);
            Buffer.ReadUInt32();

            ClickEffect = Buffer.ReadUInt32();
            Buffer.SkipBytes(26);   // ClickEffect Struct
            ClickName = Buffer.ReadString(false);
            OutputStream.WriteLine("ClickEffect = {0}, ClickName = {1}", ClickEffect, ClickName);
            Buffer.ReadUInt32();

            ClickEffect = Buffer.ReadUInt32();
            Buffer.SkipBytes(26);   // ClickEffect Struct
            ClickName = Buffer.ReadString(false);
            OutputStream.WriteLine("ClickEffect = {0}, ClickName = {1}", ClickEffect, ClickName);
            Buffer.ReadUInt32();

            ClickEffect = Buffer.ReadUInt32();
            Buffer.SkipBytes(26);   // ClickEffect Struct
            ClickName = Buffer.ReadString(false);
            OutputStream.WriteLine("ClickEffect = {0}, ClickName = {1}", ClickEffect, ClickName);
            Buffer.ReadUInt32();

            ClickEffect = Buffer.ReadUInt32();
            Buffer.SkipBytes(26);   // ClickEffect Struct
            ClickName = Buffer.ReadString(false);
            OutputStream.WriteLine("ClickEffect = {0}, ClickName = {1}", ClickEffect, ClickName);
            Buffer.ReadUInt32();

            //Buffer.SkipBytes(167);
            Buffer.SkipBytes(125);
            //Byte UnkByte = Buffer.ReadByte();
            //OutputStream.WriteLine("Unk byte is {0:X}", UnkByte);
            OutputStream.WriteLine("At String ? Pos is {0}", Buffer.GetPosition());
            UnkString = Buffer.ReadString(false);
            OutputStream.WriteLine("Unk String is {0}", UnkString);
            Buffer.SkipBytes(41);
            UInt32 SubItemCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("Buffer Pos: {0}, SubItemCount = {1}", Buffer.GetPosition(), SubItemCount);

            for (int j = 0; j < SubItemCount; ++j)
            {
                Buffer.ReadUInt32();
                DecodeItemPacket(OutputStream, Buffer, Direction);
            }


        }


        public void ExploreInventory(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            if (Buffer.Length() < 4)
                return;

            UInt32 Count = Buffer.ReadUInt32();

            for (int i = 0; i < Count; ++i)
            {
                try
                {
                    DecodeItemPacket(OutputStream, Buffer, Direction);
                }
                catch
                {
                    return;
                }
            }
        }

        public void ExploreTaskDescription(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt32 Seq = Buffer.ReadUInt32();
            UInt32 TaskID = Buffer.ReadUInt32();
            UInt32 Unk1 = Buffer.ReadUInt32();
            UInt32 Unk2 = Buffer.ReadUInt32();
            byte Unk3 = Buffer.ReadByte();

            OutputStream.WriteLine("Seq: {0}, TaskID: {1}, Unk1: {2:X}, Unk2: {3:X}, Unk3: {4:X}", Seq, TaskID, Unk1, Unk2, Unk3);

            String Title = Buffer.ReadString(false);
            OutputStream.WriteLine("Title: {0}", Title);

            UInt32 Duration = Buffer.ReadUInt32();
            UInt32 Unk4 = Buffer.ReadUInt32();
            UInt32 StartTime = Buffer.ReadUInt32();

            String Desc = Buffer.ReadString(false);


            OutputStream.WriteLine("Duration: {0}, Unk4: {1:X}, StartTime: {2:X}", Duration, Unk4, StartTime);

            OutputStream.WriteLine("Desc: {0}", Desc);

            UInt32 RewardCount = Buffer.ReadUInt32();
            UInt32 Unk5 = Buffer.ReadUInt32();
            UInt32 Unk6 = Buffer.ReadUInt32();
            UInt16 Unk7 = Buffer.ReadUInt16();

            OutputStream.WriteLine("RewardCount: {0}, Unk5: {1:X}, Unk6: {2:X}, Unk7: {3:X}", Duration, Unk5, Unk6, Unk7);

            string MyString = "";

            byte b;

            while ((b = Buffer.ReadByte()) != 0)
            {
                if (b == 0x12)
                    continue;

                MyString += Convert.ToChar(b);
            }

            OutputStream.WriteLine("RewardString: {0}", MyString);

            UInt32 Unk8 = Buffer.ReadUInt32();
            byte Unk9 = Buffer.ReadByte();

            OutputStream.WriteLine("Unk8: {0:X}, Unk9: {1:X}", Unk8, Unk9);

        }

        public void ExploreOpenNewTasksWindow(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt32 NumTasks = Buffer.ReadUInt32();
            UInt32 Unknown = Buffer.ReadUInt32();
            UInt32 TaskGiver = Buffer.ReadUInt32();

            OutputStream.WriteLine("Number of Tasks: {0}, Given by: {1}", NumTasks, TaskGiver);

            for (int i = 0; i < NumTasks; ++i)
            {
                UInt32 TaskID = Buffer.ReadUInt32();
                float Unk1 = Buffer.ReadSingle();
                UInt32 TimeLimit = Buffer.ReadUInt32();
                UInt32 Unk2 = Buffer.ReadUInt32();

                string Title = Buffer.ReadString(false);
                string Description = Buffer.ReadString(false);
                string UnkString = Buffer.ReadString(false);

                OutputStream.WriteLine("TaskID: {0}, Title: {1}", TaskID, Title);

                UInt32 ActivityCount = Buffer.ReadUInt32();
                OutputStream.WriteLine("Unknown: {0}", ActivityCount);

                OutputStream.WriteLine("");
                for (int j = 0; j < ActivityCount; ++j)
                {
                    OutputStream.WriteLine("Activity {0}", i);
                    OutputStream.WriteLine("");
                    OutputStream.WriteLine("    Unknown: {0}", Buffer.ReadUInt32());
                    OutputStream.WriteLine("    Unknown: {0}", Buffer.ReadUInt32());
                    OutputStream.WriteLine("    Unknown: {0}", Buffer.ReadUInt32());

                    OutputStream.WriteLine("    String: {0}", Buffer.ReadString(false));

                    UInt32 StringLength = Buffer.ReadUInt32();
                    OutputStream.WriteLine("    StringLength: {0}", StringLength);

                    string MyString = "";

                    for (int k = 0; k < StringLength; ++k)
                        MyString += Convert.ToChar(Buffer.ReadByte());

                    OutputStream.WriteLine("    Weird String: {0}", MyString);


                    OutputStream.WriteLine("    Unknown: {0}", Buffer.ReadUInt32());
                    OutputStream.WriteLine("    Unknown: {0}", Buffer.ReadUInt32());

                    OutputStream.WriteLine("    String: {0}", Buffer.ReadString(false));
                    OutputStream.WriteLine("    Unknown 2 bytes: {0}", Buffer.ReadUInt16());

                    //if (i == 3)
                    //{
                    //    OutputStream.WriteLine("Offset is now: {0}", Buffer.GetPosition());
                    //    return;
                    //}
                    OutputStream.WriteLine("    String: {0}", Buffer.ReadString(false));
                    OutputStream.WriteLine("    String: {0}", Buffer.ReadString(false));
                    OutputStream.WriteLine("    String: {0}", Buffer.ReadString(false));
                }
                OutputStream.WriteLine("");
                //OutputStream.WriteLine("Offset is now: {0}", Buffer.GetPosition());

            }
        }


        public void ExploreRequestClientZoneChange(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt32 ZoneID = Buffer.ReadUInt32();
            UInt32 Unknown = Buffer.ReadUInt32();
            float y = Buffer.ReadSingle();
            float x = Buffer.ReadSingle();
            float z = Buffer.ReadSingle();
            float heading = Buffer.ReadSingle();
            Buffer.SkipBytes(148);
            float uf = Buffer.ReadSingle();
            OutputStream.WriteLine("UF = {0}", uf);

        }

        public void ExploreClientUpdate(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            if (Direction == PacketDirection.ServerToClient)
            {
                UInt32 SpawnID = Buffer.ReadUInt16();
                UInt32 SpawnID2 = Buffer.ReadUInt16();
                OutputStream.WriteLine("ClientUpdate S->C SpawnID: {0}, SpawnID2: {1}", SpawnID, SpawnID2);

                UInt32 Word = Buffer.ReadUInt32();
                float Y = Utils.EQ19ToFloat((int)(Word >> 12));
                OutputStream.WriteLine("Y = {0}", Y);

                //Buffer.SkipBytes(6);
                //float DeltaY = Buffer.ReadSingle();
                //float YPos = Buffer.ReadSingle();
                //float XPos = Buffer.ReadSingle();
                //float DeltaHeading = Utils.EQ19ToFloat((int)(Word & 0x3FF));
            }




        }

        public void ExploreNPCMoveUpdate(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            PositionUpdate PosUpdate;

            PosUpdate = Decode_OP_NPCMoveUpdate(Buffer.Buffer);

            OutputStream.WriteLine("SpawnID: {0}, X = {1}, Y = {2}, Z = {3}, Heading = {4}", PosUpdate.SpawnID, PosUpdate.p.x, PosUpdate.p.y, PosUpdate.p.z, PosUpdate.p.heading);
        }

        public void ExploreMobUpdate(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            PositionUpdate PosUpdate;

            PosUpdate = Decode_OP_MobUpdate(Buffer.Buffer);

            OutputStream.WriteLine("SpawnID: {0}, X = {1}, Y = {2}, Z = {3}, Heading = {4}", PosUpdate.SpawnID, PosUpdate.p.x, PosUpdate.p.y, PosUpdate.p.z, PosUpdate.p.heading);
        }

        public void ExplorePlayerProfile(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            OutputStream.WriteLine("{0, -5}: Checksum = {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: ChecksumSize = {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown = {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown = {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("");
            OutputStream.WriteLine("{0, -5}: Gender = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Race = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Class = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Level = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Level1 = {1}", Buffer.GetPosition(), Buffer.ReadByte());

            OutputStream.WriteLine("");
            UInt32 BindCount = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: BindCount = {1}", Buffer.GetPosition() - 4, BindCount);

            for (int i = 0; i < BindCount; ++i)
            {
                OutputStream.WriteLine("{0, -5}:   Bind: {1} Zone: {2} XYZ: {3},{4},{5} Heading: {6}",
                    Buffer.GetPosition(), i, Buffer.ReadUInt32(), Buffer.ReadSingle(), Buffer.ReadSingle(),Buffer.ReadSingle(),Buffer.ReadSingle());
            }

            OutputStream.WriteLine("");
            OutputStream.WriteLine("{0, -5}: Deity = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Intoxication = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("");

            //Buffer.SkipBytes(8); // Deity, intoxication

            UInt32 UnknownCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: Unknown Count = {1}", Buffer.GetPosition() - 4, UnknownCount);



            for (int i = 0; i < UnknownCount; ++i)
            {
                OutputStream.WriteLine("{0, -5}: Unknown : {1}, Value = {2}", Buffer.GetPosition(), i, Buffer.ReadUInt32());
                //Buffer.SkipBytes(4);
            }

            UInt32 EquipmentCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: EquipmentCount = {1}", Buffer.GetPosition() - 4, EquipmentCount);

            for (int i = 0; i < EquipmentCount; ++i)
            {
                OutputStream.Write("{0, -5}: Equip: {1} Values: ", Buffer.GetPosition(), i);
                for (int j = 0; j < 5; ++j)
                    OutputStream.Write("{0} ", Buffer.ReadUInt32());

                OutputStream.WriteLine("");
                //Buffer.SkipBytes(20);
            }

            UInt32 EquipmentCount2 = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: EquipmentCount2 = {1}", Buffer.GetPosition() - 4, EquipmentCount2);

            for (int i = 0; i < EquipmentCount2; ++i)
            {
                OutputStream.Write("{0, -5}: Equip2: {1} Values: ", Buffer.GetPosition(), i);
                for (int j = 0; j < 5; ++j)
                    OutputStream.Write("{0} ", Buffer.ReadUInt32());

                OutputStream.WriteLine("");
                //Buffer.SkipBytes(20);
            }



            UInt32 TintCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: TintCount = {1}", Buffer.GetPosition() - 4, TintCount);

            for (int i = 0; i < TintCount; ++i)
            {
                OutputStream.WriteLine("{0, -5}: TintCount : {1}, Value = {2}", Buffer.GetPosition(), i, Buffer.ReadUInt32());
                //Buffer.SkipBytes(4);
            }

            UInt32 TintCount2 = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: TintCount2 = {1}", Buffer.GetPosition() - 4, TintCount2);

            for (int i = 0; i < TintCount; ++i)
            {
                OutputStream.WriteLine("{0, -5}: TintCount2 : {1}, Value = {2}", Buffer.GetPosition(), i, Buffer.ReadUInt32());
                //Buffer.SkipBytes(4);
            }

            OutputStream.WriteLine("{0, -5}: Hair Color = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Beard Color = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Eye1 Color = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Eye2 Color = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Hairstyle = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Beard = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Face = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Drakkin Heritage = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Drakkin Tattoo = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Drakkin Details = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Height = {1}", Buffer.GetPosition(), Buffer.ReadSingle());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadSingle());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadSingle());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadSingle());
            OutputStream.WriteLine("{0, -5}: Primary = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Secondary = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());



            //Buffer.SkipBytes(52);   // Per SEQ, this looks like face, haircolor, beardcolor etc.
            OutputStream.WriteLine("{0, -5}: Unspent Skill Points = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Mana = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Current HP = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            //UInt32 Points = Buffer.ReadUInt32();
            //UInt32 Mana = Buffer.ReadUInt32();
            //UInt32 CurHP = Buffer.ReadUInt32();

            //OutputStream.WriteLine("Points, Mana, CurHP = {0}, {1}, {2}", Points, Mana, CurHP);

            OutputStream.WriteLine("{0, -5}: STR = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: STA = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: CHA = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: DEX = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: INT = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: AGI = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: WIS = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            //Buffer.SkipBytes(28);
            //Buffer.SkipBytes(28);

            UInt32 AACount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: AA Count = {1}", Buffer.GetPosition() - 4, AACount);


            for (int i = 0; i < AACount; ++i)
            {
                OutputStream.WriteLine("   AA: {0}, Value: {1}, Unknown08: {2}", Buffer.ReadUInt32(), Buffer.ReadUInt32(), Buffer.ReadUInt32());
                //Buffer.SkipBytes(12);
            }

            UInt32 SkillCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: Skill Count = {1}", Buffer.GetPosition() - 4, SkillCount);

            for (int i = 0; i < SkillCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            UInt32 SomethingCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: Something Count = {1}", Buffer.GetPosition() - 4, SomethingCount);


            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            UInt32 DisciplineCount = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: Discipline Count = {1}", Buffer.GetPosition() - 4, DisciplineCount);

            for (int i = 0; i < DisciplineCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            UInt32 TimeStampCount = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: TimeStamp Count = {1}", Buffer.GetPosition() - 4, TimeStampCount);

            for (int i = 0; i < TimeStampCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            UInt32 RecastCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: Recast Count = {1}", Buffer.GetPosition() - 4, RecastCount);

            for (int i = 0; i < RecastCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            UInt32 TimeStamp2Count = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: TimeStamp2 Count = {1}", Buffer.GetPosition() - 4, TimeStamp2Count);

            for (int i = 0; i < TimeStamp2Count; ++i)
            {
                Buffer.SkipBytes(4);
            }


            UInt32 SpellBookSlots = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: SpellBookSlot Count = {1}", Buffer.GetPosition() - 4, SpellBookSlots);

            for (int i = 0; i < SpellBookSlots; ++i)
            {
                Buffer.SkipBytes(4);
            }

            UInt32 SpellMemSlots = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: Spell Mem Count = {1}", Buffer.GetPosition() - 4, SpellMemSlots);

            for (int i = 0; i < SpellMemSlots; ++i)
            {
                Buffer.SkipBytes(4);
            }

            SomethingCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: Unknown Count = {1}", Buffer.GetPosition() - 4, SomethingCount);

            for (int i = 0; i < SomethingCount; ++i)
            {
                Buffer.SkipBytes(4);
            }

            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadByte());

            UInt32 BuffCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: Buff Count = {1}", Buffer.GetPosition() - 4, BuffCount);

            for (int i = 0; i < BuffCount; ++i)
            {
                Buffer.ReadByte();
                float UnkFloat = Buffer.ReadSingle();
                UInt32 PlayerID = Buffer.ReadUInt32();
                Byte UnkByte = Buffer.ReadByte();
                UInt32 Counters1 = Buffer.ReadUInt32();
                UInt32 Duration = Buffer.ReadUInt32();
                Byte Level = Buffer.ReadByte();
                UInt32 SpellID = Buffer.ReadUInt32();
                UInt32 SlotID = Buffer.ReadUInt32();
                Buffer.SkipBytes(5);
                UInt32 Counters2 = Buffer.ReadUInt32();
                OutputStream.WriteLine("Sl: {0}, UF: {1}, PID: {2}, UByte: {3}, Cnt1: {4}, Dur: {5}, Lvl: {6} SpellID: {7}, SlotID: {8}, Cnt2: {9}",
                    i, UnkFloat, PlayerID, UnkByte, Counters1, Duration, Level, SpellID, SlotID, Counters2);
                Buffer.SkipBytes(44);
            }

            OutputStream.WriteLine("{0, -5}: Plat = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Gold = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Silver = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Copper = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Plat Cursor = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Gold Cursor = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Silver Cursor = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Copper Cursor = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Toxicity? = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Thirst? = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Hunger? = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            //Buffer.SkipBytes(20);

            OutputStream.WriteLine("{0, -5}: AA Spent = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: AA Point Count? = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: AA Assigned = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: AA Spent General = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: AA Spent Archetype = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: AA Spent Class = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: AA Spent Special = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: AA Unspent = {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown", Buffer.GetPosition(), Buffer.ReadUInt16());


            //Buffer.SkipBytes(30);

            UInt32 BandolierCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: Bandolier Count = {1}", Buffer.GetPosition() - 4, BandolierCount);

            for (int i = 0; i < BandolierCount; ++i)
            {
                Buffer.ReadString(false);

                Buffer.ReadString(false);
                Buffer.SkipBytes(8);

                Buffer.ReadString(false);
                Buffer.SkipBytes(8);

                Buffer.ReadString(false);
                Buffer.SkipBytes(8);

                Buffer.ReadString(false);
                Buffer.SkipBytes(8);
            }

            UInt32 PotionCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: Potion Count = {1}", Buffer.GetPosition() - 4, PotionCount);

            for (int i = 0; i < PotionCount; ++i)
            {
                Buffer.ReadString(false);
                Buffer.SkipBytes(8);
            }

            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadInt32());
            OutputStream.WriteLine("{0, -5}: Item HP Total? {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Endurance Total? {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Mana Total? {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Expansion Count {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            UInt32 NameLength = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: Name Length: {1}", Buffer.GetPosition() - 4, NameLength);

            int CurrentPosition = Buffer.GetPosition();
            OutputStream.WriteLine("{0, -5}: Name: {1}", Buffer.GetPosition(), Buffer.ReadString(false));

            Buffer.SetPosition(CurrentPosition + (int)NameLength);

            UInt32 LastNameLength = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: LastName Length: {1}", Buffer.GetPosition() - 4, LastNameLength);

            CurrentPosition = Buffer.GetPosition();
            OutputStream.WriteLine("{0, -5}: Last Name: {1}", Buffer.GetPosition(), Buffer.ReadString(false));

            Buffer.SetPosition(CurrentPosition + (int)LastNameLength);

            OutputStream.WriteLine("{0, -5}: Birthday {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Account Start Date {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Last Login Date {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Time Played Minutes {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Time Entitled On Account {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Expansions {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            UInt32 LanguageCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: Language Count = {1}", Buffer.GetPosition() - 4, LanguageCount);

            for (int i = 0; i < LanguageCount; ++i)
            {
                Buffer.SkipBytes(1);
            }

            OutputStream.WriteLine("{0, -5}: Zone ID {1}", Buffer.GetPosition(), Buffer.ReadUInt16());
            OutputStream.WriteLine("{0, -5}: Zone Instance {1}", Buffer.GetPosition(), Buffer.ReadUInt16());
            OutputStream.WriteLine("{0, -5}: Y,X,Z {1},{2},{3} Heading: {4}",
                Buffer.GetPosition(), Buffer.ReadSingle(), Buffer.ReadSingle(), Buffer.ReadSingle(), Buffer.ReadSingle());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());

            OutputStream.WriteLine("{0, -5}: GuildID? {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Experience {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());

            OutputStream.WriteLine("{0, -5}: Bank Plat {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Bank Gold {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Bank Silver {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Bank Copper {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            UInt32 Unknown42 = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: Unknown, value 42? {1}", Buffer.GetPosition() - 4, Unknown42);

            Buffer.SkipBytes((int)(Unknown42 * 8));

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Career Tribute Favour {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Current Tribute Favour {1}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());

            UInt32 PersonalTributeCount = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: Personal Tribute Count {1}", Buffer.GetPosition() - 4, PersonalTributeCount);
            Buffer.SkipBytes((int)(PersonalTributeCount * 8));

            UInt32 GuildTributeCount = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: Guild Tribute Count {1}", Buffer.GetPosition() - 4, GuildTributeCount);
            Buffer.SkipBytes((int)(GuildTributeCount * 8));

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("Skipping 121 bytes starting at offset {0}", Buffer.GetPosition());
            Buffer.SkipBytes(121);

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("Position now {0}", Buffer.GetPosition());

            UInt32 Unknown64 = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: Unknown64 {1}", Buffer.GetPosition() - 4, Unknown64);
            Buffer.SkipBytes((int)Unknown64);

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            Unknown64 = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: Unknown64 {1}", Buffer.GetPosition() - 4, Unknown64);
            Buffer.SkipBytes((int)Unknown64);

            Unknown64 = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: Unknown64 {1}", Buffer.GetPosition() - 4, Unknown64);
            Buffer.SkipBytes((int)Unknown64);

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("Skipping 320 bytes starting at offset {0}", Buffer.GetPosition());
            Buffer.SkipBytes(320);

            OutputStream.WriteLine("Skipping 343 bytes starting at offset {0}", Buffer.GetPosition());
            Buffer.SkipBytes(343);

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());

            UInt32 Unknown6 = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: Unknown6 {1} LDON Stuff ?", Buffer.GetPosition() - 4, Unknown6);

            for(int i = 0; i < Unknown6; ++i)
                OutputStream.WriteLine("{0, -5}: Unknown LDON? {1}", Buffer.GetPosition(), Buffer.ReadUInt32());


            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            Unknown64 = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: Unknown64 {1}", Buffer.GetPosition() - 4, Unknown64);
            Buffer.SkipBytes((int)Unknown64 * 4);

            // Air remaining ?
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            // Next 7 could be PVP stats,
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            // PVP LastKill struct ?
            OutputStream.WriteLine("Skipping string + 24 bytes starting at offset {0}", Buffer.GetPosition());
            //Buffer.SkipBytes(25);

            Byte b;
            do
            {
                b = Buffer.ReadByte();
            } while (b != 0);

            Buffer.SkipBytes(24);

            // PVP LastDeath struct ?
            OutputStream.WriteLine("Skipping string + 24 bytes starting at offset {0}", Buffer.GetPosition());
            //Buffer.SkipBytes(25);
            do
            {
                b = Buffer.ReadByte();
            } while (b != 0);

            Buffer.SkipBytes(24);

            // PVP Number of Kills in Last 24 hours ?
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            UInt32 Unknown50 = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: Unknown50 {1}", Buffer.GetPosition() - 4, Unknown50);
            // PVP Recent Kills ?
            OutputStream.WriteLine("Skipping 50 x (String + 24 bytes) starting at offset {0}", Buffer.GetPosition());
            //Buffer.SkipBytes(1338);
            for (int i = 0; i < 50; ++i)
            {
                do
                {
                    b = Buffer.ReadByte();
                } while (b != 0);

                Buffer.SkipBytes(24);

            }



            //

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Group autoconsent? {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Raid autoconsent? {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Guild autoconsent? {1:X}", Buffer.GetPosition(), Buffer.ReadByte());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadByte());

            OutputStream.WriteLine("{0, -5}: Level3? {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("{0, -5}: Showhelm? {1}", Buffer.GetPosition(), Buffer.ReadByte());

            OutputStream.WriteLine("{0, -5}: RestTimer? {1}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("Skipping 1028 bytes starting at offset {0}", Buffer.GetPosition());
            Buffer.SkipBytes(1028);

            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());
            OutputStream.WriteLine("{0, -5}: Unknown {1:X}", Buffer.GetPosition(), Buffer.ReadUInt32());

            OutputStream.WriteLine("Pointer is {0} bytes from end.", Buffer.Length() - Buffer.GetPosition());



        }

        public void ExploreZoneEntry(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            if (Direction != PacketDirection.ServerToClient)
                return;

            string FirstName = Buffer.ReadString(false);

            OutputStream.WriteLine("Name = {0}", FirstName);

            UInt32 SpawnID = Buffer.ReadUInt32();

            OutputStream.WriteLine("SpawnID = {0}", SpawnID);

            byte Level = Buffer.ReadByte();

            OutputStream.WriteLine("Level = {0}", Level);

            Buffer.SkipBytes(4);

            byte IsNPC = Buffer.ReadByte();

            OutputStream.WriteLine("IsNPC = {0}", IsNPC);

            UInt32 Bitfield = Buffer.ReadUInt32();
            OutputStream.WriteLine("Name: {0}, Bitfield: {1}", FirstName, Convert.ToString(Bitfield, 2));

            byte OtherData = Buffer.ReadByte();

            OutputStream.WriteLine("OtherData = {0}", OtherData);

            Buffer.SkipBytes(8);

            if ((OtherData & 1) > 0)
            {
                OutputStream.WriteLine("OD:     {0}", Buffer.ReadString(false));
                OutputStream.WriteLine("OD:     {0}", Buffer.ReadString(false));
                OutputStream.WriteLine("OD:     {0}", Buffer.ReadString(false));
                Buffer.SkipBytes(53);
            }

            if ((OtherData & 4) > 0)
            {
                OutputStream.WriteLine("Aura:     {0}", Buffer.ReadString(false));
                OutputStream.WriteLine("Aura:     {0}", Buffer.ReadString(false));
                Buffer.SkipBytes(54);
            }

            byte Properties = Buffer.ReadByte();
            OutputStream.WriteLine("Properties = {0}, Offset now {1}", Properties, Buffer.GetPosition());

            UInt32 BodyType = 0;

            if(Properties > 0)
                BodyType = Buffer.ReadUInt32();

            OutputStream.WriteLine("Bodytype = {0}",  BodyType);

            if (Properties != 1)
                OutputStream.WriteLine("XXXX Properties is {0}", Properties);

            for (int i = 1; i < Properties; ++i)
                OutputStream.WriteLine("   Prop: {0}", Buffer.ReadUInt32());

            OutputStream.WriteLine("Position is now {0}", Buffer.GetPosition());

            byte HP = Buffer.ReadByte();
            byte HairColor = Buffer.ReadByte();
            byte BeardColor = Buffer.ReadByte();
            byte Eye1 = Buffer.ReadByte();
            byte Eye2 = Buffer.ReadByte();
            byte HairStyle = Buffer.ReadByte();
            byte BeardStyle = Buffer.ReadByte();
            OutputStream.WriteLine("Beardstyle is {0}", BeardStyle);

            Buffer.SkipBytes(12);   // Drakkin stuff
            byte EquipChest2 = Buffer.ReadByte();
            Buffer.SkipBytes(2);
            byte Helm = Buffer.ReadByte();


            float Size = Buffer.ReadSingle();

            byte Face = Buffer.ReadByte();

            float WalkSpeed = Buffer.ReadSingle();

            float RunSpeed = Buffer.ReadSingle();

            UInt32 Race = Buffer.ReadUInt32();

            OutputStream.WriteLine("Size: {0}, Face: {1}, Walkspeed: {2}, RunSpeed: {3}, Race: {4}", Size, Face, WalkSpeed, RunSpeed, Race);

            //Buffer.SkipBytes(18);
            Buffer.SkipBytes(5);
            UInt32 GuildID = Buffer.ReadUInt32();
            UInt32 GuildRank = Buffer.ReadUInt32();
            Buffer.SkipBytes(5);
            OutputStream.WriteLine("GuildID: {0}, Guild Rank: {1}", GuildID, GuildRank);

            Buffer.ReadString(false);

            Buffer.SkipBytes(35);


            if ((IsNPC == 0) || NPCType.IsPlayableRace(Race))
            {
                for (int ColourSlot = 0; ColourSlot < 9; ++ColourSlot)
                    OutputStream.WriteLine("Color {0} is {1}", ColourSlot, Buffer.ReadUInt32());

                for (int i = 0; i < 9; ++i)
                {
                    UInt32 Equip3 = Buffer.ReadUInt32();

                    UInt32 Equipx = Buffer.ReadUInt32();

                    UInt32 Equip2 = Buffer.ReadUInt32();

                    UInt32 Equip1 = Buffer.ReadUInt32();

                    UInt32 Equip0 = Buffer.ReadUInt32();

                    OutputStream.WriteLine("Equip slot {0}: 0,1,2,x,3  is {1}, {2}, {3}, {4}, {5}", i,
                        Equip0, Equip1, Equip2, Equipx, Equip3);
                }





            }
            else
            {
                // Non playable race
                // Melee Texture 1 is 20 bytes in
                // Melee Texture 1 is 40 bytes in
                // This whole segment is 28 + 24 + 8 = 60
                // Skip 20, Read m1, skip 16, read m2, skip 16
                /*
                OutputStream.WriteLine("None playable race,  offset now {0}", Buffer.GetPosition());
                Buffer.SkipBytes(28);

                UInt32 MeleeTexture1 = Buffer.ReadUInt32();
                Buffer.SkipBytes(12);
                UInt32 MeleeTexture2 = Buffer.ReadUInt32();
                Buffer.SkipBytes(12);
                 */
                OutputStream.WriteLine("None playable race,  offset now {0}", Buffer.GetPosition());
                Buffer.SkipBytes(20);

                UInt32 MeleeTexture1 = Buffer.ReadUInt32();
                Buffer.SkipBytes(16);
                UInt32 MeleeTexture2 = Buffer.ReadUInt32();
                Buffer.SkipBytes(16);
            }

            OutputStream.WriteLine("Position starts at offset {0}", Buffer.GetPosition());

            UInt32 Position1 = Buffer.ReadUInt32();

            UInt32 Position2 = Buffer.ReadUInt32();

            UInt32 Position3 = Buffer.ReadUInt32();

            UInt32 Position4 = Buffer.ReadUInt32();

            UInt32 Position5 = Buffer.ReadUInt32();

            float YPos = Utils.EQ19ToFloat((Int32)(Position1 >> 12));

            float ZPos = Utils.EQ19ToFloat((Int32)(Position3 >> 13) & 0x7FFFF);

            float XPos = Utils.EQ19ToFloat((Int32)(Position4) & 0x7FFFF);

            float Heading = Utils.EQ19ToFloat((Int32)(Position5) & 0x7FFFF);

            OutputStream.WriteLine("(X,Y,Z) = {0}, {1}, {2}, Heading = {3}", XPos, YPos, ZPos, Heading);

            if((OtherData & 16) > 1)
                OutputStream.WriteLine("Title: {0}", Buffer.ReadString(false));

            if ((OtherData & 32) > 1)
                OutputStream.WriteLine("Suffix: {0}", Buffer.ReadString(false));

            Buffer.SkipBytes(8);

            byte IsMerc = Buffer.ReadByte();

            OutputStream.WriteLine("IsMerc: {0}", IsMerc);

            Buffer.SkipBytes(54);

            OutputStream.WriteLine("Buffer Length: {0}, Current Position: {1}", Buffer.Length(), Buffer.GetPosition());

            if (Buffer.Length() != Buffer.GetPosition())
                OutputStream.WriteLine("PARSE ERROR");





            OutputStream.WriteLine("");
        }
    }
}