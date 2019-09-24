using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Windows.Forms;
using System.Media;
using Sinrey.Device;
using NASetupDLL;
using System.Threading;

namespace test
{
    public partial class Form1 : Form
    {
        private class DeviceItem
        {
            public uint id;
            public string ip;
            public int port;
            public override string ToString()
            {
                return id.ToString("X8") + "@" + ip + ":" + port.ToString();
            }
        }

        private class ThreadParam
        {
            public uint id;
            public string mode;
            public int volume;
            public string inputsource;
            public int gain;
            public string aec;
            public string filename;
            public string streamtype;
        }
        //delegate void WorkProcessHandler(int process);
        private delegate void WorkProcessHandler(DeviceListener.Device d, string info, bool completed, int param);
        uint DeviceID = 0;
        //DeviceItem gDevItem = null;
        DeviceListener deviceListener = null;

        private void PlaySuccessNotify()
        {
            string exename = System.AppDomain.CurrentDomain.SetupInformation.ApplicationBase;
            string path = System.IO.Path.GetFullPath(exename);
            string wavfile = path + "..\\Notify.wav";

            SoundPlayer player = new SoundPlayer();
            player.SoundLocation = wavfile;
            player.Load();
            player.Play();
        }

        private void PlayFaultNotify()
        {

        }

        public Form1()
        {
            InitializeComponent();
        }

        private void EventLogin(DeviceListener.Device d)
        {
            if (DeviceID != 0) return;

            DeviceItem item = new DeviceItem();
            item.id = d.id;
            item.port = d.peerport;
            item.ip = d.peerip;

            DeviceID = d.id;
            label4.Text = item.ToString();

            PlaySuccessNotify();

            panel1.BackColor = Color.LimeGreen;
        }

        private void EventLogout(DeviceListener.Device d)
        {
            
        }
        private void button1_Click(object sender, EventArgs e)
        {
            deviceListener = new DeviceListener(this, int.Parse(textBox1.Text), textBox2.Text, textBox3.Text);

            deviceListener.EventLogin += new DeviceListener.LoginHandler(EventLogin);
            deviceListener.EventLogout += new DeviceListener.LoginHandler(EventLogout);

            button1.Enabled = false;
        }

        private void button2_Click(object sender, EventArgs e)
        {

        }

        private void button9_Click(object sender, EventArgs e)
        {
            DeviceListener.Device d = deviceListener.Find(DeviceID);
            if (d == null)
            {
                panel2.BackColor = SystemColors.ControlDark;
                label5.Text = string.Format("DEVICE ID({0:D8} IS NOT EXITS)", d.id);

                return;
            }

            panel2.BackColor = SystemColors.Control;
            _DeviceEx2 DeviceEx = new _DeviceEx2();
            _Param param = new _Param();
            int okpack = 0; 
            int faultpack = 0;
            for (int i=0;i<1000;i++)
            {
                int ret = NASetupDLL.NaSetup.NpReadSettingEx(d.peerip, "0.0.0.0.0.0", ref DeviceEx, ref param);

                if (ret == 1)
                {
                    okpack++;
                    
                }
                else
                {
                    faultpack++;
                }
                label5.Text = string.Format("PING SEND={0:D}, RECV={1:D}, DISCARD:{2:D}",i+1,okpack,faultpack);
                if (faultpack > 4) break;
                Application.DoEvents();
            }
            if (okpack == 1000)
            {
                panel2.BackColor = Color.LimeGreen;
                PlaySuccessNotify();
            }
            else
            {
                panel2.BackColor = Color.Red;
                PlayFaultNotify();
            }
        }

        Thread dataThread;
        private void button8_Click(object sender, EventArgs e)
        {
            //DeviceItem item = (DeviceItem)comboBox1.SelectedItem;
            if (DeviceID == 0) return;
            DeviceListener.Device d = deviceListener.Find(DeviceID);

            panel3.BackColor = SystemColors.Control;

            ThreadParam ap = new ThreadParam();
            ap.filename = "product_test.mp3";
            ap.id = DeviceID;
            ap.volume = 75;

            label6.Text = ap.filename;

            dataThread = new Thread(SDFilePlayThread);
            dataThread.IsBackground = true;
            dataThread.Start(ap);
        }

        private void SDPlayFileProcess(DeviceListener.Device d,string info,bool completed,int process)
        {
            progressBar1.Value = process;
            if (completed)
            {
                panel3.BackColor = Color.LimeGreen;
                PlaySuccessNotify();
            }
        }

        private void SDFilePlayThread(object obj)
        {
            IPAddress localaddr = IPAddress.Parse("0.0.0.0");
            TcpListener tcpserver = new TcpListener(localaddr, 0);
            tcpserver.Start();
            IPEndPoint localep = (IPEndPoint)tcpserver.LocalEndpoint;

            WorkProcessHandler h = new WorkProcessHandler(SDPlayFileProcess);

            ThreadParam ap = (ThreadParam)obj;
            DeviceListener.Device d = deviceListener.Find(ap.id);
            if (d == null) return;
            string fname = System.IO.Path.GetFileName(ap.filename);
            int ret = deviceListener.SDCardPlayFile(d, fname, ap.volume);
            if (ret != 0) return;
            TcpClient tc = null;
            try
            {
                while (true)
                {
                    int ret2 = deviceListener.SDCardGetPlayFileStatus(d, out string filename, out int runtime, out int process);
                    if (ret2 == 0)
                    {
                        this.BeginInvoke(h, d, filename, false, process);
                    }
                    else
                    {
                        break;
                    }
                    Thread.Sleep(1000);
                }
                this.BeginInvoke(h, d, null, true, 0);

                dataThread = null;
            }
            catch (ThreadAbortException abortException)
            {
                dataThread = null;
            }
        }

        private void button7_Click(object sender, EventArgs e)
        {
            panel4.BackColor = SystemColors.Control;

            Form2 f2 = new Form2();
            f2.deviceListener = deviceListener;
            f2.DeviceID = DeviceID;
            //f2.Device = deviceListener.Find(DeviceID);
            f2.ShowDialog();

            panel4.BackColor = Color.LimeGreen;
        }

        private void button2_Click_1(object sender, EventArgs e)
        {
            //string appname = "c:\\arp.ext -d"
            //System.Diagnostics.Process.Start()
            DeviceID = 0;
            deviceListener.RemoveAll();
            label4.Text = "";
            label5.Text = "";
            label6.Text = "";
            panel1.BackColor = SystemColors.Control;
            panel2.BackColor = SystemColors.Control;
            panel3.BackColor = SystemColors.Control;
            panel4.BackColor = SystemColors.Control;
        }
    }
}
