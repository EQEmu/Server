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
    class PatchFebruary112013Decoder : PatchJanuary162013Decoder
    {
        public PatchFebruary112013Decoder()
        {
            Version = "EQ Client Build Date February 11 2013.";
                                   
            PatchConfFileName = "patch_Feb11-2013.conf";

            SupportsSQLGeneration = false;
        }        
    }
}