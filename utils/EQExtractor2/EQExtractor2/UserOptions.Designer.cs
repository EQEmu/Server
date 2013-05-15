namespace EQExtractor2
{
    partial class UserOptions
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
            this.label1 = new System.Windows.Forms.Label();
            this.PacketDumpViewerProgram = new System.Windows.Forms.TextBox();
            this.ShowDebugWindowOnStartup = new System.Windows.Forms.CheckBox();
            this.ShowTimeStamps = new System.Windows.Forms.CheckBox();
            this.OptionsOKButton = new System.Windows.Forms.Button();
            this.OptionsCancelButton = new System.Windows.Forms.Button();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.EQPacketDebugFilename = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            //
            // label1
            //
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(107, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Packet Dump Viewer";
            //
            // PacketDumpViewerProgram
            //
            this.PacketDumpViewerProgram.Location = new System.Drawing.Point(125, 6);
            this.PacketDumpViewerProgram.Name = "PacketDumpViewerProgram";
            this.PacketDumpViewerProgram.Size = new System.Drawing.Size(390, 20);
            this.PacketDumpViewerProgram.TabIndex = 1;
            this.toolTip1.SetToolTip(this.PacketDumpViewerProgram, "If not using notepad/wordpad include the full path, e.g. C:\\Program Files (x86)\\N" +
                    "otepad++\\notepad++.exe");
            //
            // ShowDebugWindowOnStartup
            //
            this.ShowDebugWindowOnStartup.AutoSize = true;
            this.ShowDebugWindowOnStartup.Location = new System.Drawing.Point(15, 61);
            this.ShowDebugWindowOnStartup.Name = "ShowDebugWindowOnStartup";
            this.ShowDebugWindowOnStartup.Size = new System.Drawing.Size(184, 17);
            this.ShowDebugWindowOnStartup.TabIndex = 2;
            this.ShowDebugWindowOnStartup.Text = "Show Debug Window On Startup";
            this.ShowDebugWindowOnStartup.UseVisualStyleBackColor = true;
            //
            // ShowTimeStamps
            //
            this.ShowTimeStamps.AutoSize = true;
            this.ShowTimeStamps.Location = new System.Drawing.Point(15, 84);
            this.ShowTimeStamps.Name = "ShowTimeStamps";
            this.ShowTimeStamps.Size = new System.Drawing.Size(205, 17);
            this.ShowTimeStamps.TabIndex = 3;
            this.ShowTimeStamps.Text = "Include Time Stamps In Packet Dump";
            this.ShowTimeStamps.UseVisualStyleBackColor = true;
            //
            // OptionsOKButton
            //
            this.OptionsOKButton.Location = new System.Drawing.Point(125, 122);
            this.OptionsOKButton.Name = "OptionsOKButton";
            this.OptionsOKButton.Size = new System.Drawing.Size(75, 23);
            this.OptionsOKButton.TabIndex = 4;
            this.OptionsOKButton.Text = "OK";
            this.OptionsOKButton.UseVisualStyleBackColor = true;
            this.OptionsOKButton.Click += new System.EventHandler(this.OptionsOKButton_Click);
            //
            // OptionsCancelButton
            //
            this.OptionsCancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.OptionsCancelButton.Location = new System.Drawing.Point(332, 122);
            this.OptionsCancelButton.Name = "OptionsCancelButton";
            this.OptionsCancelButton.Size = new System.Drawing.Size(75, 23);
            this.OptionsCancelButton.TabIndex = 5;
            this.OptionsCancelButton.Text = "Cancel";
            this.OptionsCancelButton.UseVisualStyleBackColor = true;
            this.OptionsCancelButton.Click += new System.EventHandler(this.OptionsCancelButton_Click);
            //
            // EQPacketDebugFilename
            //
            this.EQPacketDebugFilename.Location = new System.Drawing.Point(125, 32);
            this.EQPacketDebugFilename.Name = "EQPacketDebugFilename";
            this.EQPacketDebugFilename.Size = new System.Drawing.Size(390, 20);
            this.EQPacketDebugFilename.TabIndex = 7;
            this.toolTip1.SetToolTip(this.EQPacketDebugFilename, "Output file for low level netcode debugging");
            //
            // label2
            //
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 35);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(83, 13);
            this.label2.TabIndex = 6;
            this.label2.Text = "Netcode Debug";
            this.toolTip1.SetToolTip(this.label2, "Output file for low level netcode debugging");
            //
            // UserOptions
            //
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.OptionsCancelButton;
            this.ClientSize = new System.Drawing.Size(527, 166);
            this.Controls.Add(this.EQPacketDebugFilename);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.OptionsCancelButton);
            this.Controls.Add(this.OptionsOKButton);
            this.Controls.Add(this.ShowTimeStamps);
            this.Controls.Add(this.ShowDebugWindowOnStartup);
            this.Controls.Add(this.PacketDumpViewerProgram);
            this.Controls.Add(this.label1);
            this.Name = "UserOptions";
            this.Text = "Options";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button OptionsOKButton;
        private System.Windows.Forms.Button OptionsCancelButton;
        public System.Windows.Forms.TextBox PacketDumpViewerProgram;
        public System.Windows.Forms.CheckBox ShowDebugWindowOnStartup;
        public System.Windows.Forms.CheckBox ShowTimeStamps;
        private System.Windows.Forms.ToolTip toolTip1;
        public System.Windows.Forms.TextBox EQPacketDebugFilename;
        private System.Windows.Forms.Label label2;
    }
}