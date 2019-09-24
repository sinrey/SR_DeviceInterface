using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Net;
using System.IO;
using System.Net.Sockets;
using System.Threading;
using Newtonsoft.Json;
using System.Security.Cryptography;

namespace Sinrey.Device
{

    public class DeviceListener
    {
        private class Command
        {
            public string command;
        }

        private class CommandAck
        {
            public string command;
            public int result;
        }
        private class CommandRegister
        {
            public string command;
            public uint id;
            public string authentication;
            public string session;
            public string auth_dir;
            public int result;
        }
        private class CommandStatus
        {
            public string command;
            public string mode;
            public uint id;
        }

        private class CommandStart
        {
            public string command;
            public string aecmode;
            public string mode;
            public string streamtype;
            public string dataserver;
            public int dataserverport;
            public int volume;
            public int samplerate;
            public string buffer;
            public string nodelay;
            public string inputsource;
            public int inputgain;
            public int timeout;
            public int result;
            public string protocol;
            public string param;
        }

        private class CommandStartAck
        {
            public string command;
            public int result;
        }

        private class CommandStop
        {
            public string command;
            public int result;
        }

        private class CommandDisk
        {
            public string command;
        }
        private class CommandDiskAck
        {
            public string command;
            public string disk_size;
            public string free_size;
            public int result;
        }
        private class CommandFileAck
        {
            public string command;
            public string filename;
            public int filesize;
            public int result;
        }
        private class CommandDeleteFile
        {
            public string command;
            public string filename;
        }
        private class CommandDeleteFileAck
        {
            public string command;
            public string filename;
            public int result;
        }
        private class CommandWriteFile
        {
            public string command;
            public string dataserver;
            public int dataserverport;
            public string filename;
        }

        private class CommandWriteFileAck
        {
            public string command;
            public int result;
        }
        private class CommandPlayFile
        {
            public string command;
            public string mode;
            public int volume;
            public string filename;
        }
        private class CommandPlayFileAck
        {
            public string command;
            public int result;
        }
        private class CommandPlayFileStatusAck
        {
            public string command;
            public string filename;
            public int runtime;
            public int process;
            public int result;
        }

        private class CommandSetVolume
        {
            public string command;
            public int volume;
        }

        public class Device
        {
            public string username;
            public string password;
            public uint id;
            public int loginState;
            public string peerip;
            public int peerport;
            public TcpClient tcpClient;
            public long systick;
            public byte[] jsonbuf;
            public int jsonbuflen;

            public Device(string username, string password, TcpClient tc,string ip, int port)
            {
                tcpClient = tc;
                peerip = ip;
                peerport = port;
                jsonbuf = new byte[8 * 1204];
                jsonbuflen = 0;
            }

            public void PushBytes(byte[] inbuf, int len)
            {
                if ((inbuf != null)&&(len > 0))
                {
                    if ((jsonbuflen + len) > (8 * 1024)) return;
                    Array.Copy(inbuf, 0, jsonbuf, jsonbuflen, len);
                    jsonbuflen += len;
                }
            }

            public string GetOneJsonText()
            {
                int level = 0;
                bool header = false;

                int startindex = 0;
                int endindex = 0;
                for (int i = 0; i < jsonbuflen; i++)
                {
                    byte b = jsonbuf[i];

                    //if (b.Equals('{'))
                    if(b == '{')
                    {
                        if (header == false) startindex = i;
                        header = true;
                        level++;
                        
                    }

                    //if (b.Equals('}'))
                    if(b == '}')
                    {
                        level--;
                        if ((header) && (level == 0))
                        {
                            endindex = i;
                            break;
                        }
                    }
                }
                if (endindex > startindex)
                {
                    int size = endindex - startindex + 1;
                    byte[] outbuf = new byte[size];
                    Array.Copy(jsonbuf, startindex, outbuf, 0, outbuf.Length);
                    int newstartindex = endindex + 1;

                    for (int i = newstartindex; i < jsonbuflen; i++)
                    {
                        jsonbuf[i - newstartindex] = jsonbuf[i];
                    }
                    String jsontext = System.Text.Encoding.Default.GetString(outbuf);

                    jsonbuflen = jsonbuflen - newstartindex;

                    return jsontext;
                }
                else
                {
                    return null;
                }
            }

