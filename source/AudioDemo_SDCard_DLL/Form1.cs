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
using System.Threading;
using System.Windows.Forms;
using Sinrey.DeviceInterface;

namespace AudioDemo_SDCard_DLL
{
    public partial class Form1 : Form
    {
        class ThreadParam
        {
            public uint id;
            public string filename;
            public uint volume;

        }
        private delegate void DelegateShowProcess(int process);
        Form2 FormProgress = null;
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
                sr.Close();
            }

            comboBox1.SelectedIndex = 0;

            uint ver = DeviceInterfaceDll.SR_GetVersion();
            uint verh = (ver & 0xff000000) >> 24;
            uint verm = (ver & 0xff0000) >> 16;
            uint verl = (ver & 0xffff);
            string s = string.Format("(dll ver:V{0:d}.{1:d}.{2:d})", verh, verm, verl);
            this.Text = this.Text + s;
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
                    comboBox2.Text = id.ToString();
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

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            DeviceInterfaceDll.SR_Cleanup();
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

        private void ComboBox2_DropDown(object sender, EventArgs e)
        {
            comboBox2.Items.Clear();
            foreach (ListViewItem item in listView1.Items)
            {
                uint id = Convert.ToUInt32(item.SubItems[1].Text);
                comboBox2.Items.Add(id.ToString());
            }
        }

        private void Button3_Click(object sender, EventArgs e)
        {
            if (comboBox2.Text == "")
            {
                MessageBox.Show("please select the user id first.");
                return;
            }
            uint id = Convert.ToUInt32(comboBox2.Text);
            uint ret = DeviceInterfaceDll.SR_GetCapacity(id, out DeviceInterfaceDll.SDInformation scobj);
            if (ret == DeviceInterfaceDll.RC_OK)
            {
                listView2.Items.Clear();

                label4.Text = scobj.totalCapacity.ToString() + " MB";
                label5.Text = scobj.surplusCapacity.ToString() + " MB";

                //foreach (DeviceInterfaceDll.FileList fs in scobj.audioFileList)
                for (int i = 0; i < scobj.audioFileList.Length; i++)
                {
                    ListViewItem ls = listView2.Items.Add((i + 1).ToString());
                    ls.SubItems.Add(scobj.audioFileList[i].fileName);
                    ls.SubItems.Add(scobj.audioFileList[i].fileSize.ToString());
                }
            }
            else
            {
                MessageBox.Show("read information of sd is fault.");
            }

        }

