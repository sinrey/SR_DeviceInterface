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
using Sinrey.DeviceInterface;
using System.Windows.Forms;
using System.Threading;

namespace AudioDemo_Update_DLL
{
    public partial class Form1 : Form
    {
        class ThreadParam
        {
            public UInt32 id;
            public string bin_filename;
            public string pak_filename;

        }
        private delegate void DelegateShowProcess(string fname, int process);
        Form2 FormProgress = null;
        public Form1()
        {
            InitializeComponent();

            DeviceInterfaceDll.EventConnect += OnConnected;
            DeviceInterfaceDll.EventLogin += OnLogin;
            DeviceInterfaceDll.SR_Init(this, 0, 8877);

            string exename = System.AppDomain.CurrentDomain.SetupInformation.ApplicationBase;
            string path = System.IO.Path.GetFullPath(exename);
            string txtfile = path + "..//readme.txt";
            //FileStream fs = File.OpenRead(txtfile);
            //fs.readline
            if (System.IO.File.Exists(txtfile))
            {
                StreamReader sr = new StreamReader(txtfile);
                string lines = sr.ReadToEnd();
                textBox5.Text = lines;
                tabControl1.SelectedTab = tabPage1;
            }

            comboBox1.SelectedIndex = 0;

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
                    comboBox3.Text = id.ToString();
                    break;
                }
            }
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

        private void button1_Click(object sender, EventArgs e)
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

        private void comboBox3_DropDown(object sender, EventArgs e)
        {
            comboBox3.Items.Clear();
            foreach (ListViewItem item in listView1.Items)
            {
                uint id = Convert.ToUInt32(item.SubItems[1].Text);
                comboBox3.Items.Add(id.ToString());
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "BIN file|*.bin";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                textBox3.Text = openFileDialog1.FileName;
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "PAK file|*.pak";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                textBox4.Text = openFileDialog1.FileName;
            }
        }
        private void ShowProgress(string fname, int percent)
        {
            if (FormProgress != null)
            {
                if (percent <= 100)
                {
                    FormProgress.SetLabel(fname);
                    FormProgress.SetProgress(percent);
                }
                else
                {
                    FormProgress.Close();
                    FormProgress = null;
                }
            }
        }
        private void UpdateThread(object obj)
        {
            ThreadParam ap = (ThreadParam)obj;
            Delegate h = new DelegateShowProcess(ShowProgress);

            try
            {
                uint ret;
                uint handle;
                ret = DeviceInterfaceDll.SR_Update(out handle, ap.id, 0, ap.bin_filename);
                if (ret == DeviceInterfaceDll.RC_OK)
                {
                    try
                    {
                        int count = 0;
                        byte[] bdata = new byte[1024];
                        FileStream fs = File.OpenRead(ap.bin_filename);
                        if (fs != null)
                        {
                            int rlen;
                            do
                            {
                                rlen = fs.Read(bdata, 0, bdata.Length);
                                if (rlen > 0)
                                {
                                    count += rlen;
                                    DeviceInterfaceDll.SR_UpdateData(handle, bdata, rlen);
                                    this.Invoke(h, ap.bin_filename, (int)(100 * count / fs.Length));
                                }
                            }
                            while (rlen >= bdata.Length);
                        }
                    }
                    finally
                    {
                        DeviceInterfaceDll.SR_UpdateClose(handle);
                        //this.Invoke(h, "", 101);
                    }
                }

                ret = DeviceInterfaceDll.SR_Update(out uint pak_handle, ap.id, 1, ap.pak_filename);
                if (ret == DeviceInterfaceDll.RC_OK)
                {
                    try
                    {
                        int count = 0;
                        byte[] bdata = new byte[1024];
                        FileStream fs = File.OpenRead(ap.pak_filename);
                        if (fs != null)
                        {
                            int rlen;
                            do
                            {
                                rlen = fs.Read(bdata, 0, bdata.Length);
                                if (rlen > 0)
                                {
                                    count += rlen;
                                    DeviceInterfaceDll.SR_UpdateData(pak_handle, bdata, rlen);
                                    this.Invoke(h, ap.pak_filename, (int)(100 * count / fs.Length));
                                }
                            }
                            while (rlen >= bdata.Length);
                        }
                    }
                    finally
                    {
                        DeviceInterfaceDll.SR_UpdateClose(handle);
                    }
                }
            }
            finally
            {
                this.Invoke(h, "", 101);
            }
        }

        Thread UpdateThreadHandle;
        private void button2_Click(object sender, EventArgs e)
        {
            FormProgress = new Form2();
            FormProgress.TopMost = true;
            FormProgress.StartPosition = FormStartPosition.CenterScreen;

            //FormProgress.CloseHandler = new Form2.DelegateCloseHandler(CloseProgressForm);
            //FormProgress.SetLabel("upload file:" + filename);
            FormProgress.id = Convert.ToUInt32(comboBox3.Text);
            FormProgress.bin_filename = textBox3.Text;
            FormProgress.pak_filename = textBox4.Text;
            FormProgress.Show();

            /*
            ThreadParam ap = new ThreadParam();
            ap.id = Convert.ToUInt32(comboBox3.Text);
            ap.bin_filename = textBox3.Text;
            ap.pak_filename = textBox4.Text;

            UpdateThreadHandle = new Thread(UpdateThread);
            UpdateThreadHandle.IsBackground = true;
            UpdateThreadHandle.Start(ap);
            */
        }


    }
}
