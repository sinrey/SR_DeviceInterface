using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AudioDemo_SDCard
{
    public partial class FormProcess : Form
    {
        public FormProcess()
        {
            InitializeComponent();
        }

        public void SetInfo(string info)
        {
            label1.Text = info;               
        }
        public void SetProcess(int process)
        {
            progressBar1.Value = process;
            
        }
    }
}
