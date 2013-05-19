//
// Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net). Distributed under GPL version 2.
//
//

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using SharpPcap;
using EQApplicationLayer;

namespace EQExtractor2
{
    public partial class EQExtractor2Form1 : Form
    {
        string Version = "EQExtractor2 Version 2.6.4 GIT";

        static int PacketsSeen = 0;
        static long BytesRead = 0;
        static long CaptureFileSize = 0;
        string ZoneName;
        string SpawnNameFilter = "";
        bool CoalesceWaypoints = true;

        GenerateSQLForm SQLForm = new GenerateSQLForm();
        LogForm DebugLog = new LogForm();
        UserOptions Options = new UserOptions();

        StreamWriter SQLStream;
        StreamWriter PacketDebugStream;

        EQStreamProcessor StreamProcessor;

        public EQExtractor2Form1()
        {
            InitializeComponent();
            DisplayUsageInfo();

            Options.PacketDumpViewerProgram.Text = Properties.Settings.Default.TextFileViewer;
            Options.ShowDebugWindowOnStartup.Checked = Properties.Settings.Default.ShowDebugWindowOnStartup;
            Options.ShowTimeStamps.Checked = Properties.Settings.Default.DumpTimeStamps;

        }

        public void Log(string Message)
        {
            DebugLog.ConsoleWindow.Items.Add(Message);
            DebugLog.ConsoleWindow.SelectedIndex = DebugLog.ConsoleWindow.Items.Count - 1;
            Application.DoEvents();
        }

        private void device_OnPacketArrival(object sender, SharpPcap.CaptureEventArgs e)
        {
            if (e.Packet.LinkLayerType == PacketDotNet.LinkLayers.Ethernet)
            {
                PacketDotNet.Packet packet;

                long TotalPacketSize = e.Packet.Data.Length;
                BytesRead += TotalPacketSize;
                ++PacketsSeen;

                if ((PacketsSeen > 0) && ((PacketsSeen % 10000) == 0))
                {
                    DebugLog.ConsoleWindow.SelectedIndex = DebugLog.ConsoleWindow.Items.Count - 1;
                    int Progress = (int)((float)BytesRead / (float)CaptureFileSize * 100);
                    ProgressBar.Value = Progress;

                    Application.DoEvents();
                }

                try
                {
                    packet = PacketDotNet.Packet.ParsePacket(e.Packet);
                }
                catch
                {
                    return;
                }

                var ethernetPacket = (PacketDotNet.EthernetPacket)packet;

                var udpPacket = PacketDotNet.UdpPacket.GetEncapsulated(packet);

                if (udpPacket != null)
                {
                    var ipPacket = (PacketDotNet.IpPacket)udpPacket.ParentPacket;
                    System.Net.IPAddress srcIp = ipPacket.SourceAddress;
                    System.Net.IPAddress dstIp = ipPacket.DestinationAddress;

                    byte[] Payload = udpPacket.PayloadData;

                    Int32 l = udpPacket.Length - udpPacket.Header.GetLength(0);

                    if (l > 0)
                    {
                        Array.Resize(ref Payload, l);

                        StreamProcessor.ProcessPacket(srcIp, dstIp, udpPacket.SourcePort, udpPacket.DestinationPort, Payload, packet.Timeval.Date);
                    }
                }
            }
        }

        public void WriteSQL(string Message)
        {
            SQLStream.WriteLine(Message);
         }

        public void PacketDebugLogger(string Message)
        {
            PacketDebugStream.WriteLine(Message);
        }

        private void DisableAllControls()
        {
            foreach (Control c in this.Controls)
            {
                if ((c is Button) || (c is TextBox) || (c is MaskedTextBox) || (c is CheckBox))
                    c.Enabled = false;
            }

        }

        private void EnableAllControls()
        {
            foreach (Control c in this.Controls)
                c.Enabled = true;

            menuGenerateSQL.Enabled = StreamProcessor.StreamRecognised() && StreamProcessor.SupportsSQLGeneration();
            menuDumpAAs.Enabled = StreamProcessor.StreamRecognised();
        }

