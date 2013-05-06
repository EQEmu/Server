using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchMarch152011Decoder : PatchFeb082011Decoder
    {
        public PatchMarch152011Decoder()
        {
            Version = "EQ Client Build Date March 2011.";

            PatchConfFileName = "patch_March15-2011.conf";

            ExpectedPPLength = 28536;

            PPZoneIDOffset = 21204;
        }
    }
}