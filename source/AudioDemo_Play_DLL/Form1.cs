using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.IO;
using System.Media;
using System.Windows.Forms;
using Sinrey.DeviceInterface;
using System.Runtime.InteropServices;

namespace AudioDemo_Play_DLL
{
    public partial class Form1 : Form
    {
        class ThreadParam
        {
            public UInt32 userid;
            public string filename;

        }
        private delegate void WorkProcessHandler(uint id, string info, bool completed, int param);
        public Form1()
        {
            InitializeComponent();

            DeviceInterfaceDll.EventConnect += OnConnected;
            DeviceInterfaceDll.EventLogin += OnLogin;
            DeviceInterfaceDll.SR_Init(this, 8877);

            string exename = System.AppDomain.CurrentDomain.SetupInformation.ApplicationBase;
            string path = System.IO.Path.GetFullPath(exename);
            string txtfile = path + "readme.txt";
            //FileStream fs = File.OpenRead(txtfile);
            //fs.readline
            StreamReader sr = new StreamReader(txtfile);
            string lines = sr.ReadToEnd();
            textBox3.Text = lines;
            comboBox1.SelectedIndex = 0;
            tabControl1.SelectedTab = tabPage3 ;

            uint ver = DeviceInterfaceDll.SR_GetVersion();
            uint verh = (ver & 0xff000000) >> 24;
            uint verm = (ver & 0xff0000) >> 16;
            uint verl = (ver & 0xffff);
            string s = string.Format("(dll ver:V{0:d}.{1:d}.{2:d})", verh, verm, verl);
            this.Text = this.Text + s;
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            DeviceInterfaceDll.SR_Cleanup();
        }
        private void OnConnected(DeviceInterfaceDll.InterfaceMsg msg)
        {
            //string str = string.Format("msgtype={0:D};wparam={1:X};lparam={2:D}\r\n", msg.msg, msg.WParam, msg.LParam);
            //textBox1.AppendText(str);
            uint userid = msg.WParam;
            foreach(ListViewItem item in listView1.Items)
            {
                uint id = Convert.ToUInt32(item.SubItems[1].Text);
                if(id == userid)
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
                    PlaySuccessNotify();
                    break;
                }
            }
        }
        private void PlaySuccessNotify()
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
        private void Button1_Click(object sender, EventArgs e)
        {
            if(comboBox1.Text != null)
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

        private void ComboBox3_DropDown(object sender, EventArgs e)
        {
            comboBox3.Items.Clear();
            foreach (ListViewItem item in listView1.Items)
            {
                uint id = Convert.ToUInt32(item.SubItems[1].Text);
                comboBox3.Items.Add(id.ToString());
            }
        }

        private void Button5_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "mp3 file|*.mp3";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                textBox5.Text = openFileDialog1.FileName;
            }
        }

        private Thread PlayThread;
        private void Button3_Click(object sender, EventArgs e)
        {
            //uint id = Convert.ToUInt32(comboBox1.Text);
            if(comboBox1.Text == "")
            {
                MessageBox.Show("please select the user id first.");
                return;
            }
            //if(textBox5.Text =)
            if(System.IO.File.Exists(textBox5.Text) == false)
            {
                MessageBox.Show("the play file is not exist.");
                return;
            }
            ThreadParam ap = new ThreadParam();
            ap.userid = Convert.ToUInt32(comboBox3.Text);
            ap.filename = textBox5.Text;
            PlayThread = new Thread(FilePlayThread);
            PlayThread.IsBackground = true;
            PlayThread.Start(ap);
        }

        private void FilePlayProcess(uint userid, string info, bool completed, int param)
        {
            if (completed == false)
                progressBar2.Value = param;
            else
                progressBar2.Value = 0;
        }
        private void FilePlayThread(Object arg)
        {
            uint ret;
            ThreadParam ap = (ThreadParam)arg;
            WorkProcessHandler h = new WorkProcessHandler(FilePlayProcess);
            ret = DeviceInterfaceDll.SR_UploadFile_V40(out uint sHandle, ap.userid, ap.filename, DeviceInterfaceDll.UPLOAD_AUDIO_FILE_RELEASE, false);
            if (ret == DeviceInterfaceDll.RC_OK)
            {
                try
                {
                    int offset = 0;
                    byte[] dat = new byte[1024];
                    FileStream fs = File.OpenRead(ap.filename);

                    while (true)
                    {
                        int datlen = fs.Read(dat, 0, dat.Length);
                        if (datlen > 0)
                        {
                            DeviceInterfaceDll.SR_Upload_Process(sHandle, dat, datlen);
                            offset += datlen;
                        }
                        else
                        {
                            break;
                        }
                        this.Invoke(h, sHandle, "", false, (int)(100 * offset / fs.Length));
                    }
                }
                finally
                {
                    DeviceInterfaceDll.SR_UploadClose(sHandle);
                    this.Invoke(h, sHandle, "", true, 0);
                    PlayThread = null;
                }
            }
        }

        private void Button4_Click(object sender, EventArgs e)
        {
            if (PlayThread != null)
                PlayThread.Abort();
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

        private void Button10_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "wav file|*.wav";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                textBox7.Text = openFileDialog1.FileName;
            }
        }

        Thread EmergencyPlayThead;
        private void EmergencyFilePlayProcess(uint userid, string info, bool completed, int param)
        {
            if (completed == false)
                progressBar1.Value = param;
            else
                progressBar1.Value = 0;
        }
        private void Button9_Click(object sender, EventArgs e)
        {
            if (comboBox5.Text == "")
            {
                MessageBox.Show("please select the user id first.");
                return;
            }
            //if(textBox5.Text =)
            if (System.IO.File.Exists(textBox7.Text) == false)
            {
                MessageBox.Show("the play file is not exist.");
                return;
            }

            FileStream fs = File.OpenRead(textBox7.Text);
            byte[] dat = new byte[1024];
            int datlen = fs.Read(dat, 0, 44);
            if (datlen == 44)
            {
                int formattag = 256 * dat[21] + dat[20];
                int channel = 256 * dat[23] + dat[22];
                int samplespersec = 0x1000000 * dat[27] + 0x10000 * dat[26] + 0x100 * dat[25] + dat[24];
                int bits = 0x100 * dat[35] + dat[34];

                if ((formattag != 0x0001) || (channel != 1) || (samplespersec != 16000) || (bits != 16))
                {
                    MessageBox.Show("Unsupported file format.");
                    return;
                }
            }

            ThreadParam ap = new ThreadParam();
            ap.userid = Convert.ToUInt32(comboBox5.Text);
            ap.filename = textBox7.Text;
            EmergencyPlayThead = new Thread(EmergencyFilePlayThread);
            EmergencyPlayThead.IsBackground = true;
            EmergencyPlayThead.Start(ap);
        }

        private void EmergencyFilePlayThread(Object arg)
        {
            uint sHandle = 0 ;
            uint ret;
            ThreadParam ap = (ThreadParam)arg;
            WorkProcessHandler h = new WorkProcessHandler(EmergencyFilePlayProcess);
            try
            {
                ret = DeviceInterfaceDll.SR_StartEmergency(out sHandle, ap.userid);
                if (ret == DeviceInterfaceDll.RC_OK)
                {
                    int offset = 0;
                    byte[] dat = new byte[640];
                    short[] pcm = new short[320];
                    FileStream fs = File.OpenRead(ap.filename);
                    fs.Read(dat, 0, 44);//跳过文件头，wav文件头不一定是44字节，有些会更多，此演示程序不处理此差异。
                    while (true)
                    {
                        int datlen = fs.Read(dat, 0, 640);
                        if (datlen >= 640)
                        {
                            IntPtr p = Marshal.UnsafeAddrOfPinnedArrayElement(dat, 0);
                            Marshal.Copy(p, pcm, 0, pcm.Length);
                            DeviceInterfaceDll.SR_Emergency_Data(sHandle, pcm);
                            offset += 640;
                        }
                        else
                        {
                            break;
                        }
                        this.Invoke(h, sHandle, "", false, (int)(100 * offset / fs.Length));
                    }

                }
            }
            finally
            {
                DeviceInterfaceDll.SR_StopEmergency(sHandle, ap.userid);
                this.Invoke(h, sHandle, "", true, 0);
                PlayThread = null;
            }

        }
    }
}