        private void menuLoadPCAP_Click(object sender, EventArgs e)
        {
            if (InputFileOpenDialog.ShowDialog() != DialogResult.OK)
                return;

            menuGenerateSQL.Enabled = false;
            menuPacketDump.Enabled = false;
            menuViewPackets.Enabled = false;
            menuDumpAAs.Enabled = false;

            SharpPcap.OfflinePcapDevice device;

            try
            {
                string CapFile = InputFileOpenDialog.FileName;

                device = new SharpPcap.OfflinePcapDevice(CapFile);

                device.Open();
            }
            catch
            {
                StatusBar.Text = "Error: File does not exist or not in .pcap format.";
                Log("Error: File does not exist or not in .pcap format.");
                return;
            }

            StreamProcessor = new EQStreamProcessor();

            if (!StreamProcessor.Init(Application.StartupPath, this.Log))
            {
                Log("Fatal error initialising Stream Processor. No decoders could be initialised (mostly likely misplaced patch_XXXX.conf files.");
                StatusBar.Text = "Fatal error initialising Stream Processor. No decoders could be initialised (mostly likely misplaced patch_XXXX.conf files.";
                return;
            }

            if (Options.EQPacketDebugFilename.Text.Length > 0)
            {
                try
                {
                    PacketDebugStream = new StreamWriter(Options.EQPacketDebugFilename.Text);
                    StreamProcessor.Packets.SetDebugLogHandler(PacketDebugLogger);
                }
                catch
                {
                    Log("Failed to open netcode debug file for writing.");
                    Options.EQPacketDebugFilename.Text = "";
                    StreamProcessor.Packets.SetDebugLogHandler(null);
                }
            }
            else
                StreamProcessor.Packets.SetDebugLogHandler(null);

            StatusBar.Text = "Reading packets from " + InputFileOpenDialog.FileName + ". Please wait...";

            device.OnPacketArrival +=
                new PacketArrivalEventHandler(device_OnPacketArrival);

            BytesRead = 0;
            PacketsSeen = 0;

            DebugLog.ConsoleWindow.Items.Add("-- Capturing from '" + InputFileOpenDialog.FileName);
            ProgressBar.Value = 0;
            ProgressBar.Show();

            menuFile.Enabled = false;

            CaptureFileSize = device.FileSize;

            device.Capture();

            device.Close();

            Log("End of file reached. Processed " + PacketsSeen + " packets and " + BytesRead + " bytes.");

            ProgressBar.Hide();

            if (Options.EQPacketDebugFilename.Text.Length > 0)
                PacketDebugStream.Close();

            PacketCountLabel.Text = PacketsSeen.ToString();
            if (StreamProcessor.Packets.ErrorsInStream)
                Log("There were errors encountered in the packet stream. Data may be incomplete.");

            DebugLog.ConsoleWindow.SelectedIndex = DebugLog.ConsoleWindow.Items.Count - 1;

            menuFile.Enabled = true;

            StreamProcessor.PCAPFileReadFinished();

            menuPacketDump.Enabled = true;

            menuViewPackets.Enabled = true;

            Log("Stream recognised as " + StreamProcessor.GetDecoderVersion());

            int PPLength = StreamProcessor.VerifyPlayerProfile();

            ClientVersionLabel.Text = StreamProcessor.GetDecoderVersion();

            if (PPLength == 0)
            {
                Log("Unable to find player profile packet, or packet not of correct size.");
                menuDumpAAs.Enabled = false;
                menuGenerateSQL.Enabled = false;
                ClientVersionLabel.ForeColor = Color.Red;
                ZoneLabel.Text = "";
                PacketCountLabel.Text = "";
                StatusBar.Text = "Unrecognised EQ Client Version. Press Ctrl-P to dump, or Ctrl-V to view packets.";
                return;
            }
            else
            {
                ClientVersionLabel.ForeColor = Color.Green;
                Log("Found player profile packet of the expected length (" + PPLength + ").");

                if(StreamProcessor.SupportsSQLGeneration())
                    StatusBar.Text = "Client version recognised. Press Ctrl-S to Generate SQL";
                else
                    StatusBar.Text = "Client version recognised. *SQL GENERATION NOT SUPPORTED FOR THIS CLIENT*";
            }

            ZoneName = StreamProcessor.GetZoneName();

            UInt32 ZoneNumber = StreamProcessor.GetZoneNumber();

            Log("Zonename is " + StreamProcessor.GetZoneName());

            Log("Zone number is " + ZoneNumber);

            ZoneLabel.Text = StreamProcessor.GetZoneLongName() + " [" + StreamProcessor.GetZoneName() + "] (" + ZoneNumber.ToString() + ")";

            SQLForm.ZoneIDTextBox.Text = ZoneNumber.ToString();
            SQLForm.ZoneIDTextBox.Enabled = true;
            SQLForm.DoorsTextBox.Enabled = true;
            SQLForm.NPCTypesTextBox.Enabled = true;
            SQLForm.SpawnEntryTextBox.Enabled = true;
            SQLForm.SpawnGroupTextBox.Enabled = true;
            SQLForm.Spawn2TextBox.Enabled = true;
            SQLForm.GridTextBox.Enabled = true;
            SQLForm.ObjectTextBox.Enabled = true;
            SQLForm.GroundSpawnTextBox.Enabled = true;
            SQLForm.MerchantTextBox.Enabled = true;
            SQLForm.VersionSelector.Enabled = true;
            menuGenerateSQL.Enabled = StreamProcessor.SupportsSQLGeneration();
            menuPacketDump.Enabled = true;
            menuViewPackets.Enabled = true;
            menuDumpAAs.Enabled = true;

            SQLForm.RecalculateBaseInsertIDs();

            StreamProcessor.GenerateZonePointList();
        }

