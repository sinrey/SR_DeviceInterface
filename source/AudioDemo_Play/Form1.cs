using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Windows.Forms;
using System.Threading;
using Sinrey.Device;
using System.Runtime.InteropServices;
//using GroovyCodecs.G711;
using Gimela.Net.Rtp;
using G711;

namespace AudioDemo_Play
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

        Thread dataThread;
        Thread dataEmergencyThread;

        DeviceListener deviceListener = null;
        private delegate void WorkProcessHandler(DeviceListener.Device d,string info, bool completed, int param);

        public Form1()
        {
            InitializeComponent();
        }

        //private delegate void LoginHandler();

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

        private void button5_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "mp3 file|*.mp3";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                textBox5.Text = openFileDialog1.FileName;
            }
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
                //DeviceListener.Device d = deviceListener.Find(item.id);

                ThreadParam ap = new ThreadParam();
                ap.id = item.id;
                ap.filename = textBox5.Text;
                ap.streamtype = "mp3";
                ap.volume = (int)numericUpDown1.Value;

                dataThread = new Thread(FilePlayThread);
                dataThread.IsBackground = true;
                dataThread.Start(ap);

                button3.Enabled = false;
                button4.Enabled = true;
            }
        }
        private void FilePlayProcess(DeviceListener.Device d, string info, bool completed, int param)
        {
            progressBar2.Value = param;
            if (completed)
            {
                progressBar2.Value = 0;
                button3.Enabled = true;
                button4.Enabled = false;
            }
            //Application.DoEvents();
        }

        private void FilePlayThread(object obj)
        {
            IPAddress localaddr = IPAddress.Parse("0.0.0.0");
            TcpListener tcpserver = new TcpListener(localaddr, 0);
            tcpserver.Start();
            IPEndPoint localep = (IPEndPoint)tcpserver.LocalEndpoint;

            WorkProcessHandler h = new WorkProcessHandler(FilePlayProcess);

            ThreadParam ap = (ThreadParam)obj;
            DeviceListener.Device d = deviceListener.Find(ap.id);
            if (d == null) return;
            int ret = deviceListener.FilePlayStart(d,"0.0.0.0",localep.Port,ap.streamtype,ap.volume,null);
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
                            else
                            {
                                Thread.Sleep(10);
                            }
                        }
                    }
                    tc.Close();
                }
                
                Thread.Sleep(500);
                deviceListener.FilePlayStop(d);
                this.BeginInvoke(h, d, ap.filename, true, 0);
                dataThread = null;
            }
            catch (ThreadAbortException abortException)
            {
                //this.BeginInvoke(h, ap.filename, true, 0);
                if (tc != null)tc.Close();
                
                dataThread = null;
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            if (comboBox1.SelectedItem != null)
            {
                DeviceItem item = (DeviceItem)comboBox1.SelectedItem;
                DeviceListener.Device d = deviceListener.Find(item.id);

                dataThread.Abort();
                //Thread.Sleep(100);
                //deviceListener.FilePlayStop(d);

                button3.Enabled = true;
                button4.Enabled = false;
            }
        }

        private void button6_Click(object sender, EventArgs e)
        {
            if (comboBox1.SelectedItem != null)
            {
                DeviceItem item = (DeviceItem)comboBox1.SelectedItem;
                DeviceListener.Device d = deviceListener.Find(item.id);

                int vol = (int)numericUpDown1.Value;
                deviceListener.AudioSetVolume(d, vol);
            }
        }

        private void comboBox4_DropDown(object sender, EventArgs e)
        {
            comboBox4.Items.Clear();
            foreach (DeviceItem item in listBox1.Items)
            {
                comboBox4.Items.Add(item);
            }
        }

        private void button10_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "wav file|*.wav";
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                textBox7.Text = openFileDialog1.FileName;
            }
        }

        private void button7_Click(object sender, EventArgs e)
        {
            if (comboBox4.SelectedItem != null)
            {
                DeviceItem item = (DeviceItem)comboBox4.SelectedItem;
                DeviceListener.Device d = deviceListener.Find(item.id);

                int vol = (int)numericUpDown2.Value;
                deviceListener.AudioSetVolume(d, vol);
            }
        }

        private void button9_Click(object sender, EventArgs e)
        {
            if (comboBox4.SelectedItem != null)
            {
                DeviceItem item = (DeviceItem)comboBox4.SelectedItem;
                //DeviceListener.Device d = deviceListener.Find(item.id);

                ThreadParam ap = new ThreadParam();
                ap.id = item.id;
                ap.filename = textBox7.Text;
                ap.streamtype = "g711-u";
                ap.volume = (int)numericUpDown2.Value;

                dataThread = new Thread(FileEmergencyPlayThread);
                dataThread.IsBackground = true;
                dataThread.Start(ap);

                button9.Enabled = false;
                button8.Enabled = true;
            }
        }

        private void FileEmergencyPlayProcess(DeviceListener.Device d, string info, bool completed, int param)
        {
            progressBar1.Value = param;
            if (completed)
            {
                progressBar1.Value = 0;
                button9.Enabled = true;
                button8.Enabled = false;
            }
            //Application.DoEvents();
        }
        private void FileEmergencyPlayThread(object obj)
        {
            UdpClient udpserver = new UdpClient(9999);
            IPEndPoint ep = new IPEndPoint(IPAddress.Any, 0);

            WorkProcessHandler h = new WorkProcessHandler(FileEmergencyPlayProcess);

            ThreadParam ap = (ThreadParam)obj;
            DeviceListener.Device d = deviceListener.Find(ap.id);
            if (d == null) return;
            int ret = deviceListener.FileEmergencyPlayStart(d, "0.0.0.0", 9999, ap.filename, ap.streamtype, ap.volume, d.id.ToString());
            if (ret != 0) return;

            FileStream fs = File.OpenRead(ap.filename);
            if (fs == null) return;
            
            try
            {
                long starttime = DateTime.Now.Ticks;
                string d_ip = null;
                int d_port = 0;
                while ((DateTime.Now.Ticks - starttime) < 3000 * 10000)//3000ms
                {
                    if (udpserver.Available > 0)
                    {
                        byte[] bs = udpserver.Receive(ref ep);
                        string s = System.Text.Encoding.Default.GetString(bs);
                        if (s.Contains(d.id.ToString()))
                        {
                            d_ip = ep.Address.ToString();
                            d_port = ep.Port;
                            break;
                        }
                    }
                }

                if(d_port != 0)
                {
                    int playfiletime = 0;
                    int time = System.Environment.TickCount;
                    int starttime1 = time;
                    bool eof = false;
                    byte[] fbs = new byte[320];
                    short[] pcm = new short[160];
                    int sequenceNumber = 0;
                    long timestamp = 0;
                    fs.Read(fbs, 0, 44);
                    long writebytes = 44;
                    while (!eof)
                    {
                        int runtime = System.Environment.TickCount - starttime1;
                        if((runtime + 500) >= playfiletime)
                        {
                            int readbytes = fs.Read(fbs, 0, fbs.Length);

                            IntPtr p = Marshal.UnsafeAddrOfPinnedArrayElement(fbs, 0);
                            Marshal.Copy(p, pcm, 0, 160);
                            //GroovyCodecs.G711.uLaw.ULawEncoder g711encode = new GroovyCodecs.G711.uLaw.ULawEncoder();
                            //byte[] ba = g711encode.Process(pcm);
                            byte[] ba = g711.g711Encode_ulwa(pcm);

                            RtpPacket packet = new RtpPacket(RtpPayloadType.G711_uLaw, sequenceNumber, timestamp, ba, ba.Length);
                            sequenceNumber++;
                            timestamp += 20;// ba.Length;
                            playfiletime += 20;
                            udpserver.Send(packet.ToArray(), packet.Length, d_ip, d_port);

                            writebytes += readbytes;
                            if (readbytes < fbs.Length) eof = true;
                            int process = (int)(100 * writebytes / fs.Length);
                            this.BeginInvoke(h, d, ap.filename, false, process);
                        }
                    }
                }
                udpserver.Close();
                Thread.Sleep(500);
                deviceListener.FileEmergencyPlayStop(d);
                this.BeginInvoke(h, d, ap.filename, true, 0);
            }
            catch (ThreadAbortException abortException)
            {
                udpserver.Close();
                dataThread = null;
            }
        }

        private void button8_Click(object sender, EventArgs e)
        {
            if (comboBox4.SelectedItem != null)
            {
                DeviceItem item = (DeviceItem)comboBox4.SelectedItem;
                DeviceListener.Device d = deviceListener.Find(item.id);

                dataThread.Abort();
                deviceListener.FileEmergencyPlayStop(d);

                button9.Enabled = true;
                button8.Enabled = false;
            }
        }


    }
}
