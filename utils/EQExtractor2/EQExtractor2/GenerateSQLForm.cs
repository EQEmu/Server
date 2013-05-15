using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace EQExtractor2
{
    public partial class GenerateSQLForm : Form
    {
        public string FileName = "";

        public GenerateSQLForm()
        {
            InitializeComponent();
        }

        public void RecalculateBaseInsertIDs()
        {
            UInt32 ZoneNumber = Convert.ToUInt32(ZoneIDTextBox.Text);

            NPCTypesTextBox.Text = ((ZoneNumber * 1000) + (VersionSelector.Value * 100)).ToString();
            SpawnEntryTextBox.Text = NPCTypesTextBox.Text;
            SpawnGroupTextBox.Text = NPCTypesTextBox.Text;
            Spawn2TextBox.Text = NPCTypesTextBox.Text;
            GridTextBox.Text = NPCTypesTextBox.Text;
            ObjectTextBox.Text = NPCTypesTextBox.Text;
            GroundSpawnTextBox.Text = NPCTypesTextBox.Text;
            MerchantTextBox.Text = NPCTypesTextBox.Text;
            DoorsTextBox.Text = ((VersionSelector.Value * 1000)).ToString();
        }

        private void VersionSelector_ValueChanged(object sender, EventArgs e)
        {
            RecalculateBaseInsertIDs();
        }

        private void ZoneIDTextBox_Validated(object sender, EventArgs e)
        {
            RecalculateBaseInsertIDs();
        }

        private void SpawnCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (!SpawnCheckBox.Checked)
            {
                GridCheckBox.Checked = false;
                GridCheckBox.Enabled = false;
            }
            else
                GridCheckBox.Enabled = true;
        }

        private void UpdateExistingNPCTypesCheckbox_CheckedChanged(object sender, EventArgs e)
        {
            if (UpdateExistingNPCTypesCheckbox.Checked)
            {
                SpawnCheckBox.Checked = false;
                SpawnCheckBox.Enabled = false;
                GridCheckBox.Checked = false;
                GridCheckBox.Enabled = false;
                MerchantCheckBox.Checked = false;
                MerchantCheckBox.Enabled = false;
                ZoneCheckBox.Checked = false;
                ZonePointCheckBox.Checked = false;
                GridCheckBox.Checked = false;
                DoorCheckBox.Checked = false;
                ObjectCheckBox.Checked = false;
                GroundSpawnCheckBox.Checked = false;
            }
            else
            {
                SpawnCheckBox.Enabled = true;
                GridCheckBox.Enabled = true;
                MerchantCheckBox.Enabled = true;
            }
        }

        private void GenerateSQLButton_Click(object sender, EventArgs e)
        {
            if (SQLFileDialog.ShowDialog() == DialogResult.OK)
            {
                FileName = SQLFileDialog.FileName;
                this.DialogResult = DialogResult.OK;
                //Close();
                //return;
            }

        }

        private void SQLCancelButton_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
        }


    }


}
