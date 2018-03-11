namespace helloworld
{
    partial class Form1
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
            this.TxtLog = new System.Windows.Forms.TextBox();
            this.BtnHello = new System.Windows.Forms.Button();
            this.TmrMessage = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // TxtLog
            // 
            this.TxtLog.Location = new System.Drawing.Point(13, 13);
            this.TxtLog.Multiline = true;
            this.TxtLog.Name = "TxtLog";
            this.TxtLog.Size = new System.Drawing.Size(354, 151);
            this.TxtLog.TabIndex = 0;
            // 
            // BtnHello
            // 
            this.BtnHello.Location = new System.Drawing.Point(12, 170);
            this.BtnHello.Name = "BtnHello";
            this.BtnHello.Size = new System.Drawing.Size(354, 64);
            this.BtnHello.TabIndex = 1;
            this.BtnHello.Text = "Hello World";
            this.BtnHello.UseVisualStyleBackColor = true;
            this.BtnHello.Click += new System.EventHandler(this.BtnHello_Click);
            // 
            // TmrMessage
            // 
            this.TmrMessage.Tick += new System.EventHandler(this.TmrMessage_Tick);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(380, 244);
            this.Controls.Add(this.BtnHello);
            this.Controls.Add(this.TxtLog);
            this.Name = "Form1";
            this.Text = "helloworld";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox TxtLog;
        private System.Windows.Forms.Button BtnHello;
        private System.Windows.Forms.Timer TmrMessage;
    }
}

