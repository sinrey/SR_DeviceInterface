using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Sinrey.Device;
using System.Threading;

namespace AudioDemo_SDCard
{
    public partial class Form1 : Form
    {
        private class DeviceItem
        {
            public string id;
            public string ip;
            public int port;
            public override string ToString()
            {
                //return id.ToString("X8") + "@" + ip + ":" + port.ToString();
                return id + "@" + ip + ":" + port.ToString();
            }
        }

        private class ThreadParam
        {
            public string id;
            public string mode;
            public int volume;
            public string inputsource;
            public int gain;
            public string aec;
            public string filename;
            public string streamtype;
        }

        //private delegate void LoginHandler();
        private delegate void WorkProcessHandler(DeviceListener.Device d, string info, bool completed, int param);

        Thread dataThread;
        DeviceListener deviceListener = null;
        public Form1()
        {
            InitializeComponent();
            listBox1.Items.Clear();
        }

        private void EventLogin(DeviceListener.Device d)
        {
            DeviceItem item = new DeviceItem();
            item.id = d.id;
            item.port = d.peerport;
            item.ip = d.peerip;
            listBox1.Items.Add(item);
        }

        private void EventLogout(DeviceListener.Device d)
        {
            foreach (DeviceItem item in listBox1.Items)
            {
                if (item.id == d.id)
                {
                    listBox1.Items.Remove(item);
                    break;
                }
            }
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
            deviceListener.RemoveAll();
            listBox1.Items.Clear();
        }

        private void comboBox1_DropDown(object sender, EventArgs e)
        {
            comboBox1.Items.Clear();
            foreach (DeviceItem item in listBox1.Items)
            {
                comboBox1.Items.Add(item);
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            if (comboBox1.SelectedItem != null)
            {
                DeviceItem item = (DeviceItem)comboBox1.SelectedItem;
                DeviceListener.Device d = deviceListener.Find(item.id);
                if (d != null)
                {
                    int ret = deviceListener.SDCardGetInfo(d, out string disk_size, out string free_size);
                    if (ret == 0)
                    {
                        label4.Text = disk_size;
                        label5.Text = free_size;
                    }
                }
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            int ret;
            if (comboBox1.SelectedItem != null)
            {
                listView1.Items.Clear();

                DeviceItem item = (DeviceItem)comboBox1.SelectedItem;
                DeviceListener.Device d = deviceListener.Find(item.id);
                if (d != null)
                {
                    string filename;
                    int filesize;
                    ret = deviceListener.SDCardGetFirstFile(d, out filename, out filesize);
                    while (ret == 0)
                    {
                        ListViewItem ls = new ListViewItem("");
                        ls.Text = (listView1.Items.Count + 1).ToString();
                        ls.SubItems.Add(filename);
                        ls.SubItems.Add(filesize.ToString());
                        listView1.Items.Add(ls);

                        ret = deviceListener.SDCardGetNextFile(d, out filename, out filesize);
                    }
                }
            }
        }

        private void button6_Click(object sender, EventArgs e)
        {
            int ret;
            if (comboBox1.SelectedItem != null)
            {
                DeviceItem item = (DeviceItem)comboBox1.SelectedItem;
                DeviceListener.Device d = deviceListener.Find(item.id);

                for (int i=listView1.SelectedItems.Count-1; i>=0; i--)
                {
                    ListViewItem ls = (ListViewItem)listView1.SelectedItems[i];
                    string filename = ls.SubItems[1].Text;
                    ret = deviceListener.SDCardDeleteFile(d, filename);
                    if (ret == 0)
                    {
                        listView1.Items.Remove(ls);
                    }
                }
            }
        }

        FormProcess subForm = null;

        private void uploadFileProcess(DeviceListener.Device d, string info, bool completed, int param)
        {
            if (!completed)
            {
                if (subForm != null)
                {
                    subForm.SetInfo(info);
                    subForm.SetProcess(param);
                }
            }
            else
            {
                subForm.Close();
                deviceListener.EventWorkProcess -= uploadFileProcess;
                subForm = null;
                if (param != 0) MessageBox.Show("upload file is fault, errcode is " + param.ToString());
            }
            Application.DoEvents();
        }

        private void button5_Click(object sender, EventArgs e)
        {
            if (comboBox1.SelectedItem != null)
            {
                DeviceItem item = (DeviceItem)comboBox1.SelectedItem;
                DeviceListener.Device d = deviceListener.Find(item.id);

                if (openFileDialog1.ShowDialog() == DialogResult.OK)
                {
                    string filepathname = openFileDialog1.FileName;

                    subForm = new FormProcess();
                    //subForm.Parent = this;
                    subForm.TopMost = true;
                    subForm.SetInfo(filepathname);
                    subForm.SetProcess(0);
                    subForm.Show();

                    ThreadParam ap = new ThreadParam();
                    ap.id = d.id;
                    ap.filename = filepathname;

                    dataThread = new Thread(SDFileUploadThread);
                    dataThread.IsBackground = true;
                    dataThread.Start(ap);
                }
            }
        }

        private void SDFileUploadThread(object obj)
        {
            IPAddress localaddr = IPAddress.Parse("0.0.0.0");
            TcpListener tcpserver = new TcpListener(localaddr, 0);
            tcpserver.Start();
            IPEndPoint localep = (IPEndPoint)tcpserver.LocalEndpoint;

            WorkProcessHandler h = new WorkProcessHandler(UploadFileProcess);

            ThreadParam ap = (ThreadParam)obj;
            DeviceListener.Device d = deviceListener.Find(ap.id);
            if (d == null) return;
            string fname = System.IO.Path.GetFileName(ap.filename);
            //int ret = deviceListener.FilePlayStart(d, "0.0.0.0", localep.Port, ap.streamtype, ap.volume, null);
            int ret = deviceListener.SDCardUploadFile(d,"0.0.0.0",localep.Port, fname);
            if (ret != 0) return;
            TcpClient tc = null;
            try
            {
                long starttime = DateTime.Now.Ticks;

                while ((DateTime.Now.Ticks - starttime) < 3000 * 10000)//3000ms
                {
                    if (tcpserver.Pending())
                    {
                        tc = tcpserver.AcceptTcpClient();
                        break;
                    }
                }
                if (tc != null)
                {
                    FileStream fs = File.OpenRead(ap.filename);
                    if (fs != null)
                    {
                        bool eof = false;
                        long writebytes = 0;
                        byte[] fbs = new byte[1024];
                        NetworkStream tcs = tc.GetStream();
                        while (!eof)
                        {
                            if (tcs.CanWrite)
                            {
                                int readbytes = fs.Read(fbs, 0, fbs.Length);

                                tcs.Write(fbs, 0, readbytes);
                                writebytes += readbytes;

                                if (readbytes < fbs.Length) eof = true;

                                int process = (int)(100 * writebytes / fs.Length);

                                this.BeginInvoke(h, d, ap.filename, false, process);
                            }
                        }
                    }
                    tc.Close();
                }
                
                this.BeginInvoke(h, d, ap.filename, true, 0);
                dataThread = null;
            }
            catch (ThreadAbortException abortException)
            {
                //this.BeginInvoke(h, ap.filename, true, 0);
                if (tc != null) tc.Close();

                dataThread = null;
            }
        }

        private void listView1_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listView1.SelectedItems)
            {
                label7.Text = item.SubItems[1].Text;
            }
        }