        private void ListView2_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listView2.SelectedItems)
            {
                label7.Text = item.SubItems[1].Text;
            }
        }

        Thread dataThread;
        private void Button7_Click(object sender, EventArgs e)
        {
            if (comboBox2.Text == "")
            {
                MessageBox.Show("please select the user id first.");
                return;
            }
            
            FormProgress = new Form2();
            FormProgress.TopMost = true;
            FormProgress.StartPosition = FormStartPosition.CenterScreen;

            FormProgress.CloseHandler = new Form2.DelegateCloseHandler(CloseProgressForm);
            FormProgress.SetLabel(label7.Text);
            FormProgress.Show();

            ThreadParam ap = new ThreadParam();
            ap.id = Convert.ToUInt32(comboBox2.Text);
            ap.filename = label7.Text;
            ap.volume = (uint)numericUpDown1.Value;

            dataThread = new Thread(SDPlayFileThread);
            dataThread.IsBackground = true;
            dataThread.Start(ap);
        }


        private void CloseProgressForm()
        {
            
            if (dataThread != null)
            {
                if (dataThread.ThreadState != ThreadState.Stopped)
                {
                    dataThread.Abort();
                }
                dataThread = null;
            }

            if (FormProgress != null)
            {
                FormProgress.Close();
                FormProgress = null;
            }
        }
        
        private void ShowProgress(int percent)
        {
            if (FormProgress != null)
            {
                if (percent <= 100)
                    FormProgress.SetProgress(percent);
                else
                {
                    FormProgress.Close();
                    FormProgress = null;
                }
            }
        }
        private void SDPlayFileThread(object obj)
        {
            ThreadParam ap = (ThreadParam)obj;

            Delegate h = new DelegateShowProcess(ShowProgress);

            uint ret = DeviceInterfaceDll.SR_PutFile(ap.id, ap.filename, ap.volume);
            if (ret == DeviceInterfaceDll.RC_OK)
            {
                try
                {
                    while (true)
                    {
                        ret = DeviceInterfaceDll.SR_PutFileStatus(ap.id, out uint nProcess);
                        if (ret == DeviceInterfaceDll.RC_OK)
                        {
                            int process = (int)nProcess;
                            this.Invoke(h, process);
                        }
                        else
                        {
                            break;
                        }
                        Thread.Sleep(1000);
                    }
                    //this.Invoke(h, 101);

                }
                catch (ThreadAbortException e)
                {
                    DeviceInterfaceDll.SR_PutFileClose(ap.id);
                }
                finally
                {
                    this.Invoke(h, 101);
                    dataThread = null;
                }
            }
        }

        private void Button6_Click(object sender, EventArgs e)
        {
            if (comboBox2.Text == "")
            {
                MessageBox.Show("please select the user id first.");
                return;
            }
            uint id = Convert.ToUInt32(comboBox2.Text);
            for (int i = listView2.SelectedItems.Count - 1; i >= 0; i--)
            {
                ListViewItem ls = (ListViewItem)listView2.SelectedItems[i];
                string filename = ls.SubItems[1].Text;
                uint ret = DeviceInterfaceDll.SR_DeleteFile(id, filename);
                if (ret == 0)
                {
                    listView2.Items.Remove(ls);
                }
            }
        }

        Thread uploadThread = null;
        private void Button5_Click(object sender, EventArgs e)
        {
            if (comboBox2.Text == "")
            {
                MessageBox.Show("please select the user id first.");
                return;
            }

            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                string filename = openFileDialog1.FileName;

                FormProgress = new Form2();
                FormProgress.TopMost = true;
                FormProgress.StartPosition = FormStartPosition.CenterScreen;

                //FormProgress.CloseHandler = new Form2.DelegateCloseHandler(CloseProgressForm);
                FormProgress.SetLabel("upload file:"+filename) ;
                FormProgress.Show();

                ThreadParam ap = new ThreadParam();
                ap.id = Convert.ToUInt32(comboBox2.Text);
                ap.filename = filename;

                uploadThread = new Thread(SDCardUploadFileThread);
                uploadThread.IsBackground = true;
                uploadThread.Start(ap);
            }
        }

        private void SDCardUploadFileThread(object obj)
        {
            ThreadParam ap = (ThreadParam)obj;
            Delegate h = new DelegateShowProcess(ShowProgress);

            uint ret = DeviceInterfaceDll.SR_UploadFile(out uint handle, ap.id, ap.filename, true);
            if (ret == DeviceInterfaceDll.RC_OK)
            {
                try
                {
                    long count = 0;
                    byte[] bdata = new byte[1024];
                    FileStream fs = File.OpenRead(ap.filename);
                    if (fs != null)
                    {
                        int rlen;
                        do
                        {
                            rlen = fs.Read(bdata, 0, bdata.Length);
                            if (rlen > 0)
                            {
                                count += rlen;
                                DeviceInterfaceDll.SR_UploadFileData(handle, bdata, rlen);
                                this.Invoke(h, (int)(100 * count / fs.Length));
                            }
                        }
                        while (rlen >= bdata.Length);
                    }
                }
                catch (ThreadAbortException e)
                { }
                finally
                {
                    DeviceInterfaceDll.SR_UploadClose(handle);
                    this.Invoke(h, 101);
                }
            }

        }
    }
}
