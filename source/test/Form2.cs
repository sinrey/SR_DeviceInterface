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
using G711;
using Sinrey.Device;

namespace test
{
    public partial class Form2 : Form
    {
        private delegate void DelegateShowWave(short[] pcm);

        public DeviceListener deviceListener;
        public string DeviceID;


        Thread dataThread = null;
        public Form2()
        {
            InitializeComponent();

            chart1.ChartAreas[0].AxisX.Minimum = 0;
            chart1.ChartAreas[0].AxisX.Maximum = 480;
            chart1.ChartAreas[0].AxisY.Maximum = 32768;
            chart1.ChartAreas[0].AxisY.Minimum = -32768;
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

            DeviceListener.Device d = deviceListener.Find(DeviceID);

            UdpClient udpserver;
            try
            {
                udpserver = new UdpClient(9999);
            }
            catch (Exception ex)
            {
                MessageBox.Show("port is in use");
                return;
            }
            
            int port = 9999;
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

                    while (udpserver.Available > 0)
                    {
                        bs = udpserver.Receive(ref ipep);

                        if (dev_ip.Equals(ipep.Address.ToString()))
                        {
                            RtpPacket packet = new RtpPacket(bs, bs.Length);
                            if (packet.PayloadType == RtpPayloadType.G711_uLaw)
                            {
                                short[] pcmbuf1 = g711.g711Decode_ulaw(packet.Payload);
                                this.Invoke(h, pcmbuf1);
                            }
                        }
                    }
                }
            }
            catch (ThreadAbortException abortException)
            {
                deviceListener.IntercomStop(d);

                udpserver.Close();
            }
        }

        private void Form2_Shown(object sender, EventArgs e)
        {
            dataThread = new Thread(IntercomThread);
            dataThread.IsBackground = true;
            dataThread.Start();
        }
    }
}
