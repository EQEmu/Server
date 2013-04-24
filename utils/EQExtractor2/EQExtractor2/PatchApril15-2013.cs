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

            SupportsSQLGeneration = false;
        }       
    }
}