            public bool IsLogin()
            {
                return (loginState == 3);
            }
        }

        class ThreadParam
        {
            public int port;
            public string username;
            public string password;
        }
        public delegate void LoginHandler(Device d);
        public event LoginHandler EventLogin;
        public event LoginHandler EventLogout;

        public delegate void WorkProcessHandler(Device d, string info, bool completed, int param);
        public event WorkProcessHandler EventWorkProcess;

        Form parent;
        Thread listenThread;
        private List<Device> DeviceList;

        public DeviceListener(Form f, int port, string username, string password)
        {
            parent = f;
            ThreadParam arg = new ThreadParam();
            arg.port = port;
            arg.username = username;
            arg.password = password;

            DeviceList = new List<Device>();

            listenThread = new Thread(ListenThread);
            listenThread.IsBackground = true;
            listenThread.Start(arg);
        }

        ~DeviceListener()
        {

        }

        private void EventDeviceLogin(Device d)
        {
            //parent.BeginInvoke(EventLogin, d);
            parent.Invoke(EventLogin, d);
        }

        private void EventDeviceLogout(Device d)
        {
            parent.Invoke(EventLogout, d);
        }

        private void EventDeviceUploadFile(Device d, string info, bool completed, int param)
        {
            parent.BeginInvoke(EventWorkProcess, d, info, completed, param);
        }

