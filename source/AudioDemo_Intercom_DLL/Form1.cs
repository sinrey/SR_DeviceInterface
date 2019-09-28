using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Media;
using System.Windows.Forms;
using System.Threading;
using Sinrey.DeviceInterface;
using Sinrey.SoundCard;


namespace AudioDemo_Intercom_DLL
{
    public partial class Form1 : Form
    {
        class ThreadParam
        {
            public UInt32 userid;
            public string filename;

        }
        private delegate void AudioPowerUpdate(int mic, int speaker);
        double dbBase;
        public Form1()
        {
            InitializeComponent();


            DeviceInterfaceDll.EventConnect += OnConnected;
            DeviceInterfaceDll.EventLogin += OnLogin;
            DeviceInterfaceDll.SR_Init(this, 0, 8877);

            string exename = System.AppDomain.CurrentDomain.SetupInformation.ApplicationBase;
            string path = System.IO.Path.GetFullPath(exename);
            string txtfile = path + "..\\readme.txt";
            //FileStream fs = File.OpenRead(txtfile);
            //fs.readline
            if (System.IO.File.Exists(txtfile))
            {
                StreamReader sr = new StreamReader(txtfile);
                string lines = sr.ReadToEnd();
                textBox3.Text = lines;
                tabControl1.SelectedTab = tabPage2;
            }

            comboBox1.SelectedIndex = 0;
            
            uint ver = DeviceInterfaceDll.SR_GetVersion();
            uint verh = (ver & 0xff000000) >> 24;
            uint verm = (ver & 0xff0000) >> 16;
            uint verl = (ver & 0xffff);
            string s = string.Format("(dll ver:V{0:d}.{1:d}.{2:d})", verh, verm, verl);
            this.Text = this.Text + s;

            double p = 0;
            double a = 0;
            for (int i = 0; i < 1024; i++)
            {
                a = 32768 * Math.Sin(i * 2 * 3.14159 / 1024);
                p += a * a;
            }

            dbBase = p / 1024;
        }
        private void PlayNotify()
        {
            string exename = System.AppDomain.CurrentDomain.SetupInformation.ApplicationBase;
            string path = System.IO.Path.GetFullPath(exename);
            string wavfile = path + "..\\Notify.wav";

            if (System.IO.File.Exists(wavfile))
            {
                SoundPlayer player = new SoundPlayer();
                player.SoundLocation = wavfile;
                player.Load();
                player.Play();
            }
        }
        private void OnConnected(DeviceInterfaceDll.InterfaceMsg msg)
        {
            //string str = string.Format("msgtype={0:D};wparam={1:X};lparam={2:D}\r\n", msg.msg, msg.WParam, msg.LParam);
            //textBox1.AppendText(str);
            uint userid = msg.WParam;
            foreach (ListViewItem item in listView1.Items)
            {
                uint id = Convert.ToUInt32(item.SubItems[1].Text);
                if (id == userid)
                {
                    item.ImageIndex = 1;
                    break;
                }
            }
        }

        private void OnLogin(DeviceInterfaceDll.InterfaceMsg msg)
        {
            //string str = string.Format("msgtype={0:D};wpamra={1:X};lparam={2:D}\r\n", msg.msg, msg.WParam, msg.LParam);
            //textBox1.AppendText(str);
            uint userid = msg.WParam;
            foreach (ListViewItem item in listView1.Items)
            {
                uint id = Convert.ToUInt32(item.SubItems[1].Text);
                if (id == userid)
                {
                    item.ImageIndex = 2;
                    PlayNotify();

                    comboBox5.Text = id.ToString();
                    break;
                }
            }
        }

        private void Button1_Click(object sender, EventArgs e)
        {
            if (comboBox1.Text != null)
            {
                uint ret = DeviceInterfaceDll.SR_Login(comboBox1.Text, textBox1.Text, textBox2.Text);
                if (ret > 0)
                {
                    //ListViewItem item = new ListViewItem();
                    ListViewItem item = listView1.Items.Add("");
                    item.SubItems.Add(ret.ToString());
                    item.SubItems.Add(comboBox1.Text);
                    item.ImageIndex = 0;//white
                    //MessageBox.Show(string.Format("{0:S} login is successful, userid = {1:D}", textBox1.Text, ret));
                }
                else
                {
                    MessageBox.Show(string.Format("{0:S} login is false, error = {1:D}", textBox1.Text, ret));
                }

            }
        }