        private void menuGenerateSQL_Click(object sender, EventArgs e)
        {
            if (SQLForm.ShowDialog() != DialogResult.OK)
                return;

            string SQLFile = SQLForm.FileName;

            try
            {
                SQLStream = new StreamWriter(SQLFile);
            }
            catch
            {
                Log("Unable to open file " + SQLFile + " for writing.");
                StatusBar.Text = "Unable to open file " + SQLFile + " for writing.";
                return;
            }

            UInt32 SpawnDBID = Convert.ToUInt32(SQLForm.NPCTypesTextBox.Text);
            UInt32 SpawnGroupID = Convert.ToUInt32(SQLForm.SpawnGroupTextBox.Text);
            UInt32 SpawnEntryID = Convert.ToUInt32(SQLForm.SpawnEntryTextBox.Text);
            UInt32 Spawn2ID = Convert.ToUInt32(SQLForm.Spawn2TextBox.Text);
            UInt32 GridDBID = Convert.ToUInt32(SQLForm.GridTextBox.Text);
            UInt32 MerchantDBID = Convert.ToUInt32(SQLForm.MerchantTextBox.Text);
            int DoorDBID = Convert.ToInt32(SQLForm.DoorsTextBox.Text);
            UInt32 GroundSpawnDBID = Convert.ToUInt32(SQLForm.GroundSpawnTextBox.Text);
            UInt32 ObjectDBID = Convert.ToUInt32(SQLForm.ObjectTextBox.Text);

            UInt32 ZoneID = Convert.ToUInt32(SQLForm.ZoneIDTextBox.Text);

            SpawnNameFilter = SQLForm.SpawnNameFilter.Text;
            CoalesceWaypoints = SQLForm.CoalesceWaypoints.Checked;

            WriteSQL("-- SQL created by " + Version);
            WriteSQL("--");
            WriteSQL("-- Using Decoder: " + StreamProcessor.GetDecoderVersion());
            WriteSQL("--");
            WriteSQL("-- Packets captured on " + StreamProcessor.GetCaptureStartTime().ToString());
            WriteSQL("--");
            WriteSQL("-- Change these variables if required");
            WriteSQL("--");
            WriteSQL("set @StartingNPCTypeID = " + SpawnDBID + ";");
            WriteSQL("set @StartingSpawnGroupID = " + SpawnGroupID + ";");
            WriteSQL("set @StartingSpawnEntryID = " + SpawnEntryID + ";");
            WriteSQL("set @StartingSpawn2ID = " + Spawn2ID + ";");
            WriteSQL("set @StartingGridID = " + GridDBID + ";");
            WriteSQL("set @StartingMerchantID = " + MerchantDBID + ";");
            WriteSQL("set @BaseDoorID = " + DoorDBID + ";");
            WriteSQL("set @StartingGroundSpawnID = " + GroundSpawnDBID + ";");
            WriteSQL("set @StartingObjectID = " + ObjectDBID + ";");
            WriteSQL("--");
            WriteSQL("--");

            if (SQLForm.ZoneCheckBox.Checked)
                StreamProcessor.GenerateZoneSQL(this.WriteSQL);

            if (SQLForm.ZonePointCheckBox.Checked)
                StreamProcessor.GenerateZonePointSQL(ZoneName, this.WriteSQL);

            UInt32 SpawnVersion = (UInt32)SQLForm.VersionSelector.Value;

            if (SQLForm.DoorCheckBox.Checked)
            {
                Log("Starting to generate SQL for Doors.");
                StreamProcessor.GenerateDoorsSQL(ZoneName, DoorDBID, SpawnVersion, this.WriteSQL);
                Log("Finished generating SQL for Doors.");
            }

            Log("Starting to generate SQL for Spawns and/or Grids.");

            StreamProcessor.GenerateSpawnSQL(SQLForm.SpawnCheckBox.Checked, SQLForm.GridCheckBox.Checked, SQLForm.MerchantCheckBox.Checked, ZoneName, ZoneID, SpawnVersion, SQLForm.UpdateExistingNPCTypesCheckbox.Checked, SQLForm.NPCTypesTintCheckBox.Checked, SpawnNameFilter, CoalesceWaypoints, SQLForm.InvisibleMenCheckBox.Checked, this.WriteSQL);

            Log("Finished generating SQL for Spawns and/or Grids.");

            if (SQLForm.GroundSpawnCheckBox.Checked || SQLForm.ObjectCheckBox.Checked)
            {
                Log("Starting to generate SQL for Ground Spawns and/or Objects.");

                StreamProcessor.GenerateObjectSQL(SQLForm.GroundSpawnCheckBox.Checked, SQLForm.ObjectCheckBox.Checked, SpawnVersion, this.WriteSQL);

                Log("Finished generating SQL for Ground Spawns and/or Objects.");
            }

            StatusBar.Text = "SQL written to " + SQLFile;
            SQLStream.Close();
        }

