namespace EQExtractor2
{
    partial class LogForm
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
            this.ConsoleWindow = new System.Windows.Forms.ListBox();
            this.SuspendLayout();
            //
            // ConsoleWindow
            //
            this.ConsoleWindow.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.ConsoleWindow.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.ConsoleWindow.FormattingEnabled = true;
            this.ConsoleWindow.HorizontalScrollbar = true;
            this.ConsoleWindow.ItemHeight = 14;
            this.ConsoleWindow.Location = new System.Drawing.Point(12, 12);
            this.ConsoleWindow.Name = "ConsoleWindow";
            this.ConsoleWindow.Size = new System.Drawing.Size(938, 326);
            this.ConsoleWindow.TabIndex = 35;
            //
            // LogForm
            //
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(962, 355);
            this.Controls.Add(this.ConsoleWindow);
            this.Name = "LogForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "EQExtractor2 Debug Log";
            this.Closing += new System.ComponentModel.CancelEventHandler(this.LogForm_FormClosing);
            this.ResumeLayout(false);

        }

        #endregion

        public System.Windows.Forms.ListBox ConsoleWindow;

    }
}