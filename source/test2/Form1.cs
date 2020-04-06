using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Media;
using System.Threading;
using System.Net.Sockets;
using System.Net;
using Sinrey.Device;
using Sinrey.SoundCard;
using G711;
using Gimela.Net.Rtp;

namespace test2
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
        private delegate void AudioPowerUpdate(int mic, int speaker);
        private delegate void DelegateShowWave(short[] pcm);
        string DeviceID = null;
        DeviceListener deviceListener = null;
        System.Windows.Forms.Timer timer;
        int test_step;
        Thread SDPlayThreadHandle;
        Thread IntercomThreadHandle;
        Thread MicInputThreadHandle;
        double dbBase;

        public Form1()
        {
            InitializeComponent();

            chart1.ChartAreas[0].AxisX.Minimum = 0;
            chart1.ChartAreas[0].AxisX.Maximum = 480;
            chart1.ChartAreas[0].AxisY.Maximum = 32768;
            chart1.ChartAreas[0].AxisY.Minimum = -32768;

            double p = 0;
            double a = 0;
            for (int i = 0; i < 1024; i++)
            {
                a = 32768 * Math.Sin(i * 2 * 3.14159 / 1024);
                p += a * a;
            }

            dbBase = p / 1024;

            timer = new System.Windows.Forms.Timer();//System.Windows.Forms
            timer.Interval = 10;
            timer.Tick += new EventHandler(TimeAction);
            timer.Start();
        }
        private void PlaySuccessNotify()
        {
            string exename = System.AppDomain.CurrentDomain.SetupInformation.ApplicationBase;
            string path = System.IO.Path.GetFullPath(exename);
            string wavfile = path + "Notify.wav";

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
            string exename = System.AppDomain.CurrentDomain.SetupInformation.ApplicationBase;
            string path = System.IO.Path.GetFullPath(exename);
            string wavfile = path + "Warning.wav";

            if (System.IO.File.Exists(wavfile))
            {
                SoundPlayer player = new SoundPlayer();
                player.SoundLocation = wavfile;
                player.Load();
                player.Play();
            }
        }
        private void EventLogin(DeviceListener.Device d)
        {
            if ((DeviceID != null) && (DeviceID.Equals(d.id) == false)) return;

            DeviceItem item = new DeviceItem();
            item.id = d.id;
            item.port = d.peerport;
            item.ip = d.peerip;
            item.version = d.version;
            item.devicetype = d.devicetype;

            DeviceID = d.id;
            label5.Text = item.ToString();
            label7.Text = item.version;

            label13.Text = item.ToString();
            label11.Text = item.version;
            //
            //label10.Text = item.devicetype;
            //label11.Text = item.version;
            //label11.te

            PlaySuccessNotify();

            panel1.BackColor = Color.LimeGreen;
            panel4.BackColor = Color.LimeGreen;
            //panel2.BackColor = SystemColors.Control;

            button2.Enabled = false;
            button4.Enabled = false;
            test_step = 1;
        }

        private void button5_Click(object sender, EventArgs e)
        {
            deviceListener.RemoveAll();

            DeviceID = null;

            label5.Text = "";
            label7.Text = "";

            label13.Text = "";
            label11.Text = "";

            panel1.BackColor = SystemColors.Control;
            panel4.BackColor = SystemColors.Control;

            panel2.BackColor = SystemColors.Control;
            panel3.BackColor = SystemColors.Control;
        }

        private void EventLogout(DeviceListener.Device d)
        {

        }
        void TimeAction(object sender, EventArgs e)
        {
            switch (test_step)
            {
                case 0:
                    //do nothing
                    break;
                case 1://start sd test
                    Thread.Sleep(500);
                    startSDPlay();
                    test_step = 2;
                    break;
                case 2:
                    //if (SDPlayThreadHandle == null) test_step = 3;
                    if (SDPlayThreadHandle.IsAlive == false) 
                        test_step = 3;
                    break;
                case 3:
                    button2.Enabled = true;
                    button4.Enabled = true;
                    test_step = 4;
                    break;
                default:
                    //do nothing
                    break;
            }
        }
        private void startSDPlay()
        {
            if (DeviceID == null) return;
            DeviceListener.Device d = deviceListener.Find(DeviceID);

            panel2.BackColor = SystemColors.Control;
            panel3.BackColor = SystemColors.Control;

            ThreadParam ap = new ThreadParam();
            ap.filename = "product_test.mp3";
            ap.id = DeviceID;
            ap.volume = 85;

            label14.Text = ap.filename;
            label9.Text = ap.filename;

            SDPlayThreadHandle = new Thread(SDFilePlayThread);
            SDPlayThreadHandle.IsBackground = true;
            SDPlayThreadHandle.Start(ap);
        }
        private void SDPlayFileProcess(DeviceListener.Device d, string info, bool completed, int process)
        {
            //progressBar1.Value = process;
            if (completed)
            {
                if (process >= 0)
                {
                    panel2.BackColor = Color.LimeGreen;
                    panel3.BackColor = Color.LimeGreen;
                    PlaySuccessNotify();
                }
                else
                {
                    panel2.BackColor = Color.Red;
                    panel3.BackColor = Color.Red;
                    PlayFaultNotify();
                }
                progressBar1.Value = 0;// process;
                progressBar2.Value = 0;// process;

            }
            else 
            {
                if (process >= 0)
                {
                    progressBar1.Value = process;
                    progressBar2.Value = process;
                }
            }
        }

        private void SDFilePlayThread(object obj)
        {
            WorkProcessHandler h = new WorkProcessHandler(SDPlayFileProcess);

            ThreadParam ap = (ThreadParam)obj;
            DeviceListener.Device d = deviceListener.Find(ap.id);
            if (d == null)
            {
                this.BeginInvoke(h, d, null, true, -1);
                return;
            }
            string fname = System.IO.Path.GetFileName(ap.filename);
            int ret = deviceListener.SDCardPlayFile(d, fname, ap.volume);
            if (ret != 0)
            {
                this.BeginInvoke(h, d, null, true, -2);
                return;
            }
            //TcpClient tc = null;
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
            }
            catch (ThreadAbortException abortException)
            {
                
            }
            finally
            {
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            deviceListener = new DeviceListener(this, int.Parse(textBox1.Text), textBox2.Text, textBox3.Text);

            deviceListener.EventLogin += new DeviceListener.LoginHandler(EventLogin);
            deviceListener.EventLogout += new DeviceListener.LoginHandler(EventLogout);

            button1.Enabled = false;
        }
        private void AudioPowerUpdateHandler(int mic, int speaker)
        {
            if (speaker >= 0) progressBar3.Value = speaker;
            if (mic >= 0) progressBar4.Value = mic;
        }
        public void TalkRTPServerThread(object obj)
        {
            int sequenceNumber = 0;
            long timestamp = 0;
            short[] inputbuffer = new short[2048];

            AudioPowerUpdate hander = new AudioPowerUpdate(AudioPowerUpdateHandler);
            ThreadParam ap = (ThreadParam)obj;
            DeviceListener.Device d = deviceListener.Find(ap.id);
            if (d == null) return;

            UdpClient udpserver = new UdpClient(9999);
            int port = 9999;
            bool normal_mode = ap.mode.Equals("normal");

            int ret;
            //if (normal_mode)
            ret = deviceListener.IntercomStart(d, "0.0.0.0", port, "g711-u", "rtp", ap.volume, ap.gain, ap.inputsource, ap.aec);
            //else
            //    ret = deviceListener.IntercomEmergencyStart(d, "0.0.0.0", port, "g711-u", "rtp", ap.volume, ap.gain, ap.inputsource, ap.aec);

            if (ret != 0) return;

            IPEndPoint ipep = new IPEndPoint(IPAddress.Any, 0);

            udpserver.Client.ReceiveTimeout = 3000;
            byte[] bs = udpserver.Receive(ref ipep);

            if (bs == null) return;

            string dev_ip = null;
            int dev_port = 0;
            string str = System.Text.Encoding.Default.GetString(bs);

            if (str.Contains(ap.id.ToString()) == false) return;

            dev_ip = ipep.Address.ToString();
            dev_port = ipep.Port;
            //GroovyCodecs.G711.uLaw.ULawEncoder g711encode = new GroovyCodecs.G711.uLaw.ULawEncoder();
            //GroovyCodecs.G711.uLaw.ULawDecoder g711decode = new GroovyCodecs.G711.uLaw.ULawDecoder();

            SoundCardDLL.SoundCardInit(8000);
            try
            {
                short[] micpack = new short[2048];
                int micpack_length = 0;

                short[] speakerpack = new short[2048];
                int speakerpack_length = 0;

                while (true)
                {
                    micpack_length = 0;
                    speakerpack_length = 0;

                    if (SoundCardDLL.SoundCardWaitForInputData())
                    {

                        while (true)
                        {
                            short[] pcmbuf = SoundCardDLL.SoundCardReadFrom(160);
                            if (pcmbuf == null) break;

                            if ((micpack_length + pcmbuf.Length) < 2048)
                            {
                                Array.Copy(pcmbuf, 0, micpack, micpack_length, pcmbuf.Length);
                                micpack_length += pcmbuf.Length;
                            }

                            //if (checkBox1.Checked)
                            //{
                            //    Array.Clear(pcmbuf, 0, pcmbuf.Length);
                            //}

                            //byte[] ba = g711encode.Process(pcmbuf);
                            byte[] ba = g711.g711Encode_ulwa(pcmbuf);
                            RtpPacket packet = new RtpPacket(RtpPayloadType.G711_uLaw, sequenceNumber, timestamp, ba, ba.Length);
                            sequenceNumber++;
                            timestamp += ba.Length;

                            udpserver.Send(packet.ToArray(), packet.Length, dev_ip, dev_port);
                        }
                    }

                    while (udpserver.Available > 0)
                    {
                        bs = udpserver.Receive(ref ipep);

                        if (dev_ip.Equals(ipep.Address.ToString()))
                        {
                            RtpPacket packet = new RtpPacket(bs, bs.Length);
                            if (packet.PayloadType == RtpPayloadType.G711_uLaw)
                            {
                                //short[] pcmbuf1 = g711decode.Process(packet.Payload);
                                short[] pcmbuf1 = g711.g711Decode_ulaw(packet.Payload);
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

                        this.Invoke(hander, Convert.ToInt32(mic_db), -1);
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

                        this.BeginInvoke(hander, -1, Convert.ToInt32(speaker_db));
                    }

                }
            }
            catch (ThreadAbortException abortException)
            {
                //if (normal_mode)
                deviceListener.IntercomStop(d);
                //else
                //    deviceListener.IntercomEmergencyStop(d);

                //udpserver.Close();
                //SoundCard.SoundCardClose();
            }
            finally
            {
                udpserver.Close();
                SoundCardDLL.SoundCardClose();
            }
        }
        private void button2_Click(object sender, EventArgs e)
        {
            //DeviceItem item = (DeviceItem)comboBox1.SelectedItem;
            ThreadParam ap = new ThreadParam();

            ap.id = DeviceID;
            ap.mode = "normal";
            ap.volume = 90;
            ap.aec = "disable";
            ap.inputsource = "mic";
            ap.gain = 20;// int.Parse(textBox4.Text);

            IntercomThreadHandle = new Thread(TalkRTPServerThread);
            IntercomThreadHandle.IsBackground = true;
            IntercomThreadHandle.Start(ap);

            button2.Enabled = false;
            button3.Enabled = true;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            IntercomThreadHandle.Abort();
            IntercomThreadHandle.Join();
            IntercomThreadHandle = null;
            progressBar3.Value = 0;
            progressBar4.Value = 0;
            button2.Enabled = true;
            button3.Enabled = false;
        }

        private int xl = 0;
        short[] sp_pcm = new short[480];
        void ShowWave(short[] pcm)
        {
            if ((xl + pcm.Length) > sp_pcm.Length)
            {
                //chart1.Series[0].Points.Clear();
                xl = 0;
            }

            if ((xl + pcm.Length) <= sp_pcm.Length)
            {
                Array.Copy(pcm, 0, sp_pcm, xl, pcm.Length);
                xl += pcm.Length;
            }

            if (xl >= sp_pcm.Length)
            {
                chart1.Series[0].Points.Clear();
                for (int i = 0; i < sp_pcm.Length; i++)
                {
                    chart1.Series[0].Points.AddXY(i, sp_pcm[i]);
                }
            }
        }

        private void MicInputThreadCallBack(DeviceListener.Device d, string info, bool completed, int param)
        {
            if (completed)
            {
                button4.Text = "开始";
                button4.BackColor = SystemColors.Control;
                if (param < 0)
                {
                    MessageBox.Show(info);
                }
            }
        }
        private void MicInputThread(object obj)
        {
            //int sequenceNumber = 0;
            //long timestamp = 0;
            short[] inputbuffer = new short[2048];
            WorkProcessHandler cb = new WorkProcessHandler(MicInputThreadCallBack);

            DeviceListener.Device d = deviceListener.Find(DeviceID);

            UdpClient udpserver;
            try
            {
                udpserver = new UdpClient(9999);
            }
            catch (Exception ex)
            {
                //MessageBox.Show("port is in use");
                this.BeginInvoke(cb, d, "端口已经被占用",true,-1);
                return;
            }

            int port = 9999;
            DelegateShowWave h = new DelegateShowWave(ShowWave);

            int ret;
            ret = deviceListener.IntercomStart(d, "0.0.0.0", port, "g711-u", "rtp", 100, 20, "mic", "disable");
            if (ret != 0)
            {
                this.BeginInvoke(cb, d, "命令没有回应", true, -2);
                udpserver.Close();
                return;
            }

            IPEndPoint ipep = new IPEndPoint(IPAddress.Any, 0);

            udpserver.Client.ReceiveTimeout = 3000;
            byte[] bs = udpserver.Receive(ref ipep);

            if (bs == null)
            {
                this.BeginInvoke(cb, d, "终端没有连接", true, -3);
                deviceListener.IntercomStop(d);
                udpserver.Close();
                return;
            }

            string dev_ip = null;
            int dev_port = 0;
            string str = System.Text.Encoding.Default.GetString(bs);

            if (str.Contains(DeviceID.ToString()) == false)
            {
                this.BeginInvoke(cb, d, "错误的设备ID", true, -4);
                deviceListener.IntercomStop(d);
                udpserver.Close();
                return;
            }

            dev_ip = ipep.Address.ToString();
            dev_port = ipep.Port;

            try
            {
                short[] micpack = new short[2048];
                //int micpack_length = 0;

                short[] speakerpack = new short[2048];
                //int speakerpack_length = 0;

                while (true)
                {
                    //micpack_length = 0;
                    //speakerpack_length = 0;

                    while (udpserver.Available > 0)
                    {
                        bs = udpserver.Receive(ref ipep);

                        if (dev_ip.Equals(ipep.Address.ToString()))
                        {
                            RtpPacket packet = new RtpPacket(bs, bs.Length);
                            if (packet.PayloadType == RtpPayloadType.G711_uLaw)
                            {
                                short[] pcmbuf1 = g711.g711Decode_ulaw(packet.Payload);
                                this.BeginInvoke(h, pcmbuf1);
                            }
                        }
                    }
                }
            }
            //catch (ThreadAbortException abortException)
            //{
            //    deviceListener.IntercomStop(d);
            //    //udpserver.Close();
            //}
            finally
            {
                deviceListener.IntercomStop(d);
                udpserver.Close();
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            
            if ((MicInputThreadHandle == null)||(MicInputThreadHandle.IsAlive == false))
            {
                MicInputThreadHandle = new Thread(MicInputThread);
                MicInputThreadHandle.IsBackground = true;
                MicInputThreadHandle.Start();
                button4.Text = "结束";
                button4.BackColor = Color.Red;
            }
            else 
            {
                MicInputThreadHandle.Abort();
                MicInputThreadHandle.Join();
                MicInputThreadHandle = null;
                button4.Text = "开始";
                button4.BackColor = SystemColors.Control;
            }
           
        }

    }
}
