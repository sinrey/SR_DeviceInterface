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
using System.Threading;
using System.Windows.Forms;
using Gimela.Net.Rtp;
using Sinrey.Device;
using Sinrey.SoundCard;
using G711;

namespace AudioDemo
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
        }

        double dbBase;
        Thread dataThread;
        DeviceListener deviceListener = null;

        public Form1()
        {
            InitializeComponent();

            double p = 0;
            double a = 0;
            for (int i = 0; i < 1024; i++)
            {
                a = 32768 * Math.Sin(i * 2 * 3.14159 / 1024);
                p += a * a;
            }

            dbBase = p / 1024;

            listBox1.Items.Clear();
        }

        private delegate void LoginHandler();
        private delegate void AudioPowerUpdate(int mic, int speaker);


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
            deviceListener = new DeviceListener(this, int.Parse(textBox1.Text),textBox2.Text,textBox3.Text);

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

        private void AudioPowerUpdateHandler(int mic, int speaker)
        {
            if(speaker >= 0)progressBar1.Value = speaker;
            if(mic >= 0)progressBar2.Value = mic;
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
            if(normal_mode)
                ret = deviceListener.IntercomStart(d, "0.0.0.0", port, "g711-u", "rtp", ap.volume, ap.gain, ap.inputsource, ap.aec);
            else
                ret = deviceListener.IntercomEmergencyStart(d, "0.0.0.0", port, "g711-u", "rtp", ap.volume, ap.gain, ap.inputsource, ap.aec);

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

                            if (checkBox1.Checked)
                            {
                                Array.Clear(pcmbuf, 0, pcmbuf.Length);
                            }

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

                        this.Invoke(hander, -1, Convert.ToInt32(speaker_db));
                    }

                }
            }
            catch (ThreadAbortException abortException)
            {
                if (normal_mode)
                    deviceListener.IntercomStop(d);
                else
                    deviceListener.IntercomEmergencyStop(d);

                //udpserver.Close();
                //SoundCard.SoundCardClose();
            }
            finally
            {
                udpserver.Close();
                SoundCardDLL.SoundCardClose();
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            if (comboBox1.SelectedItem != null)
            {
                DeviceItem item = (DeviceItem)comboBox1.SelectedItem;
                ThreadParam ap = new ThreadParam();

                ap.id = item.id;
                ap.mode = comboBox2.Text;
                ap.volume = (int)numericUpDown1.Value;
                ap.aec = comboBox4.Text;
                ap.inputsource = comboBox3.Text;
                ap.gain = int.Parse(textBox4.Text);

                dataThread = new Thread(TalkRTPServerThread);
                dataThread.IsBackground = true;
                dataThread.Start(ap);

                button3.Enabled = false;
                button4.Enabled = true;
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            dataThread.Abort();
            button3.Enabled = true;
            button4.Enabled = false;
        }

    }
}
