using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Configuration;

namespace SafeSkype
{
    /// <summary>
    /// The main form for the application.
    /// All the main Skype event handlers are here
    /// </summary>
    public partial class Form1 : Form
    {
        #region Constructor

        public Form1()
        {
            InitializeComponent();
        }

        #endregion

        #region Class Data

        // keyboard hook
        private SkypeHook _keyhook;
        private bool _hooked = false; // is key hook started or not

        // token pairs
        private Dictionary<string, string> _tokens = new Dictionary<string, string>(); // key=friend name, value=password

        #endregion

        #region Startup

        private void Form1_Load(object sender, EventArgs e)
        {
            // Start up our application
            WriteToLog("SafeSkype plugin v2.1");

            // Attach to Keyboard hook
            this._keyhook = new SkypeHook();
            WriteToLog("  interface: KbHook");

            // Form event handlers
            this.SizeChanged += new System.EventHandler(this.frmManager_SizeChanged);
            this.nfiMinimize.MouseDoubleClick += new MouseEventHandler(this.nfiMinimize_MouseDoubleClick);
        }

        #endregion


        #region Logging

        private void WriteToLog(string pMessage)
        {
            this.txtLog.Text += DateTime.Now.ToString("HH:mm:ss:") + " " + pMessage + Environment.NewLine;
            this.txtLog.ScrollToCaret();
        }

        private void WriteToLog()
        {
            this.txtLog.Text += Environment.NewLine;
            this.txtLog.ScrollToCaret();
        }

        #endregion

        #region Form Events

        private void frmManager_SizeChanged(object sender, EventArgs e)
        {
            // Have they minimized the form?
            if (this.WindowState == FormWindowState.Minimized)
            {
                // Yes, minimize to the system tray
                this.ShowInTaskbar = false;
                this.nfiMinimize.Visible = true;
            }
            else
            {
                // No, hide the icon and show the app
                this.ShowInTaskbar = true;
                this.nfiMinimize.Visible = false;
                this.Focus();
            }
        }

        private void nfiMinimize_MouseDoubleClick(object sender, EventArgs e)
        {
            // Re-show the application
            this.ShowInTaskbar = true;
            this.nfiMinimize.Visible = false;
            this.WindowState = FormWindowState.Normal;
            this.Focus();
        }

        public bool SetCurrentToken()
        {
            string remote = this._keyhook.GetRemoteUser();
            if (string.IsNullOrEmpty(remote))
            {
                //WriteToLog("WARNING: no conversation was chosen!");
                return false;
            }
            //WriteToLog("  conversation partner: " + remote);

            string old_pwd = this._keyhook.GetPassword();
            WriteToLog("  existing token: " + old_pwd);

            if (_tokens.ContainsKey(remote))
            {
                string pwd = _tokens[remote];
                if (pwd != old_pwd)
                {
                    this._keyhook.SetPassword(pwd);
                    WriteToLog("  token was changed to: " + pwd);
                }
                return true;
            }
            else
            {
                _tokens[remote] = old_pwd;
                WriteToLog("  token was saved.");
                return true;
            }
        }


        // start or stop encryption
        private void button1_Click(object sender, EventArgs e)
        {
            if (this.button1.Text == "Start")
            {
                this._keyhook.Start();
                string user = this._keyhook.GetLocalUser();
                string remote = this._keyhook.GetRemoteUser();
                if (string.IsNullOrEmpty(remote))
                {
                    WriteToLog("WARNING: no conversation was chosen!");
                    this._keyhook.End();
                    return;
                }

                this._hooked = true;
                WriteToLog(user + " starts safe conversation with " + remote + "...");

                // reset password
                SetCurrentToken();

                this.button1.Text = "Stop";
            }
            else
            {
                this._hooked = false;

                string user = this._keyhook.GetLocalUser(); 
                this._keyhook.End();
                WriteToLog( user + " ends safe conversation.");

                this.button1.Text = "Start";
            }
        }

        // security setting
        private void button2_Click(object sender, EventArgs e)
        {
            // init a data grid view
            frmSetting setting = new frmSetting();
            {
                foreach (string p in this._tokens.Keys)
                {
                    string[] row = new string[] { p, this._tokens[p]};
                    setting.dataGridView1.Rows.Add(row);
                }
            }

            // upon setting OK, make a change
            //if (setting.ShowDialog(this) == DialogResult.OK)
            setting.ShowDialog();
            if ( setting.Done )
            {
                this._tokens = new Dictionary<string, string>();

                for (int i= 0; i< setting.dataGridView1.Rows.Count; i++)
                {
                    string p = (string) setting.dataGridView1.Rows[i].Cells[0].Value;
                    string token = (string)setting.dataGridView1.Rows[i].Cells[1].Value;
                    if (p == null) continue;

                    this._tokens[p] = token;
                }

                if (this._hooked)
                {
                    // changing the token for current conversation
                    // note: but if you change the conversation, you must restart the hook!
                    SetCurrentToken();
                }
            }
        }

        #endregion

    }
}