        private void menuPacketDump_Click(object sender, EventArgs e)
        {
            if (PacketDumpFileDialog.ShowDialog() == DialogResult.OK)
            {
                StatusBar.Text = "Packet dump in progress. Please wait...";
                Log("Packets dump in progress...");

                DisableAllControls();

                Application.DoEvents();

                if (StreamProcessor.DumpPackets(PacketDumpFileDialog.FileName, Properties.Settings.Default.DumpTimeStamps))
                {
                    StatusBar.Text = "Packets dumped successfully.";
                    Log("Packets dumped successfully.");
                }
                else
                {
                    StatusBar.Text = "Packet dump failed.";
                    Log("Packet dump failed.");
                }

                EnableAllControls();
            }
        }

        private void menuDumpAAs_Click(object sender, EventArgs e)
        {
            if (PacketDumpFileDialog.ShowDialog() == DialogResult.OK)
            {
                Log("AA dump in progress...");

                DisableAllControls();

                if (StreamProcessor.DumpAAs(PacketDumpFileDialog.FileName))
                {
                    StatusBar.Text = "AAs dumped successfully.";
                    Log("AAs dumped successfully.");
                }
                else
                {
                    StatusBar.Text = "AA dumped failed.";
                    Log("AA dump failed.");
                }
                EnableAllControls();
            }
        }

