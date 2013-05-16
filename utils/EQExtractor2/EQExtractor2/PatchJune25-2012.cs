using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    class PatchJune252012Decoder : PatchMar152012Decoder
    {
        public PatchJune252012Decoder()
        {
            Version = "EQ Client Build Date June 25 2012.";

            PatchConfFileName = "patch_June25-2012.conf";
        }
    }
}