using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchDec072010Decoder : PatchOct202010Decoder
    {
        public PatchDec072010Decoder()
        {
            Version = "EQ Client Build Date December 7 2010.";

            PatchConfFileName = "patch_Dec7-2010.conf";

        }
        override public IdentificationStatus Identify(int OpCode, int Size, PacketDirection Direction)
        {
            if ((OpCode == OpManager.OpCodeNameToNumber("OP_ZoneEntry")) && (Direction == PacketDirection.ClientToServer))
                return IdentificationStatus.Tentative;

            if ((OpCode == OpManager.OpCodeNameToNumber("OP_SendAATable")) && (Direction == PacketDirection.ServerToClient) &&
                (Size == 120))
                return IdentificationStatus.Yes;

            return IdentificationStatus.No;
        }

        public override void RegisterExplorers()
        {
            //OpManager.RegisterExplorer("OP_ClientUpdate", ExploreClientUpdate);
        }

        public void ExploreClientUpdate(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction)
        {
            UInt16 SpawnID = Buffer.ReadUInt16();
            Buffer.SkipBytes(6);
            float x = Buffer.ReadSingle();
            float y = Buffer.ReadSingle();
            Buffer.SkipBytes(12);
            float z = Buffer.ReadSingle();

            Buffer.SkipBytes(4);
            UInt32 Temp = Buffer.ReadUInt32();
            Temp = Temp & 0x3FFFFF;
            Temp = Temp >> 10;
            float heading = Utils.EQ19ToFloat((Int32)(Temp));

            OutputStream.WriteLine("Loc: {0}, {1}, {2}  Heading: {3}", x, y, z, heading);

            OutputStream.WriteLine("");
        }
    }
}