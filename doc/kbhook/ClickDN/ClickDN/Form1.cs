using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Runtime.InteropServices;

namespace ClickDN
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private GlobalHook hook;
        private void Form1_Load(object sender, EventArgs e)
        {
            this.textBox2.Text = "Press ESC to exit\r\n";
            hook = new GlobalHook("Untitled - Notepad");
            hook.OnMouseActivity += new MouseEventHandler(this.MyMouseClick);
            hook.OnKeyDown += new KeyEventHandler(this.MyKeyDown);
            hook.OnKeyPress += new KeyPressEventHandler(this.MyKeyPress);
            hook.OnKeyUp += new KeyEventHandler(this.MyKeyUp);
            hook.Start();
        }


        #region 键盘


        private void MyKeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Escape)
            {
                this.Close();
            }
           
        }
        private void MyKeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            this.textBox2.Text += "Key press: " + e.KeyChar + "\r\n";
        }

        private void MyKeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
        {
        }


        #endregion

        #region 鼠标
        private void MyMouseClick(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Left)
                this.textBox2.Text += "Mouse point: (x= " + e.X + ", y=" + e.Y + ")\r\n";
        }

        #endregion

        //最小化窗口
        private void Form1_Resize(object sender, EventArgs e)
        {
            if (this.WindowState == FormWindowState.Minimized)
            {
                this.Visible = false;
            }
        }
        //双击托盘图标
        private void notifyIcon1_DoubleClick(object sender, EventArgs e)
        {
            this.Visible = true;
            this.WindowState = FormWindowState.Normal;
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (hook != null)
                hook.Stop();
            this.notifyIcon1.Visible = false;
        }
        private void Form1_KeyUp(object sender, KeyEventArgs e)
        {
           
        }
    }
}
