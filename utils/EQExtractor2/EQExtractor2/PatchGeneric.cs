using System;
using System.IO;
using System.Collections.Generic;
using EQExtractor2.InternalTypes;
using EQExtractor2.OpCodes;
using EQPacket;
using MyUtils;

namespace EQExtractor2.Patches
{
    public enum IdentificationStatus { No, Tentative, Yes };

    class PatchSpecficDecoder
    {
        protected class PacketToMatch
        {
	        public String	OPCodeName;
            public PacketDirection Direction;
	        public Int32	RequiredSize;
	        public bool	VersionMatched;
        };



        public PatchSpecficDecoder()
        {
            Version = "Unsupported Client Version";
            ExpectedPPLength = 0;
            PPZoneIDOffset = 0;
            PatchConfFileName = "";
            IDStatus = IdentificationStatus.No;
            SupportsSQLGeneration = true;
        }

        public string GetVersion()
        {
            return Version;
        }

        virtual public bool UnsupportedVersion()
        {
            return ExpectedPPLength == 0;
        }

        virtual public bool Init(string ConfDirectory, ref string ErrorMessage)
        {
            OpManager = new OpCodeManager();

            return false;
        }

        virtual public IdentificationStatus Identify(int OpCode, int Size, PacketDirection Direction)
        {
            return IdentificationStatus.No;
        }

        virtual public List<Door> GetDoors()
        {
            List<Door> DoorList = new List<Door>();

            return DoorList;
        }

        virtual public UInt16 GetZoneNumber()
        {
            return 0;
        }

        virtual public int VerifyPlayerProfile()
        {
            return 0;
        }

        virtual public MerchantManager GetMerchantData(NPCSpawnList NPCSL)
        {
            return null;
        }

        virtual public Item DecodeItemPacket(byte[] PacketBuffer)
        {
            Item NewItem = new Item();

            return NewItem;
        }


        virtual public List<ZonePoint> GetZonePointList()
        {
            List<ZonePoint> ZonePointList = new List<ZonePoint>();

            return ZonePointList;
        }

        virtual public NewZoneStruct GetZoneData()
        {
            NewZoneStruct NewZone = new NewZoneStruct();

            return NewZone;
        }

        virtual public List<ZoneEntryStruct> GetSpawns()
        {
            List<ZoneEntryStruct> ZoneSpawns = new List<ZoneEntryStruct>();

            return ZoneSpawns;
        }

        virtual public List<PositionUpdate> GetHighResolutionMovementUpdates()
        {
            List<PositionUpdate> Updates = new List<PositionUpdate>();

            return Updates;
        }

        virtual public List<PositionUpdate> GetLowResolutionMovementUpdates()
        {
            List<PositionUpdate> Updates = new List<PositionUpdate>();

            return Updates;
        }

        virtual public List<PositionUpdate> GetAllMovementUpdates()
        {
            List<PositionUpdate> Updates = new List<PositionUpdate>();

            return Updates;
        }

        virtual public  PositionUpdate Decode_OP_NPCMoveUpdate(byte[] UpdatePacket)
        {
            PositionUpdate PosUpdate = new PositionUpdate();

            return PosUpdate;
        }

        virtual public PositionUpdate Decode_OP_MobUpdate(byte[] MobUpdatePacket)
        {
            PositionUpdate PosUpdate = new PositionUpdate();

            return PosUpdate;
        }

        virtual public List<PositionUpdate> GetClientMovementUpdates()
        {
            List<PositionUpdate> Updates = new List<PositionUpdate>();

            return Updates;
        }

        virtual public List<GroundSpawnStruct> GetGroundSpawns()
        {
            List<GroundSpawnStruct> GroundSpawns = new List<GroundSpawnStruct>();

            return GroundSpawns;
        }

        virtual public List<UInt32> GetFindableSpawns()
        {
            List<UInt32> FindableSpawnList = new List<UInt32>();

            return FindableSpawnList;
        }

        virtual public string GetZoneName()
        {
            return "";
        }

        virtual public bool DumpAAs(string FileName)
        {
            return false;
        }

        public void GivePackets(PacketManager pm)
        {
            Packets = pm;
        }

        virtual public void RegisterExplorers()
        {
        }

        public List<byte[]> GetPacketsOfType(string OpCodeName, PacketDirection Direction)
        {
            List<byte[]> ReturnList = new List<byte[]>();

            if (OpManager == null)
                return ReturnList;

            UInt32 OpCodeNumber = OpManager.OpCodeNameToNumber(OpCodeName);

            foreach (EQApplicationPacket app in Packets.PacketList)
            {
                if ((app.OpCode == OpCodeNumber) && (app.Direction == Direction) && (app.Locked))
                    ReturnList.Add(app.Buffer);
        }

            return ReturnList;
        }

        public DateTime GetCaptureStartTime()
        {
            if (Packets.PacketList.Count > 0)
            {
                return Packets.PacketList[0].PacketTime;
            }
            return DateTime.MinValue;
        }

        public bool DumpPackets(string FileName, bool ShowTimeStamps)
        {

            StreamWriter PacketDumpStream;

            try
            {
                PacketDumpStream = new StreamWriter(FileName);
            }
            catch
            {
                return false;
            }

            string Direction = "";

            foreach (EQApplicationPacket p in Packets.PacketList)
            {
                if(ShowTimeStamps)
                    PacketDumpStream.WriteLine(p.PacketTime.ToString());

                if (p.Direction == PacketDirection.ServerToClient)
                    Direction = "[Server->Client]";
                else
                    Direction = "[Client->Server]";

                OpCode oc = OpManager.GetOpCodeByNumber(p.OpCode);

                string OpCodeName = (oc != null) ? oc.Name : "OP_Unknown";

                PacketDumpStream.WriteLine("[OPCode: 0x" + p.OpCode.ToString("x4") + "] " + OpCodeName + " " + Direction + " [Size: " + p.Buffer.Length + "]");
                PacketDumpStream.WriteLine(Utils.HexDump(p.Buffer));

                if ((oc != null) && (oc.Explorer != null))
                    oc.Explorer(PacketDumpStream, new ByteStream(p.Buffer), p.Direction);
            }

            PacketDumpStream.Close();

            return true;
        }
        public int PacketTypeCountByName(string OPCodeName)
        {
            UInt32 OpCodeNumber = OpManager.OpCodeNameToNumber(OPCodeName);

            int Count = 0;

            foreach (EQApplicationPacket app in Packets.PacketList)
            {
                if (app.OpCode == OpCodeNumber)
                    ++Count;
            }


            return Count;
        }

        protected void AddExplorerSpawn(UInt32 ID, string Name)
        {
            ExplorerSpawnRecord e = new ExplorerSpawnRecord(ID, Name);

            ExplorerSpawns.Add(e);
        }

        protected string FindExplorerSpawn(UInt32 ID)
        {
            foreach(ExplorerSpawnRecord s in ExplorerSpawns)
            {
                if (s.ID == ID)
                    return s.Name;
            }

            return "";
        }

        protected PacketManager Packets;

        public OpCodeManager OpManager;

        protected string Version;

        protected int ExpectedPPLength;

        protected int PPZoneIDOffset;

        protected string PatchConfFileName;

        protected PacketToMatch[] PacketsToMatch;

        protected UInt32 WaitingForPacket;

        protected IdentificationStatus IDStatus;

        private List<ExplorerSpawnRecord> ExplorerSpawns = new List<ExplorerSpawnRecord>();

        public bool SupportsSQLGeneration;
    }
}
