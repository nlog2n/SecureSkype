using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace SafeSkype
{
    public partial class frmSetting : Form
    {
        public bool Done = false;

        public frmSetting()
        {
            InitializeComponent();
        }

        // Cancel
        private void button2_Click(object sender, EventArgs e)
        {
            Close();
        }

        // OK
        private void button1_Click(object sender, EventArgs e)
        {
            Done = true;
            Close();
        }
    }
}
