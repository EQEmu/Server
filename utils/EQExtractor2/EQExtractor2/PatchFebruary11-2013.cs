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
    class PatchFebruary112013Decoder : PatchJanuary162013Decoder
    {
        public PatchFebruary112013Decoder()
        {
            Version = "EQ Client Build Date February 11 2013.";

            PatchConfFileName = "patch_Feb11-2013.conf";

            PacketsToMatch = new PacketToMatch[] {
                new PacketToMatch { OPCodeName = "OP_ZoneEntry", Direction = PacketDirection.ClientToServer, RequiredSize = 76, VersionMatched = false },
                new PacketToMatch { OPCodeName = "OP_PlayerProfile", Direction = PacketDirection.ServerToClient, RequiredSize = -1, VersionMatched = true },
            };

            SupportsSQLGeneration = false;
        }

        public override void RegisterExplorers()
        {
            //OpManager.RegisterExplorer("OP_PlayerProfile", ExplorePlayerProfile);
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
            //Buffer.SkipBytes(26);   // ClickEffect Struct
            //OutputStream.WriteLine("Click Effect - effect : {0}", Buffer.ReadUInt32());
            OutputStream.WriteLine("Click Effect - level2 : {0}", Buffer.ReadByte());
            OutputStream.WriteLine("Click Effect - Type : {0}", Buffer.ReadUInt32());
            OutputStream.WriteLine("Click Effect - level : {0}", Buffer.ReadByte());
            OutputStream.WriteLine("Click Effect - Max Charges : {0}", Buffer.ReadUInt32());
            OutputStream.WriteLine("Click Effect - Cast Time : {0}", Buffer.ReadUInt32());
            OutputStream.WriteLine("Click Effect - Recast : {0}", Buffer.ReadUInt32());
            OutputStream.WriteLine("Click Effect - Recast Type: {0}", Buffer.ReadUInt32());
            OutputStream.WriteLine("Click Effect - Unk5: {0}", Buffer.ReadUInt32());
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
                    Buffer.GetPosition(), i, Buffer.ReadUInt32(), Buffer.ReadSingle(), Buffer.ReadSingle(), Buffer.ReadSingle(), Buffer.ReadSingle());
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
                //Buffer.SkipBytes(4);
                OutputStream.WriteLine("Something {0} : {1}", i, Buffer.ReadUInt32());
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
                //Buffer.SkipBytes(4);
                OutputStream.WriteLine("Timestamp {0} : {1}", i, Buffer.ReadUInt32());
            }

            System.DateTime dateTime;
            UInt32 RecastCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("{0, -5}: Recast Count = {1}", Buffer.GetPosition() - 4, RecastCount);

            for (int i = 0; i < RecastCount; ++i)
            {
                //Buffer.SkipBytes(4);
                UInt32 TimeStamp = Buffer.ReadUInt32();
                dateTime = new System.DateTime(1970, 1, 1, 0, 0, 0, 0);
                dateTime = dateTime.AddSeconds(TimeStamp);
                OutputStream.WriteLine("Recast {0} : {1} {2}", i, TimeStamp, dateTime.ToString());
            }

            UInt32 TimeStamp2Count = Buffer.ReadUInt32();
            OutputStream.WriteLine("{0, -5}: TimeStamp2 Count = {1}", Buffer.GetPosition() - 4, TimeStamp2Count);

            for (int i = 0; i < TimeStamp2Count; ++i)
            {
                //Buffer.SkipBytes(4);
                UInt32 TimeStamp = Buffer.ReadUInt32();
                dateTime = new System.DateTime(1970, 1, 1, 0, 0, 0, 0);
                dateTime = dateTime.AddSeconds(TimeStamp);

                OutputStream.WriteLine("Timestamp {0} : {1} {2}", i, TimeStamp, dateTime.ToString());
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
                //Buffer.SkipBytes(4);
                OutputStream.WriteLine("Unknown {0} : {1}", i, Buffer.ReadUInt32());

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

            for (int i = 0; i < Unknown6; ++i)
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
    }
}