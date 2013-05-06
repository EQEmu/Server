namespace EQExtractor2
{
    partial class GenerateSQLForm
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.MerchantTextBox = new System.Windows.Forms.MaskedTextBox();
            this.MerchantLabel = new System.Windows.Forms.Label();
            this.GroundSpawnTextBox = new System.Windows.Forms.MaskedTextBox();
            this.GroundSpawnLabel = new System.Windows.Forms.Label();
            this.ObjectTextBox = new System.Windows.Forms.MaskedTextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.GridTextBox = new System.Windows.Forms.MaskedTextBox();
            this.GridLabel = new System.Windows.Forms.Label();
            this.Spawn2TextBox = new System.Windows.Forms.MaskedTextBox();
            this.Spawn2Label = new System.Windows.Forms.Label();
            this.SpawnEntryTextBox = new System.Windows.Forms.MaskedTextBox();
            this.SpawnEntryLabel = new System.Windows.Forms.Label();
            this.SpawnGroupTextBox = new System.Windows.Forms.MaskedTextBox();
            this.SpawnGroupLabel = new System.Windows.Forms.Label();
            this.NPCTypesTextBox = new System.Windows.Forms.MaskedTextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.DoorsTextBox = new System.Windows.Forms.MaskedTextBox();
            this.DoorsLabel = new System.Windows.Forms.Label();
            this.VersionSelector = new System.Windows.Forms.NumericUpDown();
            this.VersionLabel = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.ZoneIDTextBox = new System.Windows.Forms.MaskedTextBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.NPCTypesTintCheckBox = new System.Windows.Forms.CheckBox();
            this.UpdateExistingNPCTypesCheckbox = new System.Windows.Forms.CheckBox();
            this.ZonePointCheckBox = new System.Windows.Forms.CheckBox();
            this.ZoneCheckBox = new System.Windows.Forms.CheckBox();
            this.MerchantCheckBox = new System.Windows.Forms.CheckBox();
            this.GroundSpawnCheckBox = new System.Windows.Forms.CheckBox();
            this.ObjectCheckBox = new System.Windows.Forms.CheckBox();
            this.GridCheckBox = new System.Windows.Forms.CheckBox();
            this.SpawnCheckBox = new System.Windows.Forms.CheckBox();
            this.DoorCheckBox = new System.Windows.Forms.CheckBox();
            this.SQLFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.GenerateSQLButton = new System.Windows.Forms.Button();
            this.SQLCancelButton = new System.Windows.Forms.Button();
            this.MiscOptions = new System.Windows.Forms.GroupBox();
            this.SpawnNameFilter = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.CoalesceWaypoints = new System.Windows.Forms.CheckBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.InvisibleMenCheckBox = new System.Windows.Forms.CheckBox();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.VersionSelector)).BeginInit();
            this.groupBox2.SuspendLayout();
            this.MiscOptions.SuspendLayout();
            this.SuspendLayout();
            //
            // groupBox1
            //
            this.groupBox1.Controls.Add(this.MerchantTextBox);
            this.groupBox1.Controls.Add(this.MerchantLabel);
            this.groupBox1.Controls.Add(this.GroundSpawnTextBox);
            this.groupBox1.Controls.Add(this.GroundSpawnLabel);
            this.groupBox1.Controls.Add(this.ObjectTextBox);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.GridTextBox);
            this.groupBox1.Controls.Add(this.GridLabel);
            this.groupBox1.Controls.Add(this.Spawn2TextBox);
            this.groupBox1.Controls.Add(this.Spawn2Label);
            this.groupBox1.Controls.Add(this.SpawnEntryTextBox);
            this.groupBox1.Controls.Add(this.SpawnEntryLabel);
            this.groupBox1.Controls.Add(this.SpawnGroupTextBox);
            this.groupBox1.Controls.Add(this.SpawnGroupLabel);
            this.groupBox1.Controls.Add(this.NPCTypesTextBox);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.DoorsTextBox);
            this.groupBox1.Controls.Add(this.DoorsLabel);
            this.groupBox1.Controls.Add(this.VersionSelector);
            this.groupBox1.Controls.Add(this.VersionLabel);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.ZoneIDTextBox);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(262, 345);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Insert IDs";
            //
            // MerchantTextBox
            //
            this.MerchantTextBox.Enabled = false;
            this.MerchantTextBox.HidePromptOnLeave = true;
            this.MerchantTextBox.Location = new System.Drawing.Point(107, 302);
            this.MerchantTextBox.Mask = "0000000000";
            this.MerchantTextBox.Name = "MerchantTextBox";
            this.MerchantTextBox.PromptChar = ' ';
            this.MerchantTextBox.Size = new System.Drawing.Size(61, 20);
            this.MerchantTextBox.TabIndex = 34;
            //
            // MerchantLabel
            //
            this.MerchantLabel.AutoSize = true;
            this.MerchantLabel.Location = new System.Drawing.Point(14, 306);
            this.MerchantLabel.Name = "MerchantLabel";
            this.MerchantLabel.Size = new System.Drawing.Size(76, 13);
            this.MerchantLabel.TabIndex = 33;
            this.MerchantLabel.Text = "Merchant Lists";
            //
            // GroundSpawnTextBox
            //
            this.GroundSpawnTextBox.Enabled = false;
            this.GroundSpawnTextBox.HidePromptOnLeave = true;
            this.GroundSpawnTextBox.Location = new System.Drawing.Point(107, 274);
            this.GroundSpawnTextBox.Mask = "0000000000";
            this.GroundSpawnTextBox.Name = "GroundSpawnTextBox";
            this.GroundSpawnTextBox.PromptChar = ' ';
            this.GroundSpawnTextBox.Size = new System.Drawing.Size(61, 20);
            this.GroundSpawnTextBox.TabIndex = 32;
            //
            // GroundSpawnLabel
            //
            this.GroundSpawnLabel.AutoSize = true;
            this.GroundSpawnLabel.Location = new System.Drawing.Point(7, 278);
            this.GroundSpawnLabel.Name = "GroundSpawnLabel";
            this.GroundSpawnLabel.Size = new System.Drawing.Size(83, 13);
            this.GroundSpawnLabel.TabIndex = 31;
            this.GroundSpawnLabel.Text = "Ground Spawns";
            //
            // ObjectTextBox
            //
            this.ObjectTextBox.Enabled = false;
            this.ObjectTextBox.HidePromptOnLeave = true;
            this.ObjectTextBox.Location = new System.Drawing.Point(107, 246);
            this.ObjectTextBox.Mask = "0000000000";
            this.ObjectTextBox.Name = "ObjectTextBox";
            this.ObjectTextBox.PromptChar = ' ';
            this.ObjectTextBox.Size = new System.Drawing.Size(61, 20);
            this.ObjectTextBox.TabIndex = 30;
            //
            // label1
            //
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(47, 250);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(43, 13);
            this.label1.TabIndex = 29;
            this.label1.Text = "Objects";
            //
            // GridTextBox
            //
            this.GridTextBox.Enabled = false;
            this.GridTextBox.HidePromptOnLeave = true;
            this.GridTextBox.Location = new System.Drawing.Point(107, 218);
            this.GridTextBox.Mask = "0000000000";
            this.GridTextBox.Name = "GridTextBox";
            this.GridTextBox.PromptChar = ' ';
            this.GridTextBox.Size = new System.Drawing.Size(61, 20);
            this.GridTextBox.TabIndex = 28;
            //
            // GridLabel
            //
            this.GridLabel.AutoSize = true;
            this.GridLabel.Location = new System.Drawing.Point(59, 222);
            this.GridLabel.Name = "GridLabel";
            this.GridLabel.Size = new System.Drawing.Size(31, 13);
            this.GridLabel.TabIndex = 27;
            this.GridLabel.Text = "Grids";
            //
            // Spawn2TextBox
            //
            this.Spawn2TextBox.Enabled = false;
            this.Spawn2TextBox.HidePromptOnLeave = true;
            this.Spawn2TextBox.Location = new System.Drawing.Point(107, 190);
            this.Spawn2TextBox.Mask = "0000000000";
            this.Spawn2TextBox.Name = "Spawn2TextBox";
            this.Spawn2TextBox.PromptChar = ' ';
            this.Spawn2TextBox.Size = new System.Drawing.Size(61, 20);
            this.Spawn2TextBox.TabIndex = 26;
            //
            // Spawn2Label
            //
            this.Spawn2Label.AutoSize = true;
            this.Spawn2Label.Location = new System.Drawing.Point(44, 194);
            this.Spawn2Label.Name = "Spawn2Label";
            this.Spawn2Label.Size = new System.Drawing.Size(46, 13);
            this.Spawn2Label.TabIndex = 25;
            this.Spawn2Label.Text = "Spawn2";
            //
            // SpawnEntryTextBox
            //
            this.SpawnEntryTextBox.Enabled = false;
            this.SpawnEntryTextBox.HidePromptOnLeave = true;
            this.SpawnEntryTextBox.Location = new System.Drawing.Point(107, 162);
            this.SpawnEntryTextBox.Mask = "0000000000";
            this.SpawnEntryTextBox.Name = "SpawnEntryTextBox";
            this.SpawnEntryTextBox.PromptChar = ' ';
            this.SpawnEntryTextBox.Size = new System.Drawing.Size(61, 20);
            this.SpawnEntryTextBox.TabIndex = 24;
            //
            // SpawnEntryLabel
            //
            this.SpawnEntryLabel.AutoSize = true;
            this.SpawnEntryLabel.Location = new System.Drawing.Point(26, 166);
            this.SpawnEntryLabel.Name = "SpawnEntryLabel";
            this.SpawnEntryLabel.Size = new System.Drawing.Size(64, 13);
            this.SpawnEntryLabel.TabIndex = 23;
            this.SpawnEntryLabel.Text = "SpawnEntry";
            //
            // SpawnGroupTextBox
            //
            this.SpawnGroupTextBox.Enabled = false;
            this.SpawnGroupTextBox.HidePromptOnLeave = true;
            this.SpawnGroupTextBox.Location = new System.Drawing.Point(107, 134);
            this.SpawnGroupTextBox.Mask = "0000000000";
            this.SpawnGroupTextBox.Name = "SpawnGroupTextBox";
            this.SpawnGroupTextBox.PromptChar = ' ';
            this.SpawnGroupTextBox.Size = new System.Drawing.Size(61, 20);
            this.SpawnGroupTextBox.TabIndex = 22;
            //
            // SpawnGroupLabel
            //
            this.SpawnGroupLabel.AutoSize = true;
            this.SpawnGroupLabel.Location = new System.Drawing.Point(21, 138);
            this.SpawnGroupLabel.Name = "SpawnGroupLabel";
            this.SpawnGroupLabel.Size = new System.Drawing.Size(69, 13);
            this.SpawnGroupLabel.TabIndex = 21;
            this.SpawnGroupLabel.Text = "SpawnGroup";
            //
            // NPCTypesTextBox
            //
            this.NPCTypesTextBox.Enabled = false;
            this.NPCTypesTextBox.HidePromptOnLeave = true;
            this.NPCTypesTextBox.Location = new System.Drawing.Point(107, 106);
            this.NPCTypesTextBox.Mask = "0000000000";
            this.NPCTypesTextBox.Name = "NPCTypesTextBox";
            this.NPCTypesTextBox.PromptChar = ' ';
            this.NPCTypesTextBox.Size = new System.Drawing.Size(61, 20);
            this.NPCTypesTextBox.TabIndex = 20;
            //
            // label3
            //
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(32, 110);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(58, 13);
            this.label3.TabIndex = 19;
            this.label3.Text = "NPCTypes";
            //
            // DoorsTextBox
            //
            this.DoorsTextBox.Enabled = false;
            this.DoorsTextBox.HidePromptOnLeave = true;
            this.DoorsTextBox.Location = new System.Drawing.Point(107, 78);
            this.DoorsTextBox.Mask = "0000000000";
            this.DoorsTextBox.Name = "DoorsTextBox";
            this.DoorsTextBox.PromptChar = ' ';
            this.DoorsTextBox.Size = new System.Drawing.Size(61, 20);
            this.DoorsTextBox.TabIndex = 18;
            //
            // DoorsLabel
            //
            this.DoorsLabel.AutoSize = true;
            this.DoorsLabel.Location = new System.Drawing.Point(55, 82);
            this.DoorsLabel.Name = "DoorsLabel";
            this.DoorsLabel.Size = new System.Drawing.Size(35, 13);
            this.DoorsLabel.TabIndex = 17;
            this.DoorsLabel.Text = "Doors";
            //
            // VersionSelector
            //
            this.VersionSelector.Enabled = false;
            this.VersionSelector.Location = new System.Drawing.Point(107, 50);
            this.VersionSelector.Maximum = new decimal(new int[] {
            999,
            0,
            0,
            0});
            this.VersionSelector.Name = "VersionSelector";
            this.VersionSelector.Size = new System.Drawing.Size(39, 20);
            this.VersionSelector.TabIndex = 16;
            this.VersionSelector.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.VersionSelector.ValueChanged += new System.EventHandler(this.VersionSelector_ValueChanged);
            //
            // VersionLabel
            //
            this.VersionLabel.AutoSize = true;
            this.VersionLabel.Location = new System.Drawing.Point(48, 54);
            this.VersionLabel.Name = "VersionLabel";
            this.VersionLabel.Size = new System.Drawing.Size(42, 13);
            this.VersionLabel.TabIndex = 15;
            this.VersionLabel.Text = "Version";
            //
            // label4
            //
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(44, 26);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(46, 13);
            this.label4.TabIndex = 14;
            this.label4.Text = "ZoneID:";
            //
            // ZoneIDTextBox
            //
            this.ZoneIDTextBox.Enabled = false;
            this.ZoneIDTextBox.HidePromptOnLeave = true;
            this.ZoneIDTextBox.Location = new System.Drawing.Point(107, 22);
            this.ZoneIDTextBox.Mask = "0000000";
            this.ZoneIDTextBox.Name = "ZoneIDTextBox";
            this.ZoneIDTextBox.PromptChar = ' ';
            this.ZoneIDTextBox.Size = new System.Drawing.Size(100, 20);
            this.ZoneIDTextBox.TabIndex = 13;
            this.ZoneIDTextBox.Validated += new System.EventHandler(this.ZoneIDTextBox_Validated);
            //
            // groupBox2
            //
            this.groupBox2.Controls.Add(this.InvisibleMenCheckBox);
            this.groupBox2.Controls.Add(this.NPCTypesTintCheckBox);
            this.groupBox2.Controls.Add(this.UpdateExistingNPCTypesCheckbox);
            this.groupBox2.Controls.Add(this.ZonePointCheckBox);
            this.groupBox2.Controls.Add(this.ZoneCheckBox);
            this.groupBox2.Controls.Add(this.MerchantCheckBox);
            this.groupBox2.Controls.Add(this.GroundSpawnCheckBox);
            this.groupBox2.Controls.Add(this.ObjectCheckBox);
            this.groupBox2.Controls.Add(this.GridCheckBox);
            this.groupBox2.Controls.Add(this.SpawnCheckBox);
            this.groupBox2.Controls.Add(this.DoorCheckBox);
            this.groupBox2.Location = new System.Drawing.Point(298, 12);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(200, 345);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Include";
            //
            // NPCTypesTintCheckBox
            //
            this.NPCTypesTintCheckBox.AutoSize = true;
            this.NPCTypesTintCheckBox.Location = new System.Drawing.Point(24, 233);
            this.NPCTypesTintCheckBox.Name = "NPCTypesTintCheckBox";
            this.NPCTypesTintCheckBox.Size = new System.Drawing.Size(117, 17);
            this.NPCTypesTintCheckBox.TabIndex = 42;
            this.NPCTypesTintCheckBox.Text = "Use npc_types_tint";
            this.toolTip1.SetToolTip(this.NPCTypesTintCheckBox, "Uses the npc_types_tint table instead of the tint columns in the npc_types table." +
        "");
            this.NPCTypesTintCheckBox.UseVisualStyleBackColor = true;
            //
            // UpdateExistingNPCTypesCheckbox
            //
            this.UpdateExistingNPCTypesCheckbox.AutoSize = true;
            this.UpdateExistingNPCTypesCheckbox.Location = new System.Drawing.Point(24, 210);
            this.UpdateExistingNPCTypesCheckbox.Name = "UpdateExistingNPCTypesCheckbox";
            this.UpdateExistingNPCTypesCheckbox.Size = new System.Drawing.Size(152, 17);
            this.UpdateExistingNPCTypesCheckbox.TabIndex = 41;
            this.UpdateExistingNPCTypesCheckbox.Text = "Update existing NPC types";
            this.UpdateExistingNPCTypesCheckbox.UseVisualStyleBackColor = true;
            this.UpdateExistingNPCTypesCheckbox.CheckedChanged += new System.EventHandler(this.UpdateExistingNPCTypesCheckbox_CheckedChanged);
            //
            // ZonePointCheckBox
            //
            this.ZonePointCheckBox.AutoSize = true;
            this.ZonePointCheckBox.Checked = true;
            this.ZonePointCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ZonePointCheckBox.Location = new System.Drawing.Point(24, 187);
            this.ZonePointCheckBox.Name = "ZonePointCheckBox";
            this.ZonePointCheckBox.Size = new System.Drawing.Size(83, 17);
            this.ZonePointCheckBox.TabIndex = 40;
            this.ZonePointCheckBox.Text = "Zone Points";
            this.ZonePointCheckBox.UseVisualStyleBackColor = true;
            //
            // ZoneCheckBox
            //
            this.ZoneCheckBox.AutoSize = true;
            this.ZoneCheckBox.Checked = true;
            this.ZoneCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ZoneCheckBox.Location = new System.Drawing.Point(24, 164);
            this.ZoneCheckBox.Name = "ZoneCheckBox";
            this.ZoneCheckBox.Size = new System.Drawing.Size(84, 17);
            this.ZoneCheckBox.TabIndex = 39;
            this.ZoneCheckBox.Text = "Zone Config";
            this.ZoneCheckBox.UseVisualStyleBackColor = true;
            //
            // MerchantCheckBox
            //
            this.MerchantCheckBox.AutoSize = true;
            this.MerchantCheckBox.Checked = true;
            this.MerchantCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.MerchantCheckBox.Location = new System.Drawing.Point(24, 141);
            this.MerchantCheckBox.Name = "MerchantCheckBox";
            this.MerchantCheckBox.Size = new System.Drawing.Size(95, 17);
            this.MerchantCheckBox.TabIndex = 38;
            this.MerchantCheckBox.Text = "Merchant Lists";
            this.MerchantCheckBox.UseVisualStyleBackColor = true;
            //
            // GroundSpawnCheckBox
            //
            this.GroundSpawnCheckBox.AutoSize = true;
            this.GroundSpawnCheckBox.Checked = true;
            this.GroundSpawnCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.GroundSpawnCheckBox.Location = new System.Drawing.Point(24, 118);
            this.GroundSpawnCheckBox.Name = "GroundSpawnCheckBox";
            this.GroundSpawnCheckBox.Size = new System.Drawing.Size(102, 17);
            this.GroundSpawnCheckBox.TabIndex = 9;
            this.GroundSpawnCheckBox.Text = "Ground Spawns";
            this.GroundSpawnCheckBox.UseVisualStyleBackColor = true;
            //
            // ObjectCheckBox
            //
            this.ObjectCheckBox.AutoSize = true;
            this.ObjectCheckBox.Checked = true;
            this.ObjectCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ObjectCheckBox.Location = new System.Drawing.Point(24, 95);
            this.ObjectCheckBox.Name = "ObjectCheckBox";
            this.ObjectCheckBox.Size = new System.Drawing.Size(62, 17);
            this.ObjectCheckBox.TabIndex = 8;
            this.ObjectCheckBox.Text = "Objects";
            this.ObjectCheckBox.UseVisualStyleBackColor = true;
            //
            // GridCheckBox
            //
            this.GridCheckBox.AutoSize = true;
            this.GridCheckBox.Checked = true;
            this.GridCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.GridCheckBox.Location = new System.Drawing.Point(24, 72);
            this.GridCheckBox.Name = "GridCheckBox";
            this.GridCheckBox.Size = new System.Drawing.Size(50, 17);
            this.GridCheckBox.TabIndex = 7;
            this.GridCheckBox.Text = "Grids";
            this.GridCheckBox.UseVisualStyleBackColor = true;
            //
            // SpawnCheckBox
            //
            this.SpawnCheckBox.AutoSize = true;
            this.SpawnCheckBox.Checked = true;
            this.SpawnCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.SpawnCheckBox.Location = new System.Drawing.Point(24, 49);
            this.SpawnCheckBox.Name = "SpawnCheckBox";
            this.SpawnCheckBox.Size = new System.Drawing.Size(64, 17);
            this.SpawnCheckBox.TabIndex = 5;
            this.SpawnCheckBox.Text = "Spawns";
            this.SpawnCheckBox.UseVisualStyleBackColor = true;
            this.SpawnCheckBox.CheckedChanged += new System.EventHandler(this.SpawnCheckBox_CheckedChanged);
            //
            // DoorCheckBox
            //
            this.DoorCheckBox.AutoSize = true;
            this.DoorCheckBox.Checked = true;
            this.DoorCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.DoorCheckBox.Location = new System.Drawing.Point(24, 26);
            this.DoorCheckBox.Name = "DoorCheckBox";
            this.DoorCheckBox.Size = new System.Drawing.Size(54, 17);
            this.DoorCheckBox.TabIndex = 4;
            this.DoorCheckBox.Text = "Doors";
            this.DoorCheckBox.UseVisualStyleBackColor = true;
            //
            // SQLFileDialog
            //
            this.SQLFileDialog.Filter = "SQL Files (*.sql)|*.sql|Text Files (*.txt)|*.txt|All files (*.*)|*.*";
            this.SQLFileDialog.Title = "Generate SQL and Save As";
            //
            // GenerateSQLButton
            //
            this.GenerateSQLButton.Location = new System.Drawing.Point(126, 474);
            this.GenerateSQLButton.Name = "GenerateSQLButton";
            this.GenerateSQLButton.Size = new System.Drawing.Size(100, 27);
            this.GenerateSQLButton.TabIndex = 3;
            this.GenerateSQLButton.Text = "Generate SQL";
            this.GenerateSQLButton.UseVisualStyleBackColor = true;
            this.GenerateSQLButton.Click += new System.EventHandler(this.GenerateSQLButton_Click);
            //
            // SQLCancelButton
            //
            this.SQLCancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.SQLCancelButton.Location = new System.Drawing.Point(305, 474);
            this.SQLCancelButton.Name = "SQLCancelButton";
            this.SQLCancelButton.Size = new System.Drawing.Size(100, 27);
            this.SQLCancelButton.TabIndex = 4;
            this.SQLCancelButton.Text = "Cancel";
            this.SQLCancelButton.UseVisualStyleBackColor = true;
            this.SQLCancelButton.Click += new System.EventHandler(this.SQLCancelButton_Click);
            //
            // MiscOptions
            //
            this.MiscOptions.Controls.Add(this.SpawnNameFilter);
            this.MiscOptions.Controls.Add(this.label2);
            this.MiscOptions.Controls.Add(this.CoalesceWaypoints);
            this.MiscOptions.Location = new System.Drawing.Point(12, 368);
            this.MiscOptions.Name = "MiscOptions";
            this.MiscOptions.Size = new System.Drawing.Size(486, 76);
            this.MiscOptions.TabIndex = 2;
            this.MiscOptions.TabStop = false;
            this.MiscOptions.Text = "Misc. Options";
            //
            // SpawnNameFilter
            //
            this.SpawnNameFilter.Location = new System.Drawing.Point(229, 46);
            this.SpawnNameFilter.Name = "SpawnNameFilter";
            this.SpawnNameFilter.Size = new System.Drawing.Size(209, 20);
            this.SpawnNameFilter.TabIndex = 5;
            //
            // label2
            //
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(14, 49);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(209, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Only include spawns that contain the string";
            //
            // CoalesceWaypoints
            //
            this.CoalesceWaypoints.AutoSize = true;
            this.CoalesceWaypoints.Checked = true;
            this.CoalesceWaypoints.CheckState = System.Windows.Forms.CheckState.Checked;
            this.CoalesceWaypoints.Location = new System.Drawing.Point(17, 19);
            this.CoalesceWaypoints.Name = "CoalesceWaypoints";
            this.CoalesceWaypoints.Size = new System.Drawing.Size(184, 17);
            this.CoalesceWaypoints.TabIndex = 3;
            this.CoalesceWaypoints.Text = "Automatically coalesce waypoints";
            this.toolTip1.SetToolTip(this.CoalesceWaypoints, "Remove redundant waypoints from generated SQL");
            this.CoalesceWaypoints.UseVisualStyleBackColor = true;
            //
            // InvisibleMenCheckBox
            //
            this.InvisibleMenCheckBox.AutoSize = true;
            this.InvisibleMenCheckBox.Location = new System.Drawing.Point(24, 256);
            this.InvisibleMenCheckBox.Name = "InvisibleMenCheckBox";
            this.InvisibleMenCheckBox.Size = new System.Drawing.Size(126, 17);
            this.InvisibleMenCheckBox.TabIndex = 43;
            this.InvisibleMenCheckBox.Text = "Include Invisible Men";
            this.toolTip1.SetToolTip(this.InvisibleMenCheckBox, "Includes Race 127 NPCs");
            this.InvisibleMenCheckBox.UseVisualStyleBackColor = true;
            //
            // GenerateSQLForm
            //
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.SQLCancelButton;
            this.ClientSize = new System.Drawing.Size(514, 513);
            this.Controls.Add(this.MiscOptions);
            this.Controls.Add(this.SQLCancelButton);
            this.Controls.Add(this.GenerateSQLButton);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Name = "GenerateSQLForm";
            this.Text = "Generate SQL";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.VersionSelector)).EndInit();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.MiscOptions.ResumeLayout(false);
            this.MiscOptions.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label VersionLabel;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label DoorsLabel;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label SpawnGroupLabel;
        private System.Windows.Forms.Label SpawnEntryLabel;
        private System.Windows.Forms.Label Spawn2Label;
        private System.Windows.Forms.Label GridLabel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label GroundSpawnLabel;
        private System.Windows.Forms.Label MerchantLabel;
        public System.Windows.Forms.NumericUpDown VersionSelector;
        public System.Windows.Forms.MaskedTextBox ZoneIDTextBox;
        public System.Windows.Forms.MaskedTextBox DoorsTextBox;
        public System.Windows.Forms.MaskedTextBox NPCTypesTextBox;
        public System.Windows.Forms.MaskedTextBox SpawnGroupTextBox;
        public System.Windows.Forms.MaskedTextBox SpawnEntryTextBox;
        public System.Windows.Forms.MaskedTextBox Spawn2TextBox;
        public System.Windows.Forms.MaskedTextBox GridTextBox;
        public System.Windows.Forms.MaskedTextBox ObjectTextBox;
        public System.Windows.Forms.MaskedTextBox GroundSpawnTextBox;
        public System.Windows.Forms.MaskedTextBox MerchantTextBox;
        public System.Windows.Forms.CheckBox SpawnCheckBox;
        public System.Windows.Forms.CheckBox DoorCheckBox;
        public System.Windows.Forms.CheckBox GroundSpawnCheckBox;
        public System.Windows.Forms.CheckBox ObjectCheckBox;
        public System.Windows.Forms.CheckBox GridCheckBox;
        public System.Windows.Forms.CheckBox ZonePointCheckBox;
        public System.Windows.Forms.CheckBox ZoneCheckBox;
        public System.Windows.Forms.CheckBox MerchantCheckBox;
        public System.Windows.Forms.CheckBox NPCTypesTintCheckBox;
        public System.Windows.Forms.CheckBox UpdateExistingNPCTypesCheckbox;
        private System.Windows.Forms.SaveFileDialog SQLFileDialog;
        private System.Windows.Forms.Button GenerateSQLButton;
        private System.Windows.Forms.Button SQLCancelButton;
        private System.Windows.Forms.GroupBox MiscOptions;
        public System.Windows.Forms.CheckBox CoalesceWaypoints;
        public System.Windows.Forms.TextBox SpawnNameFilter;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ToolTip toolTip1;
        public System.Windows.Forms.CheckBox InvisibleMenCheckBox;
    }
}