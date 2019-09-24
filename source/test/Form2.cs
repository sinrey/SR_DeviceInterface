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
//using GroovyCodecs.G711;
using Gimela.Net.Rtp;
using G711;
using Sinrey.Device;

namespace test
{
    public partial class Form2 : Form
    {
        private delegate void DelegateShowWave(short[] pcm);

        public DeviceListener deviceListener;
        public UInt32 DeviceID;


        Thread dataThread = null;
        public Form2()
        {
            InitializeComponent();

            chart1.ChartAreas[0].AxisX.Minimum = 0;
            chart1.ChartAreas[0].AxisX.Maximum = 480;
            chart1.ChartAreas[0].AxisY.Maximum = 32768;
            chart1.ChartAreas[0].AxisY.Minimum = -32768;

            dataThread = new Thread(IntercomThread);
            dataThread.IsBackground = true;
            dataThread.Start();
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

        private void button1_Click(object sender, EventArgs e)
        {
            dataThread.Abort();
            dataThread.Join();
            Application.DoEvents();
            this.Close();
        }

        private void IntercomThread(object obj)
        {
            int sequenceNumber = 0;
            long timestamp = 0;
            short[] inputbuffer = new short[2048];

            //AudioPowerUpdate hander = new AudioPowerUpdate(AudioPowerUpdateHandler);
            //ThreadParam ap = (ThreadParam)obj;
            //DeviceListener.Device d = deviceListener.Find(ap.id);
            //if (_Device == null) return;
            DeviceListener.Device d = deviceListener.Find(DeviceID);

            UdpClient udpserver = new UdpClient(9999);
            int port = 9999;
            //bool normal_mode = ap.mode.Equals("normal");
            DelegateShowWave h = new DelegateShowWave(ShowWave);

            int ret;
            ret = deviceListener.IntercomStart(d, "0.0.0.0", port, "g711-u", "rtp", 100, 20, "mic", "disable");
            if (ret != 0) return;

            IPEndPoint ipep = new IPEndPoint(IPAddress.Any, 0);

            udpserver.Client.ReceiveTimeout = 3000;
            byte[] bs = udpserver.Receive(ref ipep);

            if (bs == null) return;

            string dev_ip = null;
            int dev_port = 0;
            string str = System.Text.Encoding.Default.GetString(bs);

            if (str.Contains(DeviceID.ToString()) == false) return;

            dev_ip = ipep.Address.ToString();
            dev_port = ipep.Port;
            //GroovyCodecs.G711.uLaw.ULawEncoder g711encode = new GroovyCodecs.G711.uLaw.ULawEncoder();
            //GroovyCodecs.G711.uLaw.ULawDecoder g711decode = new GroovyCodecs.G711.uLaw.ULawDecoder();

            //SoundCard.SoundCardInit(8000);
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
                    /*
                    if (SoundCard.SoundCardWaitForInputData())
                    {

                        while (true)
                        {
                            short[] pcmbuf = SoundCard.SoundCardReadFrom(160);
                            if (pcmbuf == null) break;

                            if ((micpack_length + pcmbuf.Length) < 2048)
                            {
                                Array.Copy(pcmbuf, 0, micpack, micpack_length, pcmbuf.Length);
                                micpack_length += pcmbuf.Length;
                            }


                            byte[] ba = g711encode.Process(pcmbuf);
                            RtpPacket packet = new RtpPacket(RtpPayloadType.G711_uLaw, sequenceNumber, timestamp, ba, ba.Length);
                            sequenceNumber++;
                            timestamp += ba.Length;

                            udpserver.Send(packet.ToArray(), packet.Length, dev_ip, dev_port);
                        }
                    }
                    */

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
                                this.BeginInvoke(h, pcmbuf1);
                                //SoundCard.SoundCardWriteTo(pcmbuf1);

                                //if ((speakerpack_length + pcmbuf1.Length) < 2048)
                                //{
                                //    Array.Copy(pcmbuf1, 0, speakerpack, speakerpack_length, pcmbuf1.Length);
                                //    speakerpack_length += pcmbuf1.Length;
                                //}
                            }
                        }
                    }
                    /*
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

                        this.Invoke(hander, Convert.ToInt32(mic_db), 0);
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

                        this.Invoke(hander, 0, Convert.ToInt32(speaker_db));
                    }
                    */
                }
            }
            catch (ThreadAbortException abortException)
            {
                deviceListener.IntercomStop(d);

                udpserver.Close();
            }
        }
    }
}
