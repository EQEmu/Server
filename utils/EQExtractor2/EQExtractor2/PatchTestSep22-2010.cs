using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchTestSep222010Decoder : PatchTestSep012010Decoder
    {
        public PatchTestSep222010Decoder()
        {
            Version = "EQ Client Build Date Test Server September 22 2010.";

            PatchConfFileName = "patch_Sep22-2010.conf";

            ExpectedPPLength = 26728;
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

        override public List<Door> GetDoors()
        {
            List<Door> DoorList = new List<Door>();

            List<byte[]> SpawnDoorPacket = GetPacketsOfType("OP_SpawnDoor", PacketDirection.ServerToClient);

            if ((SpawnDoorPacket.Count == 0) || (SpawnDoorPacket[0].Length == 0))
                return DoorList;

            int DoorCount = SpawnDoorPacket[0].Length / 96;

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

                Buffer.SkipBytes(28);

                string DestZone = "NONE";

                Door NewDoor = new Door(DoorName, YPos, XPos, ZPos, Heading, Incline, Size, DoorID, OpenType, StateAtSpawn, InvertState,
                                        DoorParam, DestZone, 0, 0, 0, 0);

                DoorList.Add(NewDoor);

            }
            return DoorList;
        }

        override public Item DecodeItemPacket(byte[] PacketBuffer)
        {
            ByteStream Buffer = new ByteStream(PacketBuffer);

            Item NewItem = new Item();

            //NewItem.StackSize = Buffer.ReadUInt32();             // 00
            //Buffer.SkipBytes(4);
            //NewItem.Slot = Buffer.ReadUInt32();                  // 08
            //Buffer.SkipBytes(1);
            Buffer.SetPosition(30);
            NewItem.MerchantSlot = Buffer.ReadByte();            // 13
            NewItem.Price = Buffer.ReadUInt32();                 // 14
            Buffer.SkipBytes(5);
            NewItem.Quantity = Buffer.ReadInt32();               // 23
            Buffer.SetPosition(96);
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
            Buffer.SkipBytes(17);
            Buffer.SkipBytes(8);

            byte Area = Buffer.ReadByte();
            UInt16 MainSlot = Buffer.ReadUInt16();
            Int16 SubSlot = Buffer.ReadInt16();
            Buffer.SkipBytes(62);
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
            Buffer.SkipBytes(3);
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

            //Buffer.SkipBytes(64);   // Item Secondary Body Struct

            UInt32 AugType = Buffer.ReadUInt32();
            UInt32 AugRestrict = Buffer.ReadUInt32();

            Buffer.SkipBytes(30);   // Augslot stuff

            UInt32 LDONPointType = Buffer.ReadUInt32();
            UInt32 LDONTheme = Buffer.ReadUInt32();
            UInt32 LDONPrice = Buffer.ReadUInt32();
            UInt32 LDONSellBackRate = Buffer.ReadUInt32();
            UInt32 LDONSold = Buffer.ReadUInt32();

            byte BagType = Buffer.ReadByte();
            byte BagSlots = Buffer.ReadByte();
            byte BagSize = Buffer.ReadByte();
            byte BagWeightReduction = Buffer.ReadByte();

            byte Book = Buffer.ReadByte();
            byte BookType = Buffer.ReadByte();

            OutputStream.WriteLine("\r\n     BagType: {0} BagSlots: {1} BagSize: {2} BagWR: {3}", BagType, BagSlots, BagSize, BagWeightReduction);



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

            Buffer.SkipBytes(57);   // HoT

            OutputStream.WriteLine("    Reading sublengths from offset {0}", Buffer.GetPosition());
            UInt32 SubLengths = Buffer.ReadUInt32();



            for (int i = 0; i < SubLengths; ++i)
            {
                Buffer.SkipBytes(4);
                ExploreSubItem(OutputStream, ref Buffer);
            }

            OutputStream.WriteLine("");
        }

    }
}