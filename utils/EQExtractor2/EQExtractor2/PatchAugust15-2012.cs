using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchAugust152012Decoder : PatchJuly132012Decoder
    {
        public PatchAugust152012Decoder()
        {
            Version = "EQ Client Build Date August 15 2012.";

            ExpectedPPLength = 33904;

            PPZoneIDOffset = 26572;

            PatchConfFileName = "patch_August15-2012.conf";
        }

        override public Item DecodeItemPacket(byte[] PacketBuffer)
        {
            ByteStream Buffer = new ByteStream(PacketBuffer);

            Item NewItem = new Item();

            Buffer.SetPosition(30);
            NewItem.MerchantSlot = Buffer.ReadByte();
            NewItem.Price = Buffer.ReadUInt32();
            Buffer.SkipBytes(5);
            NewItem.Quantity = Buffer.ReadInt32();
            Buffer.SetPosition(109);
            // 109
            NewItem.Name = Buffer.ReadString(true);
            NewItem.Lore = Buffer.ReadString(true);
            NewItem.IDFile = Buffer.ReadString(true);
            Buffer.SkipBytes(1);
            NewItem.ID = Buffer.ReadUInt32();

            return NewItem;
        }
    }
}