        private void menuExit_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void menuViewDebugLog_Click(object sender, EventArgs e)
        {
            menuViewDebugLog.Checked = DebugLog.Visible;

            if (!menuViewDebugLog.Checked)
            {
                menuViewDebugLog.Checked = true;
                ShowDebugLog();

            }
            else
            {
                menuViewDebugLog.Checked = false;
                DebugLog.Hide();
            }
        }

        private void menuViewPackets_Click(object sender, EventArgs e)
        {
            DisableAllControls();

            Application.DoEvents();

            string TextFileViewer = Properties.Settings.Default.TextFileViewer;

            string TempFileName = Path.GetTempFileName();

            if (StreamProcessor.DumpPackets(TempFileName, Properties.Settings.Default.DumpTimeStamps))
            {
                try
                {
                    System.Diagnostics.Process.Start(TextFileViewer, TempFileName);
                }
                catch
                {
                    StatusBar.Text = "Unable to launch " + TextFileViewer;
                }
            }
            else
            {
                StatusBar.Text = "Unexpected error while generating temporary file.";
            }
            EnableAllControls();
        }

        private void EQExtractor2Form1_Load(object sender, EventArgs e)
        {
            if (Properties.Settings.Default.ShowDebugWindowOnStartup)
            {
                ShowDebugLog();

            }
        }

        private void DisplayUsageInfo()
        {
#if DEBUG
            Version += " (Debug Build)";
#else
            Version += " (Release Build)";
#endif
            Text = Version;

            Log(Version + " Initialised.");

            Log("");
            Log("Instructions:");
            Log("Generate a .pcap file using Wireshark. To do this, park a character in the zone you want to collect in.");
            Log("Camp to character select. Start Wireshark capturing. Zone your character in and just sit around for a");
            Log("while, or go and inspect merchant inventories if you want to collect those. When finished, stop the");
            Log("Wireshark capture and save it (File/Save As).");
            Log("");
            Log("Load the .pcap file into this program by pressing Ctrl-L.");
            Log("To generate SQL, press Ctrl-S and select the check boxes and set the starting SQL INSERT IDs as required.");
            Log("Review the generated SQL before sourcing as DELETEs are auto-generated.");
            Log("Press Ctrl-V to view packets, or Ctrl-D to dump them to a text file.");
            Log("");
        }

        private void menuOptions_Click(object sender, EventArgs e)
        {
            DialogResult d = Options.ShowDialog();

            if (d == DialogResult.OK)
            {
                Properties.Settings.Default.TextFileViewer = Options.PacketDumpViewerProgram.Text;
                Properties.Settings.Default.ShowDebugWindowOnStartup = Options.ShowDebugWindowOnStartup.Checked;
                Properties.Settings.Default.DumpTimeStamps = Options.ShowTimeStamps.Checked;
                Properties.Settings.Default.Save();

                if (Properties.Settings.Default.ShowDebugWindowOnStartup)
                {
                    if (!DebugLog.Visible)
                        ShowDebugLog();
                }
                else
                {
                    if (DebugLog.Visible)
                    {
                        DebugLog.Hide();
                        menuViewDebugLog.Checked = false;
                    }
                }
            }
            else
            {
                Options.PacketDumpViewerProgram.Text = Properties.Settings.Default.TextFileViewer;
                Options.ShowDebugWindowOnStartup.Checked = Properties.Settings.Default.ShowDebugWindowOnStartup;
                Options.ShowTimeStamps.Checked = Properties.Settings.Default.DumpTimeStamps;
            }
        }

        private void ShowDebugLog()
        {
            DebugLog.Left = this.Location.X;
            DebugLog.Top = this.Location.Y + this.Height;
            DebugLog.Show();
            menuViewDebugLog.Checked = true;
            this.Focus();
        }

        private void menuView_Popup(object sender, EventArgs e)
        {
            menuViewDebugLog.Checked = DebugLog.Visible;
        }
    }
}

