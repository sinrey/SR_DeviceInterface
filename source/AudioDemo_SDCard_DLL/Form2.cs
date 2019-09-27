using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Sinrey.DeviceInterface;

namespace AudioDemo_SDCard_DLL
{
    public partial class Form2 : Form
    {
        /*
        class ThreadParam
        {
            public uint id;
            public string filename;
            public uint volume;
        }
        private delegate void DelegateShowProcess(int process);

        public uint uid;
        public string filename;
        public uint volume;
        Thread dataThread;
        */

        public delegate void DelegateCloseHandler();
        public DelegateCloseHandler CloseHandler;
        public Form2()
        {
            InitializeComponent();
        }

        private void Form2_Load(object sender, EventArgs e)
        {
            /*
            ThreadParam ap = new ThreadParam();

            ap.id = uid;
            ap.filename = filename;

            dataThread = new Thread(SDPlayFileThread);
            dataThread.IsBackground = true;
            dataThread.Start(ap);

            label1.Text = ap.filename;
            */
        }

        public void SetProgress(int percent)
        {
            progressBar1.Value = percent;
        }

        public void SetLabel(string text)
        {
            label1.Text = text;
        }
        /*
        private void ShowProcess(int process)
        {
            if (process <= 100)
                progressBar1.Value = process;
            else
                this.Close();
        }
        */
        /*
        private void SDPlayFileThread(object obj)
        {
            ThreadParam ap = (ThreadParam)obj;

            Delegate h = new DelegateShowProcess(ShowProcess);

            uint ret = DeviceInterfaceDll.SR_PutFile(ap.id, ap.filename, ap.volume);
            if (ret == DeviceInterfaceDll.RC_OK)
            {
                try
                {
                    while (true)
                    {
                        ret = DeviceInterfaceDll.SR_PutFile_Status(ap.id, out uint nProcess);
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
                    //ret = DeviceInterfaceDll.SR_PutFile_Stop
                }
                finally
                {
                    dataThread = null;
                    this.BeginInvoke(h, 101);
                }
            }

        }
        */
        private void Button1_Click(object sender, EventArgs e)
        {
            /*
            uint ret = DeviceInterfaceDll.SR_PutFile_Stop(uid);
            if (ret != DeviceInterfaceDll.RC_OK)
            {
                label1.Text = "error";
            }
            dataThread.Abort();
            //this.Close();
            */
            //this.Close();
            if(CloseHandler != null) CloseHandler();
            //this.Close();
        }
        

    }
}
