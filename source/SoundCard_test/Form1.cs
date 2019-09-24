using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Sinrey.SoundCard;

namespace SoundCard_test
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            textBox1.AppendText("Result code and infomation\r\n");

            //foreach (string s in SoundCard.RESULT_CODE)
            for (int i = 0; i < SoundCardDLL.RESULT_CODE.Length; i++)
            {
                textBox1.AppendText(i.ToString() + ":" + SoundCardDLL.RESULT_CODE[i] + "\r\n");
            }
        }

        private void Button1_Click(object sender, EventArgs e)
        {
            int ret = SoundCardDLL.SoundCardInit(16000);
            if (ret == 0)
            {
                MessageBox.Show("soundcard open is ok");
            }
            else if (ret < 0)
            {
                MessageBox.Show("memory lack");
            }
            else
            {
                int hi, low;
                hi = ret / 1000;
                low = ret % 1000;
                if (hi == 1)
                {
                    MessageBox.Show("waveinopen fault, result code=" + low.ToString());
                }
                else if (hi == 5)
                {
                    MessageBox.Show("waveoutopen fault, result code=" + low.ToString());
                }
                else
                {
                    MessageBox.Show("fault, stepid=" + hi.ToString() + ", result code=" + low.ToString());
                }
                
            }
            SoundCardDLL.SoundCardClose();
        }
    }
}