        private void ComboBox5_DropDown(object sender, EventArgs e)
        {
            comboBox5.Items.Clear();
            foreach (ListViewItem item in listView1.Items)
            {
                uint id = Convert.ToUInt32(item.SubItems[1].Text);
                comboBox5.Items.Add(id.ToString());
            }
        }

        Thread dataThread;
        private void Button3_Click(object sender, EventArgs e)
        {
            if (comboBox5.Text == "")
            {
                MessageBox.Show("please select the user id first.");
                return;
            }

            ThreadParam ap = new ThreadParam();
            ap.userid = Convert.ToUInt32(comboBox5.Text);

            dataThread = new Thread(IntercomThread);
            dataThread.IsBackground = true;
            dataThread.Start(ap);

            button3.Enabled = false;
            button4.Enabled = true;
        }
        private void Button4_Click(object sender, EventArgs e)
        {
            button3.Enabled = true;
            button4.Enabled = false;
            dataThread.Abort();
            dataThread = null;
        }
        private void AudioPowerUpdateHandler(int mic, int speaker)
        {
            if (speaker >= 0) progressBar1.Value = speaker;
            if (mic >= 0) progressBar2.Value = mic;
        }
        private void IntercomThread(object obj)
        {
            short[] micpack = new short[2048];
            int micpack_length = 0;

            short[] speakerpack = new short[2048];
            int speakerpack_length = 0;

            AudioPowerUpdate handler = new AudioPowerUpdate(AudioPowerUpdateHandler);

            UInt32 h;
            ThreadParam ap = (ThreadParam)obj;

            uint ret = DeviceInterfaceDll.SR_VoiceCom(out h, ap.userid);
            if (ret == DeviceInterfaceDll.RC_OK)
            {
                int r = SoundCardDLL.SoundCardInit(16000);
                if (r == 0)
                {
                    try
                    {
                        while(true)
                        {
                            micpack_length = 0;
                            speakerpack_length = 0;

                            if (SoundCardDLL.SoundCardWaitForInputData())
                            {
                                while (true)
                                {
                                    short[] pcmbuf = SoundCardDLL.SoundCardReadFrom(320);
                                    if (pcmbuf == null) break;

                                    if ((micpack_length + pcmbuf.Length) < 2048)
                                    {
                                        Array.Copy(pcmbuf, 0, micpack, micpack_length, pcmbuf.Length);
                                        micpack_length += pcmbuf.Length;
                                    }

                                    short[] pcmbuf1 = DeviceInterfaceDll.SR_VoiceComData(h, pcmbuf);

                                    if (pcmbuf1 != null)
                                    {
                                        SoundCardDLL.SoundCardWriteTo(pcmbuf1);

                                        if ((speakerpack_length + pcmbuf1.Length) < 2048)
                                        {
                                            Array.Copy(pcmbuf1, 0, speakerpack, speakerpack_length, pcmbuf1.Length);
                                            speakerpack_length += pcmbuf1.Length;
                                        }
                                    }
                                }
                            }

                            double mic_db = 0;
                            double speaker_db = 0;
                            if (micpack_length > 0)
                            {
                                double a = 0;
                                for (int i = 0; i < micpack_length; i++)
                                {
                                    a += micpack[i] * micpack[i];
                                }
                                mic_db = 20 * Math.Log10(a / (dbBase * micpack_length));
                                mic_db += 100;
                                if (mic_db < 0) mic_db = 0;
                                if (mic_db > 100) mic_db = 100;

                                this.Invoke(handler, Convert.ToInt32(mic_db), -1);
                            }

                            if (speakerpack_length > 0)
                            {
                                double a = 0;
                                for (int i = 0; i < speakerpack_length; i++)
                                {
                                    a += speakerpack[i] * speakerpack[i];
                                }
                                speaker_db = 20 * Math.Log10(a / (dbBase * speakerpack_length));
                                speaker_db += 100;
                                if (speaker_db < 0) speaker_db = 0;
                                if (speaker_db > 100) speaker_db = 100;

                                this.Invoke(handler, -1, Convert.ToInt32(speaker_db));
                            }
                        }
                    }
                    finally
                    {
                        DeviceInterfaceDll.SR_VoiceComClose(h, ap.userid);

                        SoundCardDLL.SoundCardClose();

                        dataThread = null;
                    }
                }
            }
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            DeviceInterfaceDll.SR_Cleanup();
        }
    }
}
