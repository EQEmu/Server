namespace EQExtractor2
{
    partial class EQExtractor2Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.InputFileOpenDialog = new System.Windows.Forms.OpenFileDialog();
            this.PacketDumpFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.ProgressBar = new System.Windows.Forms.ProgressBar();
            this.mainMenu1 = new System.Windows.Forms.MainMenu(this.components);
            this.menuFile = new System.Windows.Forms.MenuItem();
            this.menuLoadPCAP = new System.Windows.Forms.MenuItem();
            this.menuGenerateSQL = new System.Windows.Forms.MenuItem();
            this.menuPacketDump = new System.Windows.Forms.MenuItem();
            this.menuDumpAAs = new System.Windows.Forms.MenuItem();
            this.menuViewPackets = new System.Windows.Forms.MenuItem();
            this.menuExit = new System.Windows.Forms.MenuItem();
            this.menuView = new System.Windows.Forms.MenuItem();
            this.menuViewDebugLog = new System.Windows.Forms.MenuItem();
            this.menuOptions = new System.Windows.Forms.MenuItem();
            this.label1 = new System.Windows.Forms.Label();
            this.ClientVersionLabel = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.PacketCountLabel = new System.Windows.Forms.Label();
            this.ZoneLabelText = new System.Windows.Forms.Label();
            this.ZoneLabel = new System.Windows.Forms.Label();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.StatusBar = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            //
            // InputFileOpenDialog
            //
            this.InputFileOpenDialog.Filter = "Capture Files (*.pcap)|*.pcap|All files (*.*)|*.*";
            //
            // PacketDumpFileDialog
            //
            this.PacketDumpFileDialog.Filter = "Text Files (*.txt)|*.txt|All files (*.*)|*.*";
            this.PacketDumpFileDialog.Title = "Dump Packets To File";
            //
            // ProgressBar
            //
            this.ProgressBar.Location = new System.Drawing.Point(2, 78);
            this.ProgressBar.Name = "ProgressBar";
            this.ProgressBar.Size = new System.Drawing.Size(627, 23);
            this.ProgressBar.TabIndex = 35;
            this.ProgressBar.Visible = false;
            //
            // mainMenu1
            //
            this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuFile,
            this.menuView});
            //
            // menuFile
            //
            this.menuFile.Index = 0;
            this.menuFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuLoadPCAP,
            this.menuGenerateSQL,
            this.menuPacketDump,
            this.menuDumpAAs,
            this.menuViewPackets,
            this.menuExit});
            this.menuFile.Text = "File";
            //
            // menuLoadPCAP
            //
            this.menuLoadPCAP.Index = 0;
            this.menuLoadPCAP.Shortcut = System.Windows.Forms.Shortcut.CtrlL;
            this.menuLoadPCAP.Text = "Load PCAP";
            this.menuLoadPCAP.Click += new System.EventHandler(this.menuLoadPCAP_Click);
            //
            // menuGenerateSQL
            //
            this.menuGenerateSQL.Enabled = false;
            this.menuGenerateSQL.Index = 1;
            this.menuGenerateSQL.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
            this.menuGenerateSQL.Text = "Generate SQL";
            this.menuGenerateSQL.Click += new System.EventHandler(this.menuGenerateSQL_Click);
            //
            // menuPacketDump
            //
            this.menuPacketDump.Enabled = false;
            this.menuPacketDump.Index = 2;
            this.menuPacketDump.Shortcut = System.Windows.Forms.Shortcut.CtrlP;
            this.menuPacketDump.Text = "Packet Dump";
            this.menuPacketDump.Click += new System.EventHandler(this.menuPacketDump_Click);
            //
            // menuDumpAAs
            //
            this.menuDumpAAs.Enabled = false;
            this.menuDumpAAs.Index = 3;
            this.menuDumpAAs.Shortcut = System.Windows.Forms.Shortcut.CtrlA;
            this.menuDumpAAs.Text = "Dump AAs";
            this.menuDumpAAs.Click += new System.EventHandler(this.menuDumpAAs_Click);
            //
            // menuViewPackets
            //
            this.menuViewPackets.Enabled = false;
            this.menuViewPackets.Index = 4;
            this.menuViewPackets.Shortcut = System.Windows.Forms.Shortcut.CtrlV;
            this.menuViewPackets.Text = "View Packets";
            this.menuViewPackets.Click += new System.EventHandler(this.menuViewPackets_Click);
            //
            // menuExit
            //
            this.menuExit.Index = 5;
            this.menuExit.Text = "Exit";
            this.menuExit.Click += new System.EventHandler(this.menuExit_Click);
            //
            // menuView
            //
            this.menuView.Index = 1;
            this.menuView.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuViewDebugLog,
            this.menuOptions});
            this.menuView.Text = "View";
            this.menuView.Popup += new System.EventHandler(this.menuView_Popup);
            //
            // menuViewDebugLog
            //
            this.menuViewDebugLog.Index = 0;
            this.menuViewDebugLog.Shortcut = System.Windows.Forms.Shortcut.CtrlD;
            this.menuViewDebugLog.Text = "Debug Log";
            this.menuViewDebugLog.Click += new System.EventHandler(this.menuViewDebugLog_Click);
            //
            // menuOptions
            //
            this.menuOptions.Index = 1;
            this.menuOptions.Shortcut = System.Windows.Forms.Shortcut.CtrlO;
            this.menuOptions.Text = "Options";
            this.menuOptions.Click += new System.EventHandler(this.menuOptions_Click);
            //
            // label1
            //
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(92, 13);
            this.label1.TabIndex = 36;
            this.label1.Text = "EQ Client Version:";
            //
            // ClientVersionLabel
            //
            this.ClientVersionLabel.AutoSize = true;
            this.ClientVersionLabel.Location = new System.Drawing.Point(116, 9);
            this.ClientVersionLabel.Name = "ClientVersionLabel";
            this.ClientVersionLabel.Size = new System.Drawing.Size(166, 13);
            this.ClientVersionLabel.TabIndex = 37;
            this.ClientVersionLabel.Text = ".PCAP file not loaded. Press Ctrl-L";
            //
            // label2
            //
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(55, 37);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(49, 13);
            this.label2.TabIndex = 38;
            this.label2.Text = "Packets:";
            //
            // PacketCountLabel
            //
            this.PacketCountLabel.AutoSize = true;
            this.PacketCountLabel.Location = new System.Drawing.Point(116, 37);
            this.PacketCountLabel.Name = "PacketCountLabel";
            this.PacketCountLabel.Size = new System.Drawing.Size(145, 13);
            this.PacketCountLabel.TabIndex = 39;
            this.PacketCountLabel.Text = "                                              ";
            //
            // ZoneLabelText
            //
            this.ZoneLabelText.AutoSize = true;
            this.ZoneLabelText.Location = new System.Drawing.Point(72, 62);
            this.ZoneLabelText.Name = "ZoneLabelText";
            this.ZoneLabelText.Size = new System.Drawing.Size(32, 13);
            this.ZoneLabelText.TabIndex = 40;
            this.ZoneLabelText.Text = "Zone";
            //
            // ZoneLabel
            //
            this.ZoneLabel.AutoSize = true;
            this.ZoneLabel.Location = new System.Drawing.Point(116, 62);
            this.ZoneLabel.Name = "ZoneLabel";
            this.ZoneLabel.Size = new System.Drawing.Size(145, 13);
            this.ZoneLabel.TabIndex = 41;
            this.ZoneLabel.Text = "                                              ";
            //
            // statusStrip1
            //
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.StatusBar});
            this.statusStrip1.Location = new System.Drawing.Point(0, 104);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(633, 22);
            this.statusStrip1.TabIndex = 42;
            this.statusStrip1.Text = "statusStrip1";
            //
            // StatusBar
            //
            this.StatusBar.Name = "StatusBar";
            this.StatusBar.Size = new System.Drawing.Size(209, 17);
            this.StatusBar.Text = "Ready. Press Ctrl-L to load a .PCAP file";
            //
            // EQExtractor2Form1
            //
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(633, 126);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.ZoneLabel);
            this.Controls.Add(this.ZoneLabelText);
            this.Controls.Add(this.PacketCountLabel);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.ClientVersionLabel);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.ProgressBar);
            this.Menu = this.mainMenu1;
            this.Name = "EQExtractor2Form1";
            this.Text = "EQExtractor2";
            this.Load += new System.EventHandler(this.EQExtractor2Form1_Load);
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.OpenFileDialog InputFileOpenDialog;
        private System.Windows.Forms.SaveFileDialog PacketDumpFileDialog;
        private System.Windows.Forms.ProgressBar ProgressBar;
        private System.Windows.Forms.MainMenu mainMenu1;
        private System.Windows.Forms.MenuItem menuFile;
        private System.Windows.Forms.MenuItem menuLoadPCAP;
        private System.Windows.Forms.MenuItem menuGenerateSQL;
        private System.Windows.Forms.MenuItem menuPacketDump;
        private System.Windows.Forms.MenuItem menuDumpAAs;
        private System.Windows.Forms.MenuItem menuExit;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label ClientVersionLabel;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label PacketCountLabel;
        private System.Windows.Forms.Label ZoneLabelText;
        private System.Windows.Forms.Label ZoneLabel;
        private System.Windows.Forms.MenuItem menuView;
        private System.Windows.Forms.MenuItem menuViewDebugLog;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel StatusBar;
        private System.Windows.Forms.MenuItem menuViewPackets;
        private System.Windows.Forms.MenuItem menuOptions;
    }
}

