using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows.Forms;
using Sinrey.DeviceInterface;
using Sinrey.SoundCard;

namespace test_dll_1
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void OnConnected(DeviceInterfaceDll.InterfaceMsg msg)
        {
            string str = string.Format("msgtype={0:D};wparam={1:X};lparam={2:D}\r\n", msg.msg, msg.WParam, msg.LParam);
            textBox1.AppendText(str);
        }

        private void OnLogin(DeviceInterfaceDll.InterfaceMsg msg)
        {
            string str = string.Format("msgtype={0:D};wpamra={1:X};lparam={2:D}\r\n", msg.msg, msg.WParam, msg.LParam);
            textBox1.AppendText(str);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            DeviceInterfaceDll.EventConnect += OnConnected;
            DeviceInterfaceDll.EventLogin += OnLogin;
            DeviceInterfaceDll.SR_Init(this, 8877);

            textBox1.AppendText("add "+textBox2.Text+"\r");
            DeviceInterfaceDll.SR_Login(textBox2.Text, "admin", "1234");
        }

        private void button2_Click(object sender, EventArgs e)
        {
            DeviceInterfaceDll.SDInformation sdinfo;
            //int ret = DeviceInterfaceDll.SR_GetCapacity(1, out sdinfo);
            uint ret = DeviceInterfaceDll.SR_GetCapacity(1, out sdinfo);
            //if(json != null)textBox1.AppendText(json);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            button3.Enabled = false;
            string f = "f:\\tmp\\test.mp3";
            FileStream fs = File.OpenRead(f);
            UInt32 Handle;
            uint ret = DeviceInterfaceDll.SR_UploadFile_V40(out Handle, 1, f, DeviceInterfaceDll.UPLOAD_AUDIO_FILE_STORAGE, true);
            if (ret == 0)
            {
                try
                {
                    byte[] b = new byte[1024];
                    int count = 0;
                    int len = fs.Read(b, 0, b.Length);
                    while (len > 0)
                    {
                        ret = DeviceInterfaceDll.SR_Upload_Process(Handle, b, len);
                        count += b.Length;
                        progressBar1.Value = (int)(100 * count / fs.Length);
                        Application.DoEvents();
                        len = fs.Read(b, 0, b.Length);
                    }
                }
                finally
                {
                    DeviceInterfaceDll.SR_UploadClose(Handle);
                }
                progressBar1.Value = 0;
            }
            button3.Enabled = true;
        }

        private void button4_Click(object sender, EventArgs e)
        {
            button3.Enabled = false;
            string f = "f:\\tmp\\test.mp3";
            FileStream fs = File.OpenRead(f);
            UInt32 Handle;
            uint ret = DeviceInterfaceDll.SR_UploadFile_V40(out Handle, 1, f, DeviceInterfaceDll.UPLOAD_AUDIO_FILE_RELEASE, true);
            if (ret == 0)
            {
                try
                {
                    byte[] b = new byte[1024];
                    int count = 0;
                    int len = fs.Read(b, 0, b.Length);
                    while (len > 0)
                    {
                        ret = DeviceInterfaceDll.SR_Upload_Process(Handle, b, len);
                        count += b.Length;
                        progressBar1.Value = (int)(100 * count / fs.Length);
                        Application.DoEvents();
                        len = fs.Read(b, 0, b.Length);
                    }
                }
                finally
                {
                    DeviceInterfaceDll.SR_UploadClose(Handle);
                }
                progressBar1.Value = 0;
            }
            button3.Enabled = true;
        }

        private void button5_Click(object sender, EventArgs e)
        {
            button5.Enabled = false;
            string f = "test.mp3";
            //FileStream fs = File.OpenRead(f);
            uint ret = DeviceInterfaceDll.SR_PutFile(1, f);
            if (ret == 0)
            {
                try
                {
                    UInt32 process;
                    long timer = DateTime.Now.Ticks;
                    while (true)
                    {
                        if ((DateTime.Now.Ticks - timer) > (1000 * 10000))
                        {
                            timer = DateTime.Now.Ticks;
                            ret = DeviceInterfaceDll.SR_PutFile_Status(1, out process);
                            if (ret != 0) break;

                            progressBar1.Value = (int)(process % 100);
                        }
                        Application.DoEvents();
                    }
                }
                finally
                {
                    DeviceInterfaceDll.SR_PutFile_Stop(1);
                }
                progressBar1.Value = 0;
            }
            button5.Enabled = true;
        }

        private bool IntercomRun;
        private void Button6_Click(object sender, EventArgs e)
        {
            uint ret = DeviceInterfaceDll.SR_StartVoiceCom_V30(out UInt32 handle, 1);
            if (ret == 0)
            {
                IntercomRun = true;
                try
                {
                    SoundCardDLL.SoundCardInit(16000);
                    while (IntercomRun)
                    {
                        if (SoundCardDLL.SoundCardWaitForInputData())
                        {
                            /*
                            short[] pcmbuf;
                            pcmbuf = SoundCard.SoundCardReadFrom(320);
                            while (pcmbuf != null)
                            {
                                short[] recvdata = DeviceInterfaceDll.SR_VoiceCom_Data(handle, pcmbuf);
                                if (recvdata != null)
                                {
                                    SoundCard.SoundCardWriteTo(recvdata);
                                }

                                pcmbuf = SoundCard.SoundCardReadFrom(320);
                            }
                            */
                            short[] pcmbuf = SoundCardDLL.SoundCardReadFrom(320);
                            if(pcmbuf != null)
                            {
                                short[] recvdata = DeviceInterfaceDll.SR_VoiceCom_Data(handle, pcmbuf);
                                if (recvdata != null)
                                {
                                    SoundCardDLL.SoundCardWriteTo(recvdata);
                                }
                            }

                        }

                        Application.DoEvents();
                    }
                }
                finally
                {
                    DeviceInterfaceDll.SR_StopVoiceCom(handle, 1);
                    SoundCardDLL.SoundCardClose();
                }
            }
            else
            {
                textBox1.AppendText(ret.ToString()+"\r\n");
            }
        }

        private void Button7_Click(object sender, EventArgs e)
        {
            IntercomRun = false;
        }
    }
}
