using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchMay122011Decoder : PatchMarch152011Decoder
    {
        public PatchMay122011Decoder()
        {
            Version = "EQ Client Build Date May 12 2011.";

            PatchConfFileName = "patch_May12-2011.conf";

            ExpectedPPLength = 28536;

            PPZoneIDOffset = 21204;
        }

        override public PositionUpdate Decode_OP_NPCMoveUpdate(byte[] UpdatePacket)
        {
            PositionUpdate PosUpdate = new PositionUpdate();

            BitStream bs = new BitStream(UpdatePacket, (uint)UpdatePacket.Length);

            PosUpdate.SpawnID = bs.readUInt(16);

            UInt32 Unknown = bs.readUInt(16);

            UInt32 VFlags = bs.readUInt(6);

            PosUpdate.p.y = (float)bs.readInt(19) / (float)(1 << 3);

            PosUpdate.p.x = (float)bs.readInt(19) / (float)(1 << 3);

            PosUpdate.p.z = (float)bs.readInt(19) / (float)(1 << 3);

            PosUpdate.p.heading = (float)bs.readInt(12) / (float)(1 << 3);

            PosUpdate.HighRes = true;

            return PosUpdate;
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

            PosUpdate.p.x = Utils.EQ19ToFloat((Int32)(Word2 >> 6) & 0x7FFFF);

            PosUpdate.p.heading = Utils.EQ19ToFloat((Int32)(Word3 & 0xFFF));

            PosUpdate.HighRes = false;

            return PosUpdate;
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

                Buffer.SkipBytes(8);    // Skip the last UInt32

                ZonePoint NewZonePoint = new ZonePoint(Number, ZoneID, Instance, x, y, z, x, y, z, Heading, ZoneID);

                ZonePointList.Add(NewZonePoint);
            }

            return ZonePointList;
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
                UInt32 Unknown037 = BitConverter.ToUInt32(Packet, 37);
                UInt32 PrereqSkill = BitConverter.ToUInt32(Packet, 41);
                UInt32 PrereqMinpoints = BitConverter.ToUInt32(Packet, 45);
                UInt32 Type = BitConverter.ToUInt32(Packet, 49);
                UInt32 SpellID = BitConverter.ToUInt32(Packet, 53);
                UInt32 Unknown057 = BitConverter.ToUInt32(Packet, 57);
                UInt32 SpellType = BitConverter.ToUInt32(Packet, 61);
                UInt32 SpellRefresh = BitConverter.ToUInt32(Packet, 65);
                UInt16 Classes = BitConverter.ToUInt16(Packet, 69);
                UInt16 Berserker = BitConverter.ToUInt16(Packet, 71);
                UInt32 MaxLevel = BitConverter.ToUInt32(Packet, 73);
                UInt32 LastID = BitConverter.ToUInt32(Packet, 77);
                UInt32 NextID = BitConverter.ToUInt32(Packet, 81);
                UInt32 Cost2 = BitConverter.ToUInt32(Packet, 85);
                UInt32 AAExpansion = BitConverter.ToUInt32(Packet, 96);
                UInt32 SpecialCategory = BitConverter.ToUInt32(Packet, 100);
                UInt32 TotalAbilities = BitConverter.ToUInt32(Packet, 108);

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
                OutputFile.WriteLine(" Unknown037:\t" + Unknown037);
                OutputFile.WriteLine(" PrereqSkill:\t" + PrereqSkill);
                OutputFile.WriteLine(" PrereqMinPt:\t" + PrereqMinpoints);
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
                    UInt32 Ability = BitConverter.ToUInt32(Packet, 112 + (i * 16));
                    Int32 Base1 = BitConverter.ToInt32(Packet, 116 + (i * 16));
                    Int32 Base2 = BitConverter.ToInt32(Packet, 120 + (i * 16));
                    UInt32 Slot = BitConverter.ToUInt32(Packet, 124 + (i * 16));

                    OutputFile.WriteLine(String.Format("    Ability:\t{0}\tBase1:\t{1}\tBase2:\t{2}\tSlot:\t{3}", Ability, Base1, Base2, Slot));

                }
                OutputFile.WriteLine("");

            }

            OutputFile.Close();

            return true;
        }

        public override void RegisterExplorers()
        {
            //base.RegisterExplorers();

            //OpManager.RegisterExplorer("OP_SpawnDoor", ExploreSpawnDoor);

        }

        public void ExploreSpawnDoor(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            uint DoorCount = Buffer.Length() / 96;

            OutputStream.WriteLine("Door Count: {0}", DoorCount);

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

                OutputStream.WriteLine(" Name: {0} ID: {1} OT: {2} SAS: {3} IS: {4} DP: {5}",
                                        DoorName, DoorID, OpenType, StateAtSpawn, InvertState, DoorParam);

                // Skip past the trailing unknowns in the door struct, moving to the next door in the packet.

                Buffer.SkipBytes(28);
            }

        }
    }
}