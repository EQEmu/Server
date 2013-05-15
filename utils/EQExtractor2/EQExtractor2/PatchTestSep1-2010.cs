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
    class PatchTestSep012010Decoder : PatchJuly132010Decoder
    {
        public PatchTestSep012010Decoder()
        {
            Version = "EQ Client Build Date Test Server September 1 2010.";

            PatchConfFileName = "patch_Sep01-2010.conf";
        }

        override public IdentificationStatus Identify(int OpCode, int Size, PacketDirection Direction)
        {
            if((OpCode == OpManager.OpCodeNameToNumber("OP_ZoneEntry")) && (Direction == PacketDirection.ClientToServer))
                return IdentificationStatus.Yes;

            return IdentificationStatus.No;
        }

        override public Item DecodeItemPacket(byte[] PacketBuffer)
        {
            ByteStream Buffer = new ByteStream(PacketBuffer);

            Item NewItem = new Item();

            NewItem.StackSize = Buffer.ReadUInt32();             // 00
            Buffer.SkipBytes(4);
            NewItem.Slot = Buffer.ReadUInt32();                  // 08
            Buffer.SkipBytes(1);
            NewItem.MerchantSlot = Buffer.ReadByte();            // 13
            NewItem.Price = Buffer.ReadUInt32();                 // 14
            Buffer.SkipBytes(5);
            NewItem.Quantity = Buffer.ReadInt32();               // 23
            Buffer.SetPosition(71);
            NewItem.Name = Buffer.ReadString(true);
            NewItem.Lore = Buffer.ReadString(true);
            NewItem.IDFile = Buffer.ReadString(true);
            NewItem.ID = Buffer.ReadUInt32();

            return NewItem;
        }

        public override void RegisterExplorers()
        {
            base.RegisterExplorers();

            //OpManager.RegisterExplorer("OP_CharInventory", ExploreCharInventoryPacket);
            //OpManager.RegisterExplorer("OP_ItemPacket", ExploreItemPacket);
            //OpManager.RegisterExplorer("OP_MercenaryPurchaseWindow", ExploreMercenaryPurchaseWindow);
        }

        public void ExploreCharInventoryPacket(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt32 ItemCount = Buffer.ReadUInt32();

            OutputStream.WriteLine("There are {0} items in the inventory.\r\n", ItemCount );

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

        void  ExploreSubItem(StreamWriter OutputStream, ref ByteStream Buffer)
        {
            Buffer.SkipBytes(8);

            byte Area = Buffer.ReadByte();
            UInt16 MainSlot = Buffer.ReadUInt16();
            Int16 SubSlot = Buffer.ReadInt16();
            Buffer.SkipBytes(54);
            string Name = Buffer.ReadString(true);

            if (SubSlot >= 0)
                OutputStream.Write("  ");

            string AreaName = "Unknown";

            switch (Area)
            {
                case 0:
                    AreaName = "Personal Inventory";
                    break;
                case 1:
                    AreaName = "Bank";
                    break;
                case 2:
                    AreaName = "Shared Bank";
                    break;
                case 6:
                    AreaName = "Personal Tribute";
                    break;
                case 7:
                    AreaName = "Guild Tribute";
                    break;
                case 8:
                    AreaName = "Merchant";
                    break;
            }

            OutputStream.WriteLine("Area: {0} {1} Main Slot {2,2} Sub Slot {3,3} Name {4}", Area, AreaName.PadRight(20), MainSlot, SubSlot, Name);

            Buffer.ReadString(true);    // Lore
            Buffer.ReadString(true);    // IDFile

            //Buffer.SkipBytes(236);  // Item Body Struct

            UInt32 ID = Buffer.ReadUInt32();
            byte Weight = Buffer.ReadByte();
            byte NoRent = Buffer.ReadByte();
            byte NoDrop = Buffer.ReadByte();
            byte Attune = Buffer.ReadByte();
            byte Size = Buffer.ReadByte();

            OutputStream.WriteLine("   ID: {0} Weight: {1} NoRent: {2} NoDrop: {3} Attune {4} Size {5}", ID, Weight, NoRent, NoDrop, Attune, Size);

            UInt32 Slots = Buffer.ReadUInt32();
            UInt32 Price = Buffer.ReadUInt32();
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

            UInt32 SubLengths = Buffer.ReadUInt32();

            for (int i = 0; i < SubLengths; ++i)
            {
                Buffer.SkipBytes(4);
                ExploreSubItem(OutputStream, ref Buffer);
            }
        }

        public void ExploreMercenaryPurchaseWindow(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt32 TypeCount = Buffer.ReadUInt32();

            //OutputStream.WriteLine("Type Count: {0}\r\n", TypeCount);
            OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Number of Types (Journeyman and Apprentice in this case\r\n", TypeCount);
            for (int i = 0; i < TypeCount; ++i)
            {
                UInt32 TypeDBStringID = Buffer.ReadUInt32();
                //OutputStream.WriteLine("  Type {0} DBStringID {1}", i, TypeDBStringID);
                OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // DBStringID for Type {1}", TypeDBStringID, i);
            }

            UInt32 Count2 = Buffer.ReadUInt32();

            //OutputStream.WriteLine("  Count 2 is {0}", Count2);
            OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Count of Sub-types that follow", Count2);

            for (int i = 0; i < Count2; ++i)
            {
                int Offset = Buffer.GetPosition();

                UInt32 Unknown1 = Buffer.ReadUInt32();
                OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Unknown", Unknown1);
                UInt32 DBStringID1 = Buffer.ReadUInt32();
                OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // DBStringID of Type", DBStringID1);
                UInt32 DBStringID2 = Buffer.ReadUInt32();
                OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // DBStringID of Sub-Type", DBStringID2);
                UInt32 PurchaseCost = Buffer.ReadUInt32();
                OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Purchase Cost", PurchaseCost);
                UInt32 UpkeepCost = Buffer.ReadUInt32();
                OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Upkeep Cost", UpkeepCost);
                UInt32 Unknown2 = Buffer.ReadUInt32();
                OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Unknown", Unknown2);
                UInt32 Unknown3 = Buffer.ReadUInt32();
                OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Unknown", Unknown3);
                UInt32 Unknown4 = Buffer.ReadUInt32();
                OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Unknown", Unknown4);

                byte Unknown5 = Buffer.ReadByte();
                //OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint8, Buffer, {0}); // Unknown", Unknown5);

                UInt32 Unknown6 = Buffer.ReadUInt32();
                //OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Unknown", Unknown6);
                UInt32 Unknown7 = Buffer.ReadUInt32();
                //OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Unknown", Unknown7);
                UInt32 Unknown8 = Buffer.ReadUInt32();
                //OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Unknown", Unknown8);

                UInt32 StanceCount = Buffer.ReadUInt32();

                OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Number of Stances for this Merc", StanceCount);

                UInt32 Unknown10 = Buffer.ReadUInt32();
                OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Unknown", Unknown10);

                byte Unknown11 = Buffer.ReadByte();
                //OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint8, Buffer, {0}); // Unknown", Unknown11);


                //OutputStream.WriteLine("   Offset: {5} Unknown1: {0} DBStrings: {1} {2} Purchase: {3} Upkeep: {4}\r\n", Unknown1, DBStringID1, DBStringID2,
                //                PurchaseCost, UpkeepCost, Offset);
                //OutputStream.WriteLine("   Unknowns: {0} {1} {2} {3} {4} {5} {6} {7} {8}\r\n",
                //                Unknown2, Unknown3, Unknown4, Unknown5, Unknown6, Unknown7, Unknown8, Unknown10, Unknown11);

                //OutputStream.WriteLine("    Stance Count: {0}", StanceCount);

                for (int j = 0; j < StanceCount; ++j)
                {
                    UInt32 StanceNum = Buffer.ReadUInt32();
                    OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Stance Number", StanceNum);
                    UInt32 StanceType = Buffer.ReadUInt32();
                    OutputStream.WriteLine("VARSTRUCT_ENCODE_TYPE(uint32, Buffer, {0}); // Stance DBStringID (1 = Passive, 2 = Balanced etc.", StanceType);

                    //OutputStream.WriteLine("     {0}: {1}", StanceNum, StanceType);
                }
                OutputStream.WriteLine("");
            }

            OutputStream.WriteLine("\r\nBuffer position at end is {0}", Buffer.GetPosition());
            OutputStream.WriteLine("");
        }


    }
}
