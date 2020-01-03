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
using System.Threading;
using System.IO;

namespace test
{
    public partial class Form1 : Form
    {
        private class DeviceItem
        {
            public string id;
            public string ip;
            public int port;
            public string version;
            public string devicetype;
            public override string ToString()
            {
                //return id.ToString("X8") + "@" + ip + ":" + port.ToString();
                //return id+"["+devicetype+"-"+version+"]" + "@" + ip + ":" + port.ToString();
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
        private delegate void WorkProcessHandler(DeviceListener.Device d, string info, bool completed, int param);
        string DeviceID = null;
        DeviceListener deviceListener = null;

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

        private void PlayFaultNotify()
        {

        }

        public Form1()
        {
            InitializeComponent();

            string path = "../";
            var binfiles = Directory.GetFiles(path, "*.bin");
            if (binfiles.Length > 0) checkBox1.Text = binfiles[0];

            var pakfiles = Directory.GetFiles(path, "*.pak");
            if (pakfiles.Length > 0) checkBox2.Text = pakfiles[0];
        }

        private void EventLogin(DeviceListener.Device d)
        {
            if ((DeviceID != null)&&(DeviceID.Equals(d.id)==false)) return;

            DeviceItem item = new DeviceItem();
            item.id = d.id;
            item.port = d.peerport;
            item.ip = d.peerip;
            item.version = d.version;
            item.devicetype = d.devicetype;

            DeviceID = d.id;
            label4.Text = item.ToString();
            label10.Text = item.devicetype;
            label11.Text = item.version;
            //label11.te

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
            int okpack = 0;
            int faultpack = 0;
            UdpClient uc = new UdpClient();
            byte[] cmd = new byte[] {0xff,0xcc,0xcc,0xcc,0xcc,0xcc,
            0xcc,0xcc,0x01,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,
            0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,
            0xcc,0xcc,0x18,0x15};
            //IPAddress ipaddr = IPAddress.Parse(d.peerip);
            IPEndPoint ep = new IPEndPoint(IPAddress.Parse(d.peerip), 65276);
            IPEndPoint rep = new IPEndPoint(IPAddress.Any, 0);

            uc.Client.ReceiveTimeout = 1000;
            try
            {
                for (int i = 0; i < 1000; i++)
                {
                    uc.Send(cmd, cmd.Length, ep);
                    byte[] rb = uc.Receive(ref rep);
                    if (rb != null)
                    {
                        if (rep.Address.ToString().Equals(d.peerip))
                        {
                            okpack++;
                        }
                        else
                        {
                            faultpack++;
                        }
                    }
                    else
                    {
                        faultpack++;
                    }

                    label5.Text = string.Format("PING SEND={0:D}, RECV={1:D}, DISCARD:{2:D}", i + 1, okpack, faultpack);
                    if (faultpack > 4) break;
                    Application.DoEvents();
                }
            }
            catch (Exception ex)
            { 
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
            if (DeviceID == null) return;
            DeviceListener.Device d = deviceListener.Find(DeviceID);

            panel3.BackColor = SystemColors.Control;

            ThreadParam ap = new ThreadParam();
            ap.filename = "product_test.mp3";
            ap.id = DeviceID;
            ap.volume = 85;

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
            //IPAddress localaddr = IPAddress.Parse("0.0.0.0");
            //TcpListener tcpserver = new TcpListener(localaddr, 0);
            //tcpserver.Start();
            //IPEndPoint localep = (IPEndPoint)tcpserver.LocalEndpoint;

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
            DeviceID = null;
            deviceListener.RemoveAll();
            label4.Text = "";
            label5.Text = "";
            label6.Text = "";
            label7.Text = "";
            label10.Text = "";
            label11.Text = "";
            panel1.BackColor = SystemColors.Control;
            panel2.BackColor = SystemColors.Control;
            panel3.BackColor = SystemColors.Control;
            panel4.BackColor = SystemColors.Control;
            panel5.BackColor = SystemColors.Control;
            panel6.BackColor = SystemColors.Control;
        }

        private void button10_Click(object sender, EventArgs e)
        {
            if (DeviceID == null) return;
            DeviceListener.Device d = deviceListener.Find(DeviceID);

            panel3.BackColor = SystemColors.Control;

            ThreadParam ap = new ThreadParam();
            ap.filename = "product_test2.mp3";
            //ap.filename = "product_test2.mp3";
            ap.id = DeviceID;
            ap.volume = 85;

            label6.Text = ap.filename;

            dataThread = new Thread(SDFilePlayThread);
            dataThread.IsBackground = true;
            dataThread.Start(ap);
        }

        private void button12_Click(object sender, EventArgs e)
        {
            if (DeviceID == null) return;
            DeviceListener.Device d = deviceListener.Find(DeviceID);
            panel5.BackColor = SystemColors.Control;

            ThreadParam ap = new ThreadParam();
            ap.id = DeviceID;
            ap.filename = "../test1.mp3";
            ap.streamtype = "mp3";
            ap.volume = 85;

            dataThread = new Thread(FilePlayThread);
            dataThread.IsBackground = true;
            dataThread.Start(ap);
        }
        private void button11_Click(object sender, EventArgs e)
        {
            if (DeviceID == null) return;
            DeviceListener.Device d = deviceListener.Find(DeviceID);
            panel5.BackColor = SystemColors.Control;

            ThreadParam ap = new ThreadParam();
            ap.id = DeviceID;
            ap.filename = "../test2.mp3";
            ap.streamtype = "mp3";
            ap.volume = 85;

            dataThread = new Thread(FilePlayThread);
            dataThread.IsBackground = true;
            dataThread.Start(ap);
        }
        private void FilePlayProcess(DeviceListener.Device d, string info, bool completed, int param)
        {
            progressBar2.Value = param;
            if (completed)
            {
                progressBar2.Value = 0;
                panel5.BackColor = Color.LimeGreen;
                PlaySuccessNotify();
            }
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
            int ret = deviceListener.FilePlayStart(d, "0.0.0.0", localep.Port, ap.streamtype, ap.volume, null);
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

                Thread.Sleep(3000);
                deviceListener.FilePlayStop(d);
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

        private void button15_Click(object sender, EventArgs e)
        {
            label8.Text = "---";
            label9.Text = "---";
            if (DeviceID == null)
            {
                MessageBox.Show("device is not login.");
                return;
            }
            DeviceListener.Device d = deviceListener.Find(DeviceID);

            string pakfilename = checkBox2.Text;
            d = deviceListener.Find(DeviceID);
            if ((checkBox2.Checked) && (File.Exists(pakfilename)))
            {
                IPAddress localaddr = IPAddress.Parse("0.0.0.0");
                TcpListener tcpserver = new TcpListener(localaddr, 0);
                tcpserver.Start();
                IPEndPoint localep = (IPEndPoint)tcpserver.LocalEndpoint;

                TcpClient tc = null;
                int starttime = System.Environment.TickCount;
                try
                {
                    int ret = deviceListener.FirmwareUpdate(d, "0.0.0.0", localep.Port, pakfilename, "data");
                    if (ret == 0)
                    {
                        while ((System.Environment.TickCount - starttime) < 3000 * 10000)//3000ms
                        {
                            if (tcpserver.Pending())
                            {
                                tc = tcpserver.AcceptTcpClient();
                                break;
                            }
                            Application.DoEvents();
                        }

                        if (tc != null)
                        {
                            NetworkStream tcs = tc.GetStream();
                            FileStream fs = File.OpenRead(pakfilename);
                            if (fs != null)
                            {
                                bool eof = false;
                                long writebytes = 0;
                                byte[] fbs = new byte[1024];
                                
                                while (!eof)
                                {
                                    if (tcs.CanWrite)
                                    {
                                        int readbytes = fs.Read(fbs, 0, fbs.Length);
                                        tcs.Write(fbs, 0, readbytes);
                                        writebytes += readbytes;
                                        if (writebytes >= fs.Length) eof = true;
                                        int process = (int)(100 * writebytes / fs.Length);

                                        label9.Text = process.ToString() + "%";

                                        Application.DoEvents();
                                    }
                                }
                            }
                            int timer = System.Environment.TickCount;
                            tc.Client.Shutdown(System.Net.Sockets.SocketShutdown.Send);

                            byte[] rb = new byte[256];
                            int rlen = tcs.Read(rb, 0, rb.Length);
                            while (rlen > 0)
                            {
                                rlen = tcs.Read(rb, 0, rb.Length);
                            }
                            tc.Close();
                            int t = System.Environment.TickCount - timer;
                        }

                    }
                }
                finally
                {
                    tcpserver.Stop();
                    tcpserver = null;
                }
            }

            Thread.Sleep(1000);
            string binfilename = checkBox1.Text;
            
            if ((checkBox1.Checked) && (File.Exists(binfilename)))
            {
                IPAddress localaddr = IPAddress.Parse("0.0.0.0");
                TcpListener tcpserver = new TcpListener(localaddr, 0);
                tcpserver.Start();
                IPEndPoint localep = (IPEndPoint)tcpserver.LocalEndpoint;

                TcpClient tc = null;
                int starttime = System.Environment.TickCount;
                try
                {
                    int ret = deviceListener.FirmwareUpdate(d, "0.0.0.0", localep.Port, binfilename, "bin");
                    if (ret == 0)
                    { 
                        while ((System.Environment.TickCount - starttime) < 3000 * 10000)//3000ms
                        {
                            if (tcpserver.Pending())
                            {
                                tc = tcpserver.AcceptTcpClient();
                                break;
                            }
                            Application.DoEvents();
                        }

                        if (tc != null)
                        {
                            NetworkStream tcs = tc.GetStream();
                            FileStream fs = File.OpenRead(binfilename);
                            if (fs != null)
                            {
                                bool eof = false;
                                long writebytes = 0;
                                byte[] fbs = new byte[1024];
                                
                                while (!eof)
                                {
                                    if (tcs.CanWrite)
                                    {
                                        int readbytes = fs.Read(fbs, 0, fbs.Length);
                                        tcs.Write(fbs, 0, readbytes);
                                        writebytes += readbytes;
                                        if (writebytes >= fs.Length) eof = true;
                                        int process = (int)(100 * writebytes / fs.Length);

                                        label8.Text = process.ToString() + "%";

                                        Application.DoEvents();
                                    }
                                    Thread.Sleep(10);
                                }
                            }

                            int timer = System.Environment.TickCount;
                            tc.Client.Shutdown(System.Net.Sockets.SocketShutdown.Send);

                            byte[] rb = new byte[256];
                            int rlen = tcs.Read(rb, 0, rb.Length);
                            while (rlen > 0)
                            {
                                rlen = tcs.Read(rb, 0, rb.Length);
                            }
                            tc.Close();
                            int t = System.Environment.TickCount - timer;
                            //label8.Text = t.ToString();
                        }                    
                    }
                }
                finally
                {
                    tcpserver.Stop();
                    tcpserver = null;
                }
            }

            d = deviceListener.Find(DeviceID);
            deviceListener.Apply(d);
            PlaySuccessNotify();

            button2_Click_1(null, null);
        }
    }
}
