using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Threading;
using Sinrey.DeviceInterface;

namespace AudioDemo_Update_DLL
{
    public partial class Form2 : Form
    {
        //public delegate void DelegateCloseHandler();
        //public DelegateCloseHandler CloseHandler;
        private delegate void DelegateShowProcess(string n, int process);
        private delegate void DelageteCloseForm();

        public UInt32 id;
        public string bin_filename;
        public string pak_filename;
        public Form2()
        {
            InitializeComponent();
        }
        
        public void SetProgress(int percent)
        {
            progressBar1.Value = percent;
        }

        public void SetLabel(string text)
        {
            label1.Text = text;
        }

        private void CloseForm()
        {
            UpdateThreadHandle.Abort();
            UpdateThreadHandle.Join();
            this.Close();
        }
        private void ShowProcess(string n, int process)
        {
            if (process > 100)
            {
                Delegate hc = new DelageteCloseForm(CloseForm);
                this.BeginInvoke(hc);
            }
            else
            {
                //if (process == 100)
                //{
                //    Thread.Sleep(0);
                //}
                label1.Text = n;
                //label1.Text = process.ToString();
                progressBar1.Value = process;
                //Application.DoEvents();
            }
        }
        Thread UpdateThreadHandle;
        private void UpdateThread(object obj)
        {
            //ThreadParam ap = (ThreadParam)obj;
            Delegate h = new DelegateShowProcess(ShowProcess);

            try
            {
                uint ret;
                if ((bin_filename != null) && (File.Exists(bin_filename)))
                {
                    ret = DeviceInterfaceDll.SR_Update(out uint bin_handle, id, 0, bin_filename);
                    if (ret == DeviceInterfaceDll.RC_OK)
                    {
                        try
                        {
                            int count = 0;
                            byte[] bdata = new byte[1024];
                            FileStream fs = File.OpenRead(bin_filename);
                            if (fs != null)
                            {
                                int rlen;
                                do
                                {
                                    rlen = fs.Read(bdata, 0, bdata.Length);
                                    if (rlen > 0)
                                    {
                                        count += rlen;
                                        DeviceInterfaceDll.SR_UpdateData(bin_handle, bdata, rlen);
                                        this.Invoke(h, bin_filename, (int)(100 * count / fs.Length));
                                    }
                                    //System.Windows.Forms.Application.DoEvents();
                                }
                                while (rlen >= bdata.Length);
                            }
                            fs.Close();
                        }
                        finally
                        {
                            DeviceInterfaceDll.SR_UpdateClose(bin_handle);
                        }
                    }
                }

                Thread.Sleep(500);

                if ((pak_filename != null) && (File.Exists(pak_filename)))
                {
                    ret = DeviceInterfaceDll.SR_Update(out uint pak_handle, id, 1, pak_filename);
                    if (ret == DeviceInterfaceDll.RC_OK)
                    {
                        try
                        {
                            int count = 0;
                            byte[] bdata = new byte[1024];
                            FileStream fs = File.OpenRead(pak_filename);
                            if (fs != null)
                            {
                                int rlen;
                                do
                                {
                                    rlen = fs.Read(bdata, 0, bdata.Length);
                                    if (rlen > 0)
                                    {
                                        count += rlen;
                                        DeviceInterfaceDll.SR_UpdateData(pak_handle, bdata, rlen);
                                        this.Invoke(h, pak_filename, (int)(100 * count / fs.Length));
                                    }
                                }
                                while (rlen >= bdata.Length);

                                //wait update 200
                            }
                            //else
                            //{
                            //    Thread.Sleep(0);
                            //}
                            fs.Close();
                        }
                        finally
                        {
                            DeviceInterfaceDll.SR_UpdateClose(pak_handle);
                        }
                    }
                }

                DeviceInterfaceDll.SR_Apply(id);
            }
            finally
            {
                this.Invoke(h, "", 101);
            }
        }

        private void Form2_Shown(object sender, EventArgs e)
        {
            UpdateThreadHandle = new Thread(UpdateThread);
            UpdateThreadHandle.IsBackground = true;
            UpdateThreadHandle.Start();
        }
    }
}
