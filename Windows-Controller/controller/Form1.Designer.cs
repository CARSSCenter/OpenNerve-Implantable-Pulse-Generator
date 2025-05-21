namespace controller
{
    partial class Form1
    {
        private System.ComponentModel.IContainer components = null;
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }
        #region Windows Form Designer generated code
        private void InitializeComponent()
        {
            bConnect = new Button();
            bDisconnect = new Button();
            txtPA = new TextBox();
            labelOpcode = new Label();
            bSendPA = new Button();
            label1 = new Label();
            bSendPW = new Button();
            txtPW = new TextBox();
            bSendPF = new Button();
            txtPF = new TextBox();
            label2 = new Label();
            bStimOff = new Button();
            bStimOn = new Button();
            bSendTN = new Button();
            txtTN = new TextBox();
            label3 = new Label();
            bSendPR = new Button();
            txtPR = new TextBox();
            label4 = new Label();
            bGet = new Button();
            labelRX = new Label();
            labelTX = new Label();
            label5 = new Label();
            label6 = new Label();
            bSendTF = new Button();
            txtTF = new TextBox();
            label7 = new Label();
            bGetTF = new Button();
            bGetTN = new Button();
            bGetPR = new Button();
            bGetPF = new Button();
            bGetPW = new Button();
            bGetPA = new Button();
            SuspendLayout();
            // 
            // bConnect
            // 
            bConnect.Font = new Font("Times New Roman", 14F);
            bConnect.Location = new Point(17, 1);
            bConnect.Margin = new Padding(4, 5, 4, 5);
            bConnect.Name = "bConnect";
            bConnect.Size = new Size(221, 51);
            bConnect.TabIndex = 15;
            bConnect.Text = "Connect";
            bConnect.UseVisualStyleBackColor = true;
            bConnect.Click += bConnect_Click;
            // 
            // bDisconnect
            // 
            bDisconnect.Enabled = false;
            bDisconnect.Font = new Font("Times New Roman", 14F);
            bDisconnect.Location = new Point(17, 52);
            bDisconnect.Margin = new Padding(4, 5, 4, 5);
            bDisconnect.Name = "bDisconnect";
            bDisconnect.Size = new Size(221, 51);
            bDisconnect.TabIndex = 16;
            bDisconnect.Text = "Disconnect";
            bDisconnect.UseVisualStyleBackColor = true;
            bDisconnect.Click += bDisconnect_Click;
            // 
            // txtPA
            // 
            txtPA.Font = new Font("Times New Roman", 12F);
            txtPA.Location = new Point(26, 187);
            txtPA.Margin = new Padding(4, 5, 4, 5);
            txtPA.Name = "txtPA";
            txtPA.Size = new Size(72, 35);
            txtPA.TabIndex = 17;
            txtPA.TextAlign = HorizontalAlignment.Center;
            // 
            // labelOpcode
            // 
            labelOpcode.Font = new Font("Times New Roman", 14F);
            labelOpcode.Location = new Point(7, 154);
            labelOpcode.Margin = new Padding(4, 0, 4, 0);
            labelOpcode.MaximumSize = new Size(200, 33);
            labelOpcode.MinimumSize = new Size(240, 33);
            labelOpcode.Name = "labelOpcode";
            labelOpcode.Size = new Size(240, 33);
            labelOpcode.TabIndex = 18;
            labelOpcode.Text = "Amp, 0-0.2-5";
            labelOpcode.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // bSendPA
            // 
            bSendPA.Enabled = false;
            bSendPA.Font = new Font("Times New Roman", 14F);
            bSendPA.Location = new Point(106, 185);
            bSendPA.Margin = new Padding(4, 5, 4, 5);
            bSendPA.Name = "bSendPA";
            bSendPA.Size = new Size(71, 40);
            bSendPA.TabIndex = 19;
            bSendPA.Text = "set";
            bSendPA.UseVisualStyleBackColor = true;
            bSendPA.Click += bSendPA_Click;
            // 
            // label1
            // 
            label1.Font = new Font("Times New Roman", 14F);
            label1.Location = new Point(7, 225);
            label1.Margin = new Padding(4, 0, 4, 0);
            label1.MaximumSize = new Size(200, 33);
            label1.MinimumSize = new Size(240, 33);
            label1.Name = "label1";
            label1.Size = new Size(240, 33);
            label1.TabIndex = 20;
            label1.Text = "Width,100-1000";
            label1.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // bSendPW
            // 
            bSendPW.Enabled = false;
            bSendPW.Font = new Font("Times New Roman", 14F);
            bSendPW.Location = new Point(106, 260);
            bSendPW.Margin = new Padding(4, 5, 4, 5);
            bSendPW.Name = "bSendPW";
            bSendPW.Size = new Size(71, 40);
            bSendPW.TabIndex = 22;
            bSendPW.Text = "set";
            bSendPW.UseVisualStyleBackColor = true;
            bSendPW.Click += bSendPW_Click;
            // 
            // txtPW
            // 
            txtPW.Font = new Font("Times New Roman", 12F);
            txtPW.Location = new Point(26, 261);
            txtPW.Margin = new Padding(4, 5, 4, 5);
            txtPW.Name = "txtPW";
            txtPW.Size = new Size(72, 35);
            txtPW.TabIndex = 21;
            txtPW.TextAlign = HorizontalAlignment.Center;
            // 
            // bSendPF
            // 
            bSendPF.Enabled = false;
            bSendPF.Font = new Font("Times New Roman", 14F);
            bSendPF.Location = new Point(106, 334);
            bSendPF.Margin = new Padding(4, 5, 4, 5);
            bSendPF.Name = "bSendPF";
            bSendPF.Size = new Size(71, 40);
            bSendPF.TabIndex = 25;
            bSendPF.Text = "set";
            bSendPF.UseVisualStyleBackColor = true;
            bSendPF.Click += bSendPF_Click;
            // 
            // txtPF
            // 
            txtPF.Font = new Font("Times New Roman", 12F);
            txtPF.Location = new Point(26, 336);
            txtPF.Margin = new Padding(4, 5, 4, 5);
            txtPF.Name = "txtPF";
            txtPF.Size = new Size(72, 35);
            txtPF.TabIndex = 24;
            txtPF.TextAlign = HorizontalAlignment.Center;
            // 
            // label2
            // 
            label2.Font = new Font("Times New Roman", 14F);
            label2.Location = new Point(7, 300);
            label2.Margin = new Padding(4, 0, 4, 0);
            label2.MaximumSize = new Size(200, 33);
            label2.MinimumSize = new Size(240, 33);
            label2.Name = "label2";
            label2.Size = new Size(240, 33);
            label2.TabIndex = 23;
            label2.Text = "Freq, 1-100";
            label2.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // bStimOff
            // 
            bStimOff.Enabled = false;
            bStimOff.Font = new Font("Times New Roman", 14F);
            bStimOff.Location = new Point(15, 651);
            bStimOff.Margin = new Padding(4, 5, 4, 5);
            bStimOff.Name = "bStimOff";
            bStimOff.Size = new Size(221, 40);
            bStimOff.TabIndex = 27;
            bStimOff.Text = "Stim Off";
            bStimOff.UseVisualStyleBackColor = true;
            bStimOff.Click += bStimOff_Click;
            // 
            // bStimOn
            // 
            bStimOn.Enabled = false;
            bStimOn.Font = new Font("Times New Roman", 14F);
            bStimOn.Location = new Point(15, 609);
            bStimOn.Margin = new Padding(4, 5, 4, 5);
            bStimOn.Name = "bStimOn";
            bStimOn.Size = new Size(221, 40);
            bStimOn.TabIndex = 26;
            bStimOn.Text = "Stim On";
            bStimOn.UseVisualStyleBackColor = true;
            bStimOn.Click += bStimOn_Click;
            // 
            // bSendTN
            // 
            bSendTN.Enabled = false;
            bSendTN.Font = new Font("Times New Roman", 14F);
            bSendTN.Location = new Point(106, 486);
            bSendTN.Margin = new Padding(4, 5, 4, 5);
            bSendTN.Name = "bSendTN";
            bSendTN.Size = new Size(71, 40);
            bSendTN.TabIndex = 34;
            bSendTN.Text = "set";
            bSendTN.UseVisualStyleBackColor = true;
            bSendTN.Click += bSendTN_Click;
            // 
            // txtTN
            // 
            txtTN.Font = new Font("Times New Roman", 12F);
            txtTN.Location = new Point(26, 488);
            txtTN.Margin = new Padding(4, 5, 4, 5);
            txtTN.Name = "txtTN";
            txtTN.Size = new Size(72, 35);
            txtTN.TabIndex = 33;
            txtTN.TextAlign = HorizontalAlignment.Center;
            // 
            // label3
            // 
            label3.Font = new Font("Times New Roman", 14F);
            label3.Location = new Point(7, 451);
            label3.Margin = new Padding(4, 0, 4, 0);
            label3.MaximumSize = new Size(200, 33);
            label3.MinimumSize = new Size(240, 33);
            label3.Name = "label3";
            label3.Size = new Size(240, 33);
            label3.TabIndex = 32;
            label3.Text = "TrainOn, 10-300";
            label3.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // bSendPR
            // 
            bSendPR.Enabled = false;
            bSendPR.Font = new Font("Times New Roman", 14F);
            bSendPR.Location = new Point(106, 409);
            bSendPR.Margin = new Padding(4, 5, 4, 5);
            bSendPR.Name = "bSendPR";
            bSendPR.Size = new Size(71, 40);
            bSendPR.TabIndex = 31;
            bSendPR.Text = "set";
            bSendPR.UseVisualStyleBackColor = true;
            bSendPR.Click += bSendPR_Click;
            // 
            // txtPR
            // 
            txtPR.Font = new Font("Times New Roman", 12F);
            txtPR.Location = new Point(26, 412);
            txtPR.Margin = new Padding(4, 5, 4, 5);
            txtPR.Name = "txtPR";
            txtPR.Size = new Size(72, 35);
            txtPR.TabIndex = 30;
            txtPR.TextAlign = HorizontalAlignment.Center;
            // 
            // label4
            // 
            label4.Font = new Font("Times New Roman", 14F);
            label4.Location = new Point(7, 374);
            label4.Margin = new Padding(4, 0, 4, 0);
            label4.MaximumSize = new Size(200, 33);
            label4.MinimumSize = new Size(240, 33);
            label4.Name = "label4";
            label4.Size = new Size(240, 33);
            label4.TabIndex = 29;
            label4.Text = "Ramp, 1-10";
            label4.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // bGet
            // 
            bGet.Enabled = false;
            bGet.Font = new Font("Times New Roman", 14F);
            bGet.Location = new Point(17, 103);
            bGet.Margin = new Padding(4, 5, 4, 5);
            bGet.Name = "bGet";
            bGet.Size = new Size(221, 51);
            bGet.TabIndex = 35;
            bGet.Text = "Get Params";
            bGet.UseVisualStyleBackColor = true;
            bGet.Click += bGet_Click;
            // 
            // labelRX
            // 
            labelRX.Font = new Font("Times New Roman", 12F);
            labelRX.Location = new Point(58, 728);
            labelRX.Margin = new Padding(4, 0, 4, 0);
            labelRX.MinimumSize = new Size(140, 34);
            labelRX.Name = "labelRX";
            labelRX.Size = new Size(190, 34);
            labelRX.TabIndex = 36;
            labelRX.Text = "-";
            labelRX.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // labelTX
            // 
            labelTX.Font = new Font("Times New Roman", 12F);
            labelTX.Location = new Point(58, 691);
            labelTX.Margin = new Padding(4, 0, 4, 0);
            labelTX.MinimumSize = new Size(140, 34);
            labelTX.Name = "labelTX";
            labelTX.Size = new Size(190, 34);
            labelTX.TabIndex = 37;
            labelTX.Text = "-";
            labelTX.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // label5
            // 
            label5.Font = new Font("Times New Roman", 12F);
            label5.Location = new Point(4, 690);
            label5.Margin = new Padding(4, 0, 4, 0);
            label5.MinimumSize = new Size(40, 34);
            label5.Name = "label5";
            label5.Size = new Size(53, 34);
            label5.TabIndex = 39;
            label5.Text = "TX";
            label5.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // label6
            // 
            label6.Font = new Font("Times New Roman", 12F);
            label6.Location = new Point(4, 727);
            label6.Margin = new Padding(4, 0, 4, 0);
            label6.MinimumSize = new Size(40, 34);
            label6.Name = "label6";
            label6.Size = new Size(53, 34);
            label6.TabIndex = 38;
            label6.Text = "RX";
            label6.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // bSendTF
            // 
            bSendTF.Enabled = false;
            bSendTF.Font = new Font("Times New Roman", 14F);
            bSendTF.Location = new Point(106, 563);
            bSendTF.Margin = new Padding(4, 5, 4, 5);
            bSendTF.Name = "bSendTF";
            bSendTF.Size = new Size(71, 40);
            bSendTF.TabIndex = 42;
            bSendTF.Text = "set";
            bSendTF.UseVisualStyleBackColor = true;
            bSendTF.Click += bSendTF_Click;
            // 
            // txtTF
            // 
            txtTF.Font = new Font("Times New Roman", 12F);
            txtTF.Location = new Point(26, 565);
            txtTF.Margin = new Padding(4, 5, 4, 5);
            txtTF.Name = "txtTF";
            txtTF.Size = new Size(72, 35);
            txtTF.TabIndex = 41;
            txtTF.TextAlign = HorizontalAlignment.Center;
            // 
            // label7
            // 
            label7.Font = new Font("Times New Roman", 14F);
            label7.Location = new Point(7, 527);
            label7.Margin = new Padding(4, 0, 4, 0);
            label7.MaximumSize = new Size(200, 33);
            label7.MinimumSize = new Size(240, 33);
            label7.Name = "label7";
            label7.Size = new Size(240, 33);
            label7.TabIndex = 40;
            label7.Text = "TrainOff, 0-300";
            label7.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // bGetTF
            // 
            bGetTF.Enabled = false;
            bGetTF.Font = new Font("Times New Roman", 14F);
            bGetTF.Location = new Point(182, 563);
            bGetTF.Margin = new Padding(4, 5, 4, 5);
            bGetTF.Name = "bGetTF";
            bGetTF.Size = new Size(64, 40);
            bGetTF.TabIndex = 48;
            bGetTF.Text = "get";
            bGetTF.UseVisualStyleBackColor = true;
            bGetTF.Click += bGetTF_Click;
            // 
            // bGetTN
            // 
            bGetTN.Enabled = false;
            bGetTN.Font = new Font("Times New Roman", 14F);
            bGetTN.Location = new Point(182, 486);
            bGetTN.Margin = new Padding(4, 5, 4, 5);
            bGetTN.Name = "bGetTN";
            bGetTN.Size = new Size(64, 40);
            bGetTN.TabIndex = 47;
            bGetTN.Text = "get";
            bGetTN.UseVisualStyleBackColor = true;
            bGetTN.Click += bGetTN_Click;
            // 
            // bGetPR
            // 
            bGetPR.Enabled = false;
            bGetPR.Font = new Font("Times New Roman", 14F);
            bGetPR.Location = new Point(182, 409);
            bGetPR.Margin = new Padding(4, 5, 4, 5);
            bGetPR.Name = "bGetPR";
            bGetPR.Size = new Size(64, 40);
            bGetPR.TabIndex = 46;
            bGetPR.Text = "get";
            bGetPR.UseVisualStyleBackColor = true;
            bGetPR.Click += bGetPR_Click;
            // 
            // bGetPF
            // 
            bGetPF.Enabled = false;
            bGetPF.Font = new Font("Times New Roman", 14F);
            bGetPF.Location = new Point(182, 334);
            bGetPF.Margin = new Padding(4, 5, 4, 5);
            bGetPF.Name = "bGetPF";
            bGetPF.Size = new Size(64, 40);
            bGetPF.TabIndex = 45;
            bGetPF.Text = "get";
            bGetPF.UseVisualStyleBackColor = true;
            bGetPF.Click += bGetPF_Click;
            // 
            // bGetPW
            // 
            bGetPW.Enabled = false;
            bGetPW.Font = new Font("Times New Roman", 14F);
            bGetPW.Location = new Point(182, 260);
            bGetPW.Margin = new Padding(4, 5, 4, 5);
            bGetPW.Name = "bGetPW";
            bGetPW.Size = new Size(64, 40);
            bGetPW.TabIndex = 44;
            bGetPW.Text = "get";
            bGetPW.UseVisualStyleBackColor = true;
            bGetPW.Click += bGetPW_Click;
            // 
            // bGetPA
            // 
            bGetPA.Enabled = false;
            bGetPA.Font = new Font("Times New Roman", 14F);
            bGetPA.Location = new Point(182, 185);
            bGetPA.Margin = new Padding(4, 5, 4, 5);
            bGetPA.Name = "bGetPA";
            bGetPA.Size = new Size(64, 40);
            bGetPA.TabIndex = 43;
            bGetPA.Text = "get";
            bGetPA.UseVisualStyleBackColor = true;
            bGetPA.Click += bGetPA_Click;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(10F, 25F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(254, 768);
            Controls.Add(bGetTF);
            Controls.Add(bGetTN);
            Controls.Add(bGetPR);
            Controls.Add(bGetPF);
            Controls.Add(bGetPW);
            Controls.Add(bGetPA);
            Controls.Add(bSendTF);
            Controls.Add(txtTF);
            Controls.Add(label7);
            Controls.Add(label5);
            Controls.Add(label6);
            Controls.Add(labelTX);
            Controls.Add(labelRX);
            Controls.Add(bGet);
            Controls.Add(bSendTN);
            Controls.Add(txtTN);
            Controls.Add(label3);
            Controls.Add(bSendPR);
            Controls.Add(txtPR);
            Controls.Add(label4);
            Controls.Add(bStimOff);
            Controls.Add(bStimOn);
            Controls.Add(bSendPF);
            Controls.Add(txtPF);
            Controls.Add(label2);
            Controls.Add(bSendPW);
            Controls.Add(txtPW);
            Controls.Add(label1);
            Controls.Add(bSendPA);
            Controls.Add(labelOpcode);
            Controls.Add(txtPA);
            Controls.Add(bDisconnect);
            Controls.Add(bConnect);
            FormBorderStyle = FormBorderStyle.FixedSingle;
            Margin = new Padding(2);
            MaximizeBox = false;
            MdiChildrenMinimizedAnchorBottom = false;
            MinimizeBox = false;
            Name = "Form1";
            Text = "Form1";
            Load += Form1_Load;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        public Button bConnect;
        public Button bDisconnect;
        public TextBox txtPA;
        public Label labelOpcode;
        public Button bSendPA;
        public Label label1;
        public Button bSendPW;
        public TextBox txtPW;
        public Button bSendPF;
        public TextBox txtPF;
        public Label label2;
        public Button bStimOff;
        public Button bStimOn;
        public Button bSendTN;
        public TextBox txtTN;
        public Label label3;
        public Button bSendPR;
        public TextBox txtPR;
        public Label label4;
        public Button bGet;
        public Label labelRX;
        public Label labelTX;
        public Label label5;
        public Label label6;
        public Button bSendTF;
        public TextBox txtTF;
        public Label label7;
        public Button bGetTF;
        public Button bGetTN;
        public Button bGetPR;
        public Button bGetPF;
        public Button bGetPW;
        public Button bGetPA;
    }
}
