using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchJuly132012Decoder : PatchJune252012Decoder
    {
        public PatchJuly132012Decoder()
        {
            Version = "EQ Client Build Date July 13 2012.";

            ExpectedPPLength = 33784;

            PPZoneIDOffset = 26452;

            PatchConfFileName = "patch_July13-2012.conf";
        }
    }
}