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
    class PatchApril152013Decoder : PatchMarch132013Decoder
    {
        public PatchApril152013Decoder()
        {
            Version = "EQ Client Build Date April 15 2013.";

            PatchConfFileName = "patch_April15-2013.conf";

            SupportsSQLGeneration = true;
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

                NewSpawn.Gender = (Bitfield & 3);

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

                if ((NewSpawn.IsNPC == 0) || NPCType.IsPlayableRace(NewSpawn.Race))
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

                NewSpawn.YPos = Utils.EQ19ToFloat((Int32)((Position1 >> 12) & 0x7FFFF));

                NewSpawn.ZPos = Utils.EQ19ToFloat((Int32)(Position2) & 0x7FFFF);

                NewSpawn.XPos = Utils.EQ19ToFloat((Int32)(Position4 >> 13) & 0x7FFFF);

                NewSpawn.Heading = Utils.EQ19ToFloat((Int32)(Position3 >> 13) & 0xFFF);

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
    }
}