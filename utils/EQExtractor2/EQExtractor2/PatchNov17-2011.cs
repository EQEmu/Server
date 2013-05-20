using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchNov172011Decoder : PatchAug042011Decoder
    {
        public PatchNov172011Decoder()
        {
            Version = "EQ Client Build Date November 17 2011.";

            ExpectedPPLength = 29560;

            PPZoneIDOffset = 22228;

            PatchConfFileName = "patch_Nov17-2011.conf";
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

                Buffer.SkipBytes(26);   // Unknown byte + 6 unknown uint32

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

        public override void RegisterExplorers()
        {
            //base.RegisterExplorers();

            //OpManager.RegisterExplorer("OP_NPCMoveUpdate", ExploreNPCMoveUpdate);
            //OpManager.RegisterExplorer("OP_MobUpdate", ExploreMobUpdate);

        }

        public void ExploreNPCMoveUpdate(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            BitStream bs = new BitStream(Buffer.Buffer, Buffer.Length());

            UInt32 SpawnID = bs.readUInt(16);

            UInt32 Unknown = bs.readUInt(16);

            UInt32 VFlags = bs.readUInt(6);

            float y = (float)bs.readInt(19) / (float)(1 << 3);

            float x = (float)bs.readInt(19) / (float)(1 << 3);

            float z = (float)bs.readInt(19) / (float)(1 << 3);

            float heading = (float)bs.readInt(12) / (float)(1 << 3);

            OutputStream.WriteLine("Spawn: {0} Flags: {1}, XYZ: {2}, {3}, {4}, Heading: {5}", SpawnID, VFlags, x, y, z, heading);
        }

        public void ExploreMobUpdate(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt32 SpawnID = Buffer.ReadUInt16();

            Buffer.SkipBytes(2);

            UInt32 Word1 = Buffer.ReadUInt32();

            UInt32 Word2 = Buffer.ReadUInt32();

            UInt16 Word3 = Buffer.ReadUInt16();

            float y = Utils.EQ19ToFloat((Int32)(Word1 & 0x7FFFF));

            // Z is in the top 13 bits of Word1 and the bottom 6 of Word2

            UInt32 ZPart1 = Word1 >> 19;    // ZPart1 now has low order bits of Z in bottom 13 bits
            UInt32 ZPart2 = Word2 & 0x3F;   // ZPart2 now has high order bits of Z in bottom 6 bits

            ZPart2 = ZPart2 << 13;

            float z = Utils.EQ19ToFloat((Int32)(ZPart1 | ZPart2));

            float x = Utils.EQ19ToFloat((Int32)(Word2 >> 6) & 0x7FFFF);

            float heading = Utils.EQ19ToFloat((Int32)(Word3 & 0xFFF));

            OutputStream.WriteLine("Spawn: {0} XYZ: {1}, {2}, {3}, Heading: {4}", SpawnID, x, y, z, heading);
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
                ByteStream Buffer = new ByteStream(Packet);

                UInt32 AAID = Buffer.ReadUInt32();
                byte Unknown004 = Buffer.ReadByte();
                UInt32 HotKeySID = Buffer.ReadUInt32();
                UInt32 HotKeySID2 = Buffer.ReadUInt32();
                UInt32 TitleSID = Buffer.ReadUInt32();
                UInt32 DescSID = Buffer.ReadUInt32();
                UInt32 ClassType = Buffer.ReadUInt32();
                UInt32 Cost = Buffer.ReadUInt32();
                UInt32 Seq = Buffer.ReadUInt32();
                UInt32 CurrentLevel = Buffer.ReadUInt32();

                UInt32 PreReqSkillCount = Buffer.ReadUInt32();
                UInt32 [] PreReqSkills = new UInt32[PreReqSkillCount];

                for(int i = 0; i < PreReqSkillCount; ++i)
                    PreReqSkills[i] = Buffer.ReadUInt32();  // The SEQ value of the AA packet containing the pre-req AA

                UInt32 PreReqMinPointCount = Buffer.ReadUInt32();
                UInt32[] PreReqMinPoints = new UInt32[PreReqMinPointCount];

                for (int i = 0; i < PreReqMinPointCount; ++i)
                    PreReqMinPoints[i] = Buffer.ReadUInt32();

                UInt32 Type = Buffer.ReadUInt32();
                UInt32 SpellID = Buffer.ReadUInt32();
                UInt32 Unknown057 = Buffer.ReadUInt32();
                UInt32 SpellType = Buffer.ReadUInt32();
                UInt32 SpellRefresh = Buffer.ReadUInt32();
                UInt16 Classes = Buffer.ReadUInt16();
                UInt16 Berserker = Buffer.ReadUInt16();
                UInt32 MaxLevel = Buffer.ReadUInt32();
                UInt32 LastID = Buffer.ReadUInt32();
                UInt32 NextID = Buffer.ReadUInt32();
                UInt32 Cost2 = Buffer.ReadUInt32();
                Buffer.SkipBytes(7);
                UInt32 AAExpansion = Buffer.ReadUInt32();
                UInt32 SpecialCategory = Buffer.ReadUInt32();
                Buffer.SkipBytes(4);
                UInt32 TotalAbilities = Buffer.ReadUInt32();

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
                OutputFile.Write(" PreReqSkills (SEQs):\t");
                for (int i = 0; i < PreReqSkillCount; ++i)
                    OutputFile.Write("{0} ", PreReqSkills[i]);

                OutputFile.WriteLine("");

                OutputFile.Write(" PreReqSkills MinPoints:\t");
                for (int i = 0; i < PreReqMinPointCount; ++i)
                    OutputFile.Write("{0} ", PreReqMinPoints[i]);

                OutputFile.WriteLine("");

                OutputFile.WriteLine(" Type:\t\t" + Type);
                OutputFile.WriteLine(" SpellID:\t" + SpellID);
                OutputFile.WriteLine(" Unknown057:\t" + Unknown057);
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
                    UInt32 Ability = Buffer.ReadUInt32();
                    Int32 Base1 = Buffer.ReadInt32();
                    Int32 Base2 = Buffer.ReadInt32();
                    UInt32 Slot = Buffer.ReadUInt32();

                    OutputFile.WriteLine(String.Format("    Ability:\t{0}\tBase1:\t{1}\tBase2:\t{2}\tSlot:\t{3}", Ability, Base1, Base2, Slot));

                }
                OutputFile.WriteLine("");

            }

            OutputFile.Close();

            return true;
        }
    }
}