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
    class PatchJanuary162013Decoder : PatchDecember102012Decoder
    {
        public PatchJanuary162013Decoder()
        {
            Version = "EQ Client Build Date January 16 2013.";

            PatchConfFileName = "patch_Jan16-2013.conf";
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

                NewSpawn.YPos = Utils.EQ19ToFloat((Int32)((Position4 >> 13) & 0x7FFFF));

                NewSpawn.ZPos = Utils.EQ19ToFloat((Int32)(Position5 >> 10) & 0x7FFFF);

                NewSpawn.XPos = Utils.EQ19ToFloat((Int32)(Position2 >> 10) & 0x7FFFF);

                NewSpawn.Heading = Utils.EQ19ToFloat((Int32)(Position3) & 0xFFF);

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
            //OpManager.RegisterExplorer("OP_ZoneEntry", ExploreZoneEntry);
            //OpManager.RegisterExplorer("OP_NPCMoveUpdate", ExploreNPCMoveUpdate);
            //OpManager.RegisterExplorer("OP_MobUpdate", ExploreMobUpdate);
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

            if (Properties > 0)
                BodyType = Buffer.ReadUInt32();

            OutputStream.WriteLine("Bodytype = {0}", BodyType);

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

            Buffer.SkipBytes(18);

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

            float YPos = Utils.EQ19ToFloat((Int32)((Position4 >> 13) & 0x7FFFF));

            float ZPos = Utils.EQ19ToFloat((Int32)(Position5 >> 10) & 0x7FFFF);

            float XPos = Utils.EQ19ToFloat((Int32)(Position2 >> 10) & 0x7FFFF);

            //float Heading = Utils.EQ19ToFloat((Int32)(Position1 >> 13) & 0x3FF);
            //float Heading = Utils.EQ19ToFloat((Int32)(Position2) & 0x3FF);
            float Heading = Utils.EQ19ToFloat((Int32)(Position3) & 0x3FF);

            //for(int i = 0; i < 32; ++i)
            //   OutputStream.WriteLine("Pos3 << {0} = {1}", i, Utils.EQ19ToFloat((Int32)(Position3 >> i) & 0x3FF));

            OutputStream.WriteLine("(X,Y,Z) = {0}, {1}, {2}, Heading = {3}", XPos, YPos, ZPos, Heading);

            if ((OtherData & 16) > 1)
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