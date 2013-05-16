//
// Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net). Distributed under GPL version 2.
//
//

using System;
using System.IO;
using System.Collections.Generic;
using EQPacket;
using MyUtils;

namespace EQExtractor2.OpCodes
{
    public class OpCode
    {
        public UInt32 Value;
        public string Name;
        public ExplorerMethod Explorer;
        public UInt32 Count;

        public OpCode(string inName, UInt32 inValue)
        {
            Name = inName;
            Value = inValue;
            Explorer = null;
            Count = 0;

        }
    }

    public delegate void ExplorerMethod(StreamWriter OutputStream, ByteStream Buffer, PacketDirection Direction);

    public class OpCodeManager
    {
        public List<OpCode> OpCodeList = new List<OpCode>();

        public bool Init(string ConfFile, ref string ErrorMessage)
        {
            StreamReader sr;

            string PatchFile = ConfFile;

            try
            {
                sr = new StreamReader(PatchFile);
            }
            catch
            {
                ErrorMessage = "Unable to open opcode file " + PatchFile;
                return false;
            }

            string Line;

            while ((Line = sr.ReadLine()) != null)
            {
                if (Line.Length == 0)
                    continue;
                if (Line[0] == '#')
                    continue;

                int EqualsSign = Line.IndexOf("=");

                if (EqualsSign < 0)
                    continue;
                string OPCodeName = Line.Substring(0, EqualsSign);
                string OPCodeValue = Line.Substring(EqualsSign + 1, 6);
                UInt32 OPCodeNumber = 0;
                try
                {
                    OPCodeNumber = Convert.ToUInt32(OPCodeValue, 16);
                }
                catch
                {
                    ErrorMessage = "Malformed OPCode value for " + OPCodeName;
                    OPCodeNumber = 0;
                }

                if (OPCodeNumber > 0)
                    AddOpCode(OPCodeName, OPCodeNumber);
            }

            return true;
        }

        public void AddOpCode(string OpCodeName, UInt32 OpCodeValue)
        {
            OpCode NewOpCode = new OpCode(OpCodeName, OpCodeValue);
            OpCodeList.Add(NewOpCode);

        }

        public string OpCodeToName(int OpCodeValue)
        {
            foreach (OpCode oc in OpCodeList)
            {
                if (oc.Value == OpCodeValue)
                    return oc.Name;
            }

            return "OP_Unknown";
        }

        public OpCode GetOpCodeByNumber(int OpCodeValue)
        {
            foreach (OpCode oc in OpCodeList)
            {
                if (oc.Value == OpCodeValue)
                    return oc;
            }

            return null;
        }

        public UInt32 OpCodeNameToNumber(string Name)
        {
            foreach (OpCode oc in OpCodeList)
            {
                if (oc.Name == Name)
                    return oc.Value;
            }
            return 0;
        }

        public void ResetCounts()
        {
            foreach (OpCode oc in OpCodeList)
                oc.Count = 0;
        }

        public void RegisterExplorer(string Name, ExplorerMethod Explorer)
        {
            foreach (OpCode oc in OpCodeList)
            {
                if (oc.Name == Name)
                {
                    oc.Explorer = Explorer;
                    return;
                }
            }
        }

        public void UnRegisterExplorer(string Name)
        {
            // Designed to be used when a decoder wants to use some of it's base classes explorers, but some need to be UnRegistered
            // as they are incompatible with the current patch version.
            // i.e.
            // base.RegisterExplorers();
            // UnRegisterExplorer("OP_NotCompatible");
            //
            foreach (OpCode oc in OpCodeList)
            {
                if (oc.Name == Name)
                {
                    oc.Explorer = null;
                    return;
                }
            }
        }
    }
}