        private void ListenThread(Object obj)
        {
            byte[] bytes = new byte[1024];

            ThreadParam arg = (ThreadParam)obj;
            TcpListener tcplistener;
            tcplistener = new TcpListener(IPAddress.Parse("0.0.0.0"), arg.port);
            tcplistener.Start();

            while (true)
            {
                if (tcplistener.Pending())
                {
                    string remoteaddr;
                    TcpClient tcpclient = tcplistener.AcceptTcpClient();
                    remoteaddr = ((IPEndPoint)tcpclient.Client.RemoteEndPoint).Address.ToString();

                    Device d = new Device(arg.username, arg.password, tcpclient, remoteaddr, ((IPEndPoint)tcpclient.Client.RemoteEndPoint).Port);
                    d.loginState = 1;
                    DeviceList.Add(d);
                }

                bool modify = false;
                foreach (Device d in DeviceList)
                {
                    int rlen;
                    lock (d)//if (d.tcpClient.Connected) //lock (d)
                    {
                        if (d.tcpClient.Connected)
                        //lock(d)
                        {
                            if (d.tcpClient.Available > 0)
                            {
                                NetworkStream ns = d.tcpClient.GetStream();
                                rlen = ns.Read(bytes, 0, bytes.Length);
                                {
                                    if (rlen > 0)
                                    {
                                        d.PushBytes(bytes, rlen);
                                    }
                                }
                            }
                        }
                        else
                        {
                            EventDeviceLogout(d);
                        }

                        string jsontext = d.GetOneJsonText();
                        if (jsontext != null)
                        {
                            CommandStatus c = JsonConvert.DeserializeObject<CommandStatus>(jsontext);
                            if (c.command.Equals("register"))
                            {
                                CommandRegister cr = JsonConvert.DeserializeObject<CommandRegister>(jsontext);
                                if (d.loginState == 1)
                                {
                                    if ((cr.authentication != null) && (cr.session != null))
                                    {
                                        MD5 md5 = new MD5CryptoServiceProvider();
                                        byte[] auth = System.Text.Encoding.Default.GetBytes(cr.session + "@" + arg.username + "@" + arg.password);
                                        byte[] tmp = md5.ComputeHash(auth);
                                        string md5str = null;

                                        for (int i = 0; i < tmp.Length; i++)md5str += tmp[i].ToString("x2");

                                        if (cr.authentication.Equals(md5str))
                                        {
                                            string session = Guid.NewGuid().ToString("N");
                                            byte[] bs1 = System.Text.Encoding.Default.GetBytes(session + "@" + arg.username + "@" + arg.password);
                                            byte[] tmp1 = md5.ComputeHash(bs1);
                                            string md52 = null;

                                            for (int i = 0; i < tmp.Length; i++)md52 += tmp1[i].ToString("x2");

                                            CommandRegister cr2 = new CommandRegister();
                                            cr2.command = "register";
                                            cr2.result = 200;
                                            cr2.session = session;
                                            cr2.authentication = md52;

                                            string jsontext1 = JsonConvert.SerializeObject(cr2);
                                            NetworkStream ns = d.tcpClient.GetStream();
                                            //if (ns.CanWrite)
                                            {
                                                byte[] bs = System.Text.Encoding.Default.GetBytes(jsontext1);
                                                ns.Write(bs, 0, bs.Length);
                                            }
                                            d.id = cr.id;
                                            d.loginState = 2;
                                        }
                                    }
                                }
                                else if (d.loginState == 2)
                                {
                                    if (cr.result == 200)
                                    {
                                        d.loginState = 3;
                                        EventDeviceLogin(d);

                                        for (int i = DeviceList.Count - 1; i >= 0; i--)
                                        {
                                            
                                            Device d1 = DeviceList[i];
                                            if (d1.id == d.id)
                                            {
                                                if (!d1.Equals(d))
                                                {
                                                    this.Remove(d1);
                                                    modify = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else if (c.command.Equals("status"))
                            {
                                CommandStatus cs = JsonConvert.DeserializeObject<CommandStatus>(jsontext);
                                d.systick = DateTime.Now.Ticks;
                            }
                        }
                    }
                    if (modify) break;
                }
                Thread.Sleep(10);
            }
        }

        public void Remove(Device d)
        {
            d.tcpClient.Close();
            DeviceList.Remove(d);
            EventDeviceLogout(d);
        }

        public void RemoveAll()
        {
            for (int i = DeviceList.Count - 1; i >= 0; i--)
            {
                Device d = DeviceList[i];
                Remove(d);
            }
        }

        public Device Find(uint device_id)
        {
            foreach(Device d in DeviceList)
            {
                if (d.id == device_id) return d;
            }
            return null;
        }

        public string WaitAck(Device d, string command)
        {
            string result = null;
            List<string> jsonack = new List<string>();
            NetworkStream ns = d.tcpClient.GetStream();
            ns.ReadTimeout = 3000;
            byte[] rs = new byte[2048];
            while (true)
            {
                int len = ns.Read(rs, 0, rs.Length);
                if (len > 0)
                {
                    d.PushBytes(rs, len);

                    //检查缓冲内是否有期望的回应
                    while (true)
                    {
                        string jsontext2 = d.GetOneJsonText();
                        if (jsontext2 != null)
                        {
                            CommandAck c = JsonConvert.DeserializeObject<CommandAck>(jsontext2);
                            if (c.command.Equals(command))
                            {
                                result = jsontext2;
                                break;
                            }
                            else
                            {
                                //未能处理的消息,加入到未决回应序列。
                                jsonack.Add(jsontext2);
                            }
                        }
                        else
                        {
                            //没有有效的回应，退出检查，再次读取网络数据
                            break;
                        }
                        
                    }
                    if (result != null) break;
                }
                else
                {
                    //网络无数据了。
                    break;
                }
            }

            //未处理的回应，将其序列化后再次填入接收缓存,由监听线程处理。
            foreach (string s in jsonack)
            {
                byte[] ba = System.Text.Encoding.Default.GetBytes(s);
                d.PushBytes(ba, ba.Length);
            }

            return result;
        }

        private string SendCommand(Device d, string command, string jsontext)
        {
            byte[] bs = System.Text.Encoding.Default.GetBytes(jsontext);
            NetworkStream ns = d.tcpClient.GetStream();
            ns.Write(bs, 0, bs.Length);
            return WaitAck(d, command);
        }

        public int AudioClose(Device d)
        {
            if (d == null) return -1;

            CommandStop cs = new CommandStop();
            cs.command = "stop";
            string jsontext = JsonConvert.SerializeObject(cs);
            string ack;
            lock (d)
            {
                ack = SendCommand(d, "stop", jsontext);
            }
            
            if (ack != null)
            {
                CommandStop cs1 = JsonConvert.DeserializeObject<CommandStop>(ack);
                if ((cs1.result >= 200)&&(cs1.result < 300))
                {
                    return 0;
                }
            }
            return -1;
        }

        public int AudioSetVolume(Device d,int vol)
        {
            CommandSetVolume csv = new CommandSetVolume();
            csv.command = "set";
            csv.volume = vol;
            string jsontext = JsonConvert.SerializeObject(csv);
            string ack = SendCommand(d, "set", jsontext);
            return 0;
        }

        public int IntercomStart(Device d, string s_ip, int s_port, string streamtype, string protocol, int volume, int gain, string source, string aec)
        {
            if (d == null) return -1;
            int m = 0;
            CommandStart cs2 = new CommandStart();
            cs2.command = "start";
            cs2.streamtype = streamtype;
            cs2.dataserver = s_ip;
            cs2.dataserverport = s_port;
            cs2.mode = "recvsend";
            cs2.volume = volume;
            cs2.samplerate = 8000;
            cs2.inputgain = gain;
            cs2.inputsource = source;
            cs2.protocol = protocol;
            cs2.buffer = "disable";
            cs2.aecmode = aec;
            cs2.param = d.id.ToString();
            string jsontext = JsonConvert.SerializeObject(cs2);
            string ack;
            lock (d)
            {
                ack = SendCommand(d, "start", jsontext);
            }
            if (ack != null)
            {
                CommandStartAck cs = JsonConvert.DeserializeObject<CommandStartAck>(ack);
                if ((cs.result == 100)||(cs.result == 200))
                {
                    return 0;
                }
            }
            return -1;
        }

        public int IntercomStop(Device d)
        {
            return AudioClose(d);
        }

        public int IntercomEmergencyStart(Device d, string s_ip, int s_port, string streamtype, string protocol, int volume, int gain, string source, string aec)
        {
            if (d == null) return -1;
            CommandStart cs2 = new CommandStart();

            cs2.command = "emergency_start";
            cs2.streamtype = streamtype;
            cs2.dataserver = s_ip;
            cs2.dataserverport = s_port;
            cs2.mode = "recvsend";
            cs2.volume = volume;
            cs2.samplerate = 8000;
            cs2.inputgain = gain;
            cs2.inputsource = source;
            cs2.protocol = protocol;
            cs2.buffer = "disable";
            cs2.aecmode = aec;
            cs2.param = d.id.ToString();
            string jsontext = JsonConvert.SerializeObject(cs2);
            string ack;
            lock (d)
            {
                ack = SendCommand(d, "emergency_start", jsontext);
            }
            if (ack != null)
            {
                CommandStartAck cs = JsonConvert.DeserializeObject<CommandStartAck>(ack);
                if ((cs.result == 100) || (cs.result == 200))
                {
                    return 0;
                }
            }
            return -1;
        }

        public int IntercomEmergencyStop(Device d)
        {
            if (d == null) return -1;

            CommandStop cs = new CommandStop();
            cs.command = "emergency_stop";
            string jsontext = JsonConvert.SerializeObject(cs);
            string ack;
            lock (d)
            {
                ack = SendCommand(d, "emergency_stop", jsontext);
            }

            if (ack != null)
            {
                CommandStop cs1 = JsonConvert.DeserializeObject<CommandStop>(ack);
                if ((cs1.result >= 200) && (cs1.result < 300))
                {
                    return 0;
                }
            }
            return -1;
        }

        public int SDCardGetInfo(Device d, out string disk_size, out string free_size)
        {
            string jsontext;
            Command cmd = new Command
            {
                command = "get_disk_info"
            };
            jsontext = JsonConvert.SerializeObject(cmd);
            string ack = null;
            lock (d)
            {
                ack = SendCommand(d, "get_disk_info", jsontext);
            }
            if (ack != null)
            {
                CommandDiskAck cr = JsonConvert.DeserializeObject<CommandDiskAck>(ack);
                if (cr.result == 200)
                {
                    disk_size = cr.disk_size;
                    free_size = cr.free_size;
                    return 0;
                }
            }
            disk_size = null;
            free_size = null;
            return -1;
        }

        public int SDCardGetFirstFile(Device d, out string FileName, out int FileSize)
        {
            string jsontext;
            Command cmd = new Command
            {
                command = "get_first_file"
            };
            jsontext = JsonConvert.SerializeObject(cmd);
            string ack = null;
            lock (d)
            {
                ack = SendCommand(d, "get_first_file", jsontext);
            }
            if (ack != null)
            {
                CommandFileAck cf = JsonConvert.DeserializeObject<CommandFileAck>(ack);
                if ((cf.result == 200) && (cf.filename != null))
                {
                    FileName = cf.filename;
                    FileSize = cf.filesize;
                    return 0;
                }
            }
            FileName = null;
            FileSize = 0;
            return -1;
        }

        public int SDCardGetNextFile(Device d, out string filename, out int filesize)
        {
            string jsontext;
            Command cmd = new Command
            {
                command = "get_next_file"
            };
            jsontext = JsonConvert.SerializeObject(cmd);
            string ack = null;
            lock (d)
            {
                ack = SendCommand(d, "get_next_file", jsontext);

            }
            if (ack != null)
            {
                CommandFileAck cf = JsonConvert.DeserializeObject<CommandFileAck>(ack);
                if ((cf.result == 200) && (cf.filename != null))
                {
                    filename = cf.filename;
                    filesize = cf.filesize;
                    return 0;
                }
            }
            filename = null;
            filesize = 0;
            return -1;
        }

        public int SDCardDeleteFile(Device d, string fname)
        {
            int result = 0;
            string jsontext;
            CommandDeleteFile cmd = new CommandDeleteFile
            {
                command = "delete_file",
                filename = fname
            };
            jsontext = JsonConvert.SerializeObject(cmd);
            string ack = null;
            lock (d)
            {
                ack = SendCommand(d, "delete_file", jsontext);
            }
            
            if (ack != null)
            {
                CommandDeleteFileAck cdf = JsonConvert.DeserializeObject<CommandDeleteFileAck>(ack);
                if (cdf.result == 200)
                {
                    result = 0;
                }
            }
            result = -1;
            return result;
        }

        private class UploadFileThreadParam
        {
            public Device d;
            public string filepathname;
        }

        public int SDCardUploadFile(Device d, string s_ip, int s_port, string fname)
        {
            int result = 0;

            CommandWriteFile cwf = new CommandWriteFile
            {
                command = "write_file",
                dataserver = "0.0.0.0",
                dataserverport = s_port,
                filename = fname
            };
            string jsontext = JsonConvert.SerializeObject(cwf);
            string ack = null;
            lock (d)
            {
                ack = SendCommand(d, "write_file", jsontext);
            }
           
            if (ack != null)
            {
                CommandWriteFileAck cdf = JsonConvert.DeserializeObject<CommandWriteFileAck>(ack);
                if (cdf.result == 100)
                {
                    return 0;
                }
                else result = -2;
            }
            else result = -1;

            return result;
        }

        public int SDCardPlayFile(Device d, string fname, int vol)
        {
            int result = 0;
            string jsontext;
            CommandPlayFile cmd = new CommandPlayFile
            {
                command = "start",
                mode = "file",
                volume = vol,
                filename = fname
            };
            jsontext = JsonConvert.SerializeObject(cmd);
            string ack = null;
            lock (d)
            {
                ack = SendCommand(d, "start", jsontext);
            }
            if (ack != null)
            {
                CommandPlayFileAck cdf = JsonConvert.DeserializeObject<CommandPlayFileAck>(ack);
                if (cdf.result == 100)
                {
                    string ack2 = WaitAck(d, "start");
                    if (ack2 != null)
                    {
                        CommandPlayFileAck cdf2 = JsonConvert.DeserializeObject<CommandPlayFileAck>(ack2);
                        if (cdf2.result == 200)
                        {
                            result = 0;
                        }
                        else result = -4;
                    }
                    else result = -3;
                }
                else result = -2;
            }
            else result = -1;

            return result;
        }

        public int SDCardGetPlayFileStatus(Device d, out string filename, out int runtime, out int process)
        {
            string jsontext;
            Command cmd = new Command
            {
                command = "status_playfile"
            };
            jsontext = JsonConvert.SerializeObject(cmd);
            string ack = null;
            lock (d)
            {
                ack = SendCommand(d, "status_playfile", jsontext);
            }
            if (ack != null)
            {
                CommandPlayFileStatusAck cpf = JsonConvert.DeserializeObject<CommandPlayFileStatusAck>(ack);
                if (cpf.result == 200)
                {
                    filename = cpf.filename;
                    runtime = cpf.runtime;
                    process = cpf.process;
                    return 0;
                }
            }
            filename = null;
            runtime = 0;
            process = 0;
            return -1;
        }

        public int SDCardPlayFileStop(Device d)
        {
            return AudioClose(d);
        }

        public int FilePlayStart(Device d, string s_ip, int s_port, string stype, int vol, string param)
        {
            int result = 0;

            CommandStart cs = new CommandStart();
            cs.command = "start";
            cs.streamtype = stype;
            cs.mode = "recvonly";
            cs.protocol = "tcp";
            cs.volume = vol;
            cs.param = param;
            cs.dataserver = s_ip;
            cs.dataserverport = s_port;

            string jsontext = JsonConvert.SerializeObject(cs);
            string ack = null;
            lock (d)
            {
                ack = SendCommand(d, "start", jsontext);
            }

            if (ack != null)
            {
                CommandStartAck csa = JsonConvert.DeserializeObject<CommandStartAck>(ack);
                if (csa.result == 100)
                {
                    result = 0;
                }
                else result = -2;
            }
            else result = -1;
            return result;
        }

        public int FilePlayStop(Device d)
        {
            return AudioClose(d);
        }
        
        public int FileEmergencyPlayStart(Device d, string s_ip, int s_port, string fname, string stype, int vol, string param)
        {
            int result = 0;
            CommandStart cs = new CommandStart();
            cs.command = "emergency_start";
            cs.streamtype = stype;
            cs.mode = "recvonly";
            cs.protocol = "rtp";
            cs.volume = vol;
            cs.param = d.id.ToString();
            cs.dataserver = s_ip;
            cs.dataserverport = s_port;

            string jsontext = JsonConvert.SerializeObject(cs);
            string ack = null;
            lock (d)
            {
                ack = SendCommand(d, "emergency_start", jsontext);
            }

            if (ack != null)
            {
                CommandStartAck csa = JsonConvert.DeserializeObject<CommandStartAck>(ack);
                if (csa.result == 100)
                {
                    result = 0;
                }
                else result = -2;

            }
            else result = -1;
            return result;
        }

        public int FileEmergencyPlayStop(Device d)
        {
            if (d == null) return -1;

            Command cs = new Command();
            cs.command = "emergency_stop";
            string jsontext = JsonConvert.SerializeObject(cs);
            string ack;
            lock (d)
            {
                ack = SendCommand(d, "emergency_stop", jsontext);
            }

            if (ack != null)
            {
                CommandStop cs1 = JsonConvert.DeserializeObject<CommandStop>(ack);
                if ((cs1.result >= 200) && (cs1.result < 300))
                {
                    return 0;
                }
            }
            return -1;
        }
    }
}