        private void UploadFileProcess(DeviceListener.Device d, string info, bool completed, int param)
        {
            if (!completed)
            {
                if (subForm != null)
                {
                    subForm.SetInfo(info);
                    subForm.SetProcess(param);
                }
            }
            else
            {
                if (subForm != null)
                {
                    subForm.Close();
                    subForm = null;
                }
            }
            //Application.DoEvents();
        }

        private void button7_Click(object sender, EventArgs e)
        {
            if (comboBox1.SelectedItem != null)
            {
                DeviceItem item = (DeviceItem)comboBox1.SelectedItem;
                DeviceListener.Device d = deviceListener.Find(item.id);

                subForm = new FormProcess();
                //subForm.Parent = this;
                //subForm.TopMost = true;
                subForm.SetInfo("");
                subForm.SetProcess(0);
                subForm.Show();

                ThreadParam ap = new ThreadParam();
                ap.filename = label7.Text;
                ap.id = d.id;
                ap.volume = (int)numericUpDown1.Value;

                dataThread = new Thread(SDFilePlayThread);
                dataThread.IsBackground = true;
                dataThread.Start(ap);
            }
                
        }
        private void SDFilePlayThread(object obj)
        {
            IPAddress localaddr = IPAddress.Parse("0.0.0.0");
            TcpListener tcpserver = new TcpListener(localaddr, 0);
            tcpserver.Start();
            IPEndPoint localep = (IPEndPoint)tcpserver.LocalEndpoint;

            WorkProcessHandler h = new WorkProcessHandler(UploadFileProcess);

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
                    if(ret2 == 0)
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

        private void button8_Click(object sender, EventArgs e)
        {
            if (comboBox1.SelectedItem != null)
            {
                DeviceItem item = (DeviceItem)comboBox1.SelectedItem;
                DeviceListener.Device d = deviceListener.Find(item.id);

                deviceListener.SDCardPlayFileStop(d);
            }
        }
    }
}
