using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Newtonsoft.Json;

namespace Sinrey.DeviceInterface
{
    class DeviceInterfaceDll
    {
        public const uint MSGTYPE_NONE = 0;
        public const uint MSGTYPE_CONNECTED = 1;
        public const uint MSGTYPE_DISCONNECTED = 2;
        public const uint MSGTYPE_DEVICE_LOGIN = 3;
        public const uint MSGTYPE_DEVICE_LOGOUT = 4;

        public const uint RC_OK = 0;
        public const uint RC_UNKNOWN = 1;

        public class InterfaceMsg
        {
            public UInt32 msg;
            public UInt32 WParam;
            public UInt32 LParam;
        }

        public struct SR_USER_LOGIN_INFO
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 129)]
            public string sDeviceAddress;   // IPV4 & IPV6 
            public byte byRes;
            public Int16 wPort;     // 端口
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string sUserName;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string sPassword;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string byRes2;
        };

        public struct SR_DEVICEINFO
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 48)]
            public string sSerialNumber;
            public byte bySingleStartDTalkChan;
            public byte bySingleDTalkChanNums;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string byRes;
        };

        public struct FileList
        {
            public string fileName;
            public Int32 fileSize;
        }
        public class SDInformation
        {
            public Int32 totalCapacity;
            public Int32 surplusCapacity;
            public FileList[] audioFileList;
        }

        public const UInt32 UPLOAD_AUDIO_FILE_STORAGE = 56;
        public const UInt32 UPLOAD_AUDIO_FILE_RELEASE = 57;

        private const string DLL_NAME = "DeviceInterfaceDll.dll";

        private delegate Int32 DelegateExpectionCallBack(UInt32 MsgType, UInt32 WParam, UInt32 LParam, IntPtr OutputParam);
        private static int fExpectionCallBack(UInt32 MsgType, UInt32 WParam, UInt32 LParam, IntPtr pUser)
        {
            if ((MsgType == MSGTYPE_CONNECTED) || (MsgType == MSGTYPE_DISCONNECTED))
            {
                InterfaceMsg obj = new InterfaceMsg();
                obj.msg = MsgType;
                obj.WParam = WParam;//lUserID
                obj.LParam = LParam;//lHandle
                parent.BeginInvoke(EventConnect, obj);
            }
            else if ((MsgType == MSGTYPE_DEVICE_LOGIN) || (MsgType == MSGTYPE_DEVICE_LOGOUT))
            {
                InterfaceMsg obj = new InterfaceMsg();
                obj.msg = MsgType;
                obj.WParam = WParam;//lUserID
                obj.LParam = LParam;//lHandle
                parent.BeginInvoke(EventLogin, obj);
            }
            return 0;
        }

        [DllImport(DLL_NAME, EntryPoint = "SR_GetVersion")]
        private static extern UInt32 _SR_GetVersion();
        //SR_GetLastError
        [DllImport(DLL_NAME, EntryPoint = "SR_GetLastError")]
        private static extern UInt32 _SR_GetLastError();

        [DllImport(DLL_NAME, EntryPoint = "SR_Init")]
        private static extern UInt32 _SR_Init(UInt32 mode, UInt32 port);

        [DllImport(DLL_NAME, EntryPoint = "SR_Cleanup")]
        private static extern UInt32 _SR_Cleanup();

        [DllImport(DLL_NAME, EntryPoint = "SR_Login")]
        private static extern UInt32 _SR_Login(IntPtr pLoginInfo, IntPtr lpDeviceInfo);

        [DllImport(DLL_NAME, EntryPoint = "SR_SetExceptionCallBack")]
        private static extern UInt32 _SR_SetExceptionCallBack(DelegateExpectionCallBack pCallBack);

        [DllImport(DLL_NAME, EntryPoint = "SR_GetCapacity")]
        private static extern UInt32 _SR_GetCapacity(UInt32 lUserID, IntPtr lpJsonBuffer, UInt32 nJsonBufferSize, ref UInt32 nRecvBytes, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_GetCapacity(UINT32 lUserID, PCHAR lpJsonBuffer, UINT32 nJsonBufferSize, UINT32 *nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_UploadFile")]
        private static extern UInt32 _SR_UploadFile(ref UInt32 lUploadHandle, UInt32 lUserID, IntPtr sFileName, bool bCover, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_UploadFile_V40(UINT32* lUploadHandle, UINT32 lUserID, CONST CHAR* sFileName, UINT32 dwUploadType, BOOL bCover, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_UploadFileData")]
        private static extern UInt32 _SR_UploadFileData(UInt32 lUploadHandle, IntPtr lpInBuffer, UInt32 BufferSize, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_Upload_Process(UINT32* lUploadHandle, PCHAR lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_UploadFileClose")]
        private static extern UInt32 _SR_UploadFileClose(UInt32 lUploadHandle, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_UploadClose(UINT32 lUploadHandle, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_PlayFile")]
        private static extern UInt32 _SR_PlayFile(ref UInt32 lUploadHandle, UInt32 lUserID, IntPtr sFileName, UInt32 nVolume, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_UploadFile_V40(UINT32* lUploadHandle, UINT32 lUserID, CONST CHAR* sFileName, UINT32 dwUploadType, BOOL bCover, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_PlayFileData")]
        private static extern UInt32 _SR_PlayFileData(UInt32 lUploadHandle, IntPtr lpInBuffer, UInt32 BufferSize, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_Upload_Process(UINT32* lUploadHandle, PCHAR lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_PlayFileClose")]
        private static extern UInt32 _SR_PlayFileClose(UInt32 lUploadHandle, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_UploadClose(UINT32 lUploadHandle, LPVOID lpInputParam, LPVOID lpOutputParam)


        [DllImport(DLL_NAME, EntryPoint = "SR_DeleteFile")]
        private static extern UInt32 _SR_DeleteFile(UInt32 lUserID, IntPtr sFileName, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_DeleteFile(UINT32 lUserID, CONST PCHAR sFileName, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_PutFile")]
        private static extern UInt32 _SR_PutFile(UInt32 lUserID, IntPtr sFileName, UInt32 nVolume, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_PutFile(UINT32 lUserID, CONST CHAR* sFileName, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_PutFileStatus")]
        private static extern UInt32 _SR_PutFileStatus(UInt32 lUserID, ref UInt32 nProcess, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_SetPutFile_Status(UINT32 lUserID, UINT32* nProcess, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_PutFileClose")]
        private static extern UInt32 _SR_PutFileClose(UInt32 lUserID, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_PutFile_Stop(UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_VoiceCom")]
        private static extern UInt32 _SR_VoiceCom(ref UInt32 IVoiceComHandle, UInt32 lUserID, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_StartVoiceCom_V30(UINT32* IVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);

        [DllImport(DLL_NAME, EntryPoint = "SR_VoiceComData")]
        private static extern UInt32 _SR_VoiceComData(UInt32 IVoiceComHandle, IntPtr pSendDataBuffer, UInt32 nSendBufferSize, IntPtr pRecvDataBuffer,  UInt32 nRecvBufferSize, ref UInt32 nRecvBytes, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_VoiceCom_Data(UINT32 IVoiceComHandle, PCHAR pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam);

        [DllImport(DLL_NAME, EntryPoint = "SR_VoiceComClose")]
        private static extern UInt32 _SR_VoiceComClose(UInt32 IVoiceComHandle, UInt32 lUserID, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_StopVoiceCom(UINT32 iVoiceComHandle, LPVOID lpInputParam, LPVOID lpOutputParam);

        [DllImport(DLL_NAME, EntryPoint = "SR_Emergency")]
        private static extern UInt32 _SR_Emergency(ref UInt32 IVoiceComHandle, UInt32 lUserID, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_StartVoiceCom_V30(UINT32* IVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);

        [DllImport(DLL_NAME, EntryPoint = "SR_EmergencyData")]
        private static extern UInt32 _SR_EmergencyData(UInt32 IVoiceComHandle, IntPtr pSendDataBuffer, UInt32 nSendBufferSize, IntPtr pRecvDataBuffer, UInt32 nRecvBufferSize, ref UInt32 nRecvBytes, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_VoiceCom_Data(UINT32 IVoiceComHandle, PCHAR pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam);

        [DllImport(DLL_NAME, EntryPoint = "SR_EmergencyClose")]
        private static extern UInt32 _SR_EmergencyClose(UInt32 IVoiceComHandle, UInt32 lUserID, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_StopVoiceCom(UINT32 iVoiceComHandle, LPVOID lpInputParam, LPVOID lpOutputParam);

        [DllImport(DLL_NAME, EntryPoint = "SR_SetVolume")]
        private static extern UInt32 _SR_SetVolume(UInt32 lUserID, UInt32 volume);

        [DllImport(DLL_NAME, EntryPoint = "SR_Update")]
        private static extern UInt32 _SR_Update(ref UInt32 lUpdateHandle, UInt32 lUserID, UInt32 nMode, IntPtr sFileName, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_UploadFile_V40(UINT32* lUploadHandle, UINT32 lUserID, CONST CHAR* sFileName, UINT32 dwUploadType, BOOL bCover, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_UpdateData")]
        private static extern UInt32 _SR_UpdateData(UInt32 lUpdateHandle, IntPtr lpInBuffer, UInt32 BufferSize, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_Upload_Process(UINT32* lUploadHandle, PCHAR lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_UpdateClose")]
        private static extern UInt32 _SR_UpdateClose(UInt32 lUpdateHandle, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_UploadClose(UINT32 lUploadHandle, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_Apply")]
        private static extern UInt32 _SR_Apply(UInt32 lUpdateHandle);
        //UINT32 _stdcall SR_UploadClose(UINT32 lUploadHandle, LPVOID lpInputParam, LPVOID lpOutputParam)


        public delegate void EventHandler(InterfaceMsg obj);
        public static event EventHandler EventConnect;
        public static event EventHandler EventLogin;

        private static Form parent;
        private static DelegateExpectionCallBack pExpectionCallBack;

        public static uint SR_GetVersion()
        {
            return _SR_GetVersion();
        }
        public static uint SR_GetLastError()
        {
            return _SR_GetLastError();
        }
        public static uint SR_Init(Form f, int mode, int port)
        {
            parent = f;
            pExpectionCallBack = new DelegateExpectionCallBack(fExpectionCallBack);
            _SR_SetExceptionCallBack(pExpectionCallBack);
            return _SR_Init((UInt32)mode, (UInt32)port);
        }

        public static uint SR_Cleanup()
        {
            return _SR_Cleanup();
        }

        //返回值：
        //-2：内存错误
        //-1：重注册
        //>0：注册成功，返回的用户UserID，后续操作需要提供此UserID
        public static uint SR_Login(string ip, string username, string password)
        {
            SR_USER_LOGIN_INFO userInfo = new SR_USER_LOGIN_INFO();
            userInfo.sDeviceAddress = ip;
            userInfo.sUserName = username;
            userInfo.sPassword = password;

            IntPtr p1 = Marshal.AllocHGlobal(Marshal.SizeOf(userInfo));
            Marshal.StructureToPtr<SR_USER_LOGIN_INFO>(userInfo, p1, true);


            SR_DEVICEINFO devInfo = new SR_DEVICEINFO();
            IntPtr p2 = Marshal.AllocHGlobal(Marshal.SizeOf(devInfo));
            Marshal.StructureToPtr<SR_DEVICEINFO>(devInfo, p2, true);

            uint ret = _SR_Login(p1, p2);

            Marshal.FreeHGlobal(p1);
            Marshal.FreeHGlobal(p2);

            return ret;
        }

        //SR_GetCapacity函数耗时与sd卡内的文件成正比，如有必要请在线程内调用。
        public static uint SR_GetCapacity(UInt32 userid, out SDInformation scobj)
        {
            UInt32 nRecvBytes = 0;
            IntPtr jsonbuf = Marshal.AllocHGlobal(8 * 1024);
            IntPtr lpInputParam = new IntPtr();
            IntPtr lpOutputParam = new IntPtr();
            uint ret = _SR_GetCapacity(userid, jsonbuf, 8 * 1024, ref nRecvBytes, lpInputParam, lpOutputParam);
            scobj = null;
            String jsontext = null;
            if (ret == 0)
            {
                byte[] jsonbs = new byte[nRecvBytes];
                Marshal.Copy(jsonbuf, jsonbs, 0, (int)nRecvBytes);
                jsontext = System.Text.Encoding.Default.GetString(jsonbs);
                SDInformation sc = JsonConvert.DeserializeObject<SDInformation>(jsontext);
                scobj = sc;
                return 0;
            }

            Marshal.FreeHGlobal(jsonbuf);
            return 1;
        }

        //int _SR_UploadFile_V40(ref UInt32 lUploadHandle, UInt32 lUserID, IntPtr sFileName, UInt32 dwUploadType, bool bCover, IntPtr lpInputParam, IntPtr lpOutputParam);
        public static uint SR_UploadFile(out UInt32 lUploadHandle, UInt32 lUserID, string sFileName, bool bCover)
        {
            UInt32 Handle = 0;
            IntPtr fn = Marshal.StringToHGlobalAnsi(sFileName);
            uint ret = _SR_UploadFile(ref Handle, lUserID, fn, bCover,new IntPtr(0),new IntPtr(0));
            lUploadHandle = Handle;
            Marshal.FreeHGlobal(fn);
            return ret;
        }

        //private static extern int SR_Upload_Process(UInt32 lUploadHandle, IntPtr lpInBuffer, UInt32 BufferSize, IntPtr lpInputParam, IntPtr lpOutputParam);
        public static uint SR_UploadFileData(UInt32 lUploadHandle, byte[] lpInBuffer, int nSize)
        {
            //_SR_Upload_Process(UInt32 lUploadHandle, IntPtr lpInBuffer, UInt32 BufferSize, IntPtr lpInputParam, IntPtr lpOutputParam);
            IntPtr pbuf = Marshal.AllocHGlobal(nSize);
            Marshal.Copy(lpInBuffer, 0, pbuf, nSize);
            return _SR_UploadFileData(lUploadHandle, pbuf, (UInt32)nSize, new IntPtr(0), new IntPtr(0));
        }

        //private static extern int _SR_UploadClose(UInt32 lUploadHandle, IntPtr lpInputParam, IntPtr lpOutputParam);
        public static uint SR_UploadClose(UInt32 lUploadHandle)
        {
            return _SR_UploadFileClose(lUploadHandle, new IntPtr(0), new IntPtr(0));
        }

        public static uint SR_Update(out UInt32 lUpdateHandle, UInt32 lUserID, UInt32 nMode, string sFileName)
        {
            UInt32 Handle = 0;
            IntPtr fn = Marshal.StringToHGlobalAnsi(sFileName);
            uint ret = _SR_Update(ref Handle, lUserID, nMode, fn, new IntPtr(0), new IntPtr(0));
            lUpdateHandle = Handle;
            Marshal.FreeHGlobal(fn);
            return ret;
        }

        public static uint SR_UpdateData(UInt32 lUpdateHandle, byte[] lpInBuffer, int nSize)
        {
            IntPtr pbuf = Marshal.AllocHGlobal(nSize);
            Marshal.Copy(lpInBuffer, 0, pbuf, nSize);
            return _SR_UpdateData(lUpdateHandle, pbuf, (UInt32)nSize, new IntPtr(0), new IntPtr(0));
        }

        public static uint SR_UpdateClose(UInt32 lUpdateHandle)
        {
            return _SR_UpdateClose(lUpdateHandle, new IntPtr(0), new IntPtr(0));
        }
        public static uint SR_PlayFile(out UInt32 lUploadHandle, UInt32 lUserID, string sFileName, UInt32 nVolume)
        {
            UInt32 Handle = 0;
            IntPtr fn = Marshal.StringToHGlobalAnsi(sFileName);
            uint ret = _SR_PlayFile(ref Handle, lUserID, fn, nVolume, new IntPtr(0), new IntPtr(0));
            lUploadHandle = Handle;
            Marshal.FreeHGlobal(fn);
            return ret;
        }

        //private static extern int SR_Upload_Process(UInt32 lUploadHandle, IntPtr lpInBuffer, UInt32 BufferSize, IntPtr lpInputParam, IntPtr lpOutputParam);
        public static uint SR_PlayFileData(UInt32 lUploadHandle, byte[] lpInBuffer, int nSize)
        {
            //_SR_Upload_Process(UInt32 lUploadHandle, IntPtr lpInBuffer, UInt32 BufferSize, IntPtr lpInputParam, IntPtr lpOutputParam);
            IntPtr pbuf = Marshal.AllocHGlobal(nSize);
            Marshal.Copy(lpInBuffer, 0, pbuf, nSize);
            return _SR_PlayFileData(lUploadHandle, pbuf, (UInt32)nSize, new IntPtr(0), new IntPtr(0));
        }

        //private static extern int _SR_UploadClose(UInt32 lUploadHandle, IntPtr lpInputParam, IntPtr lpOutputParam);
        public static uint SR_PlayFileClose(UInt32 lUploadHandle)
        {
            return _SR_PlayFileClose(lUploadHandle, new IntPtr(0), new IntPtr(0));
        }

        public static uint SR_DeleteFile(UInt32 lUserID, string sFileName)
        {
            IntPtr fn = Marshal.StringToHGlobalAnsi(sFileName);
            uint ret = _SR_DeleteFile(lUserID, fn, new IntPtr(0), new IntPtr(0));
            Marshal.FreeHGlobal(fn);
            return ret;
        }


        public static uint SR_PutFile(UInt32 lUserID, string sFileName, uint volume)
        {
            IntPtr fn = Marshal.StringToHGlobalAnsi(sFileName);
            uint ret = _SR_PutFile(lUserID, fn, volume, new IntPtr(0), new IntPtr(0));
            Marshal.FreeHGlobal(fn);
            return ret;
        }

        public static uint SR_PutFileStatus(UInt32 lUserID, out UInt32 nProcess)
        {
            UInt32 process = 0 ;
            uint ret = _SR_PutFileStatus(lUserID, ref process, new IntPtr(0), new IntPtr(0));
            nProcess = process;
            return ret;
        }

        public static uint SR_PutFileClose(UInt32 lUserID)
        {
            uint ret = _SR_PutFileClose(lUserID, new IntPtr(0), new IntPtr(0));
            return ret;
        }

        public static uint SR_VoiceCom(out UInt32 IVoiceComHandle, UInt32 lUserID)
        {
            UInt32 handle = 0 ;
            uint ret = _SR_VoiceCom(ref handle, lUserID, new IntPtr(0), new IntPtr(0));
            IVoiceComHandle = handle;
            return ret;
        }

        //将sendbuf数据发送给设备，返回从设备接收到的数据
        public static short[] SR_VoiceComData(UInt32 IVoiceComHandle, short[] sendbuf)
        {
            short[] result = null;
            UInt32 nRecvBytes = 0;
            Int32 bufsize = sendbuf.Length;
            IntPtr sbuf = Marshal.AllocHGlobal(2*bufsize);
            IntPtr rbuf = Marshal.AllocHGlobal(2*bufsize);
            Marshal.Copy(sendbuf, 0, sbuf, sendbuf.Length);
            uint ret = _SR_VoiceComData(IVoiceComHandle, sbuf, (UInt32)(2*bufsize), rbuf, (UInt32)(2 * bufsize), ref nRecvBytes, new IntPtr(0), new IntPtr(0));
            if (ret > 0)
            {
                result = new short[nRecvBytes/2];
                Marshal.Copy(rbuf, result, 0, (int)nRecvBytes/2);
            }
            Marshal.FreeHGlobal(sbuf);
            Marshal.FreeHGlobal(rbuf);

            return result;
        }

        public static uint SR_VoiceComClose(UInt32 IVoiceComHandle, UInt32 lUserID)
        {
            return _SR_VoiceComClose(IVoiceComHandle, lUserID, new IntPtr(0), new IntPtr(0));
        }

        public static uint SR_Emergency(out UInt32 IEmergencyHandle, UInt32 lUserID)
        {
            UInt32 handle = 0;
            uint ret = _SR_Emergency(ref handle, lUserID, new IntPtr(0), new IntPtr(0));
            IEmergencyHandle = handle;
            return ret;
        }

        //将sendbuf数据发送给设备，返回从设备接收到的数据
        public static short[] SR_EmergencyData(UInt32 IEmergencyHandle, short[] sendbuf)
        {
            short[] result = null;
            UInt32 nRecvBytes = 0;
            Int32 bufsize = sendbuf.Length;
            IntPtr sbuf = Marshal.AllocHGlobal(2 * bufsize);
            //IntPtr rbuf = Marshal.AllocHGlobal(2 * bufsize);
            Marshal.Copy(sendbuf, 0, sbuf, sendbuf.Length);
            uint ret = _SR_EmergencyData(IEmergencyHandle, sbuf, (UInt32)(2 * bufsize), new IntPtr(0), 0, ref nRecvBytes, new IntPtr(0), new IntPtr(0)) ;
            //if (ret > 0)
            //{
            //    result = new short[nRecvBytes / 2];
            //    Marshal.Copy(rbuf, result, 0, (int)nRecvBytes / 2);
            //}
            Marshal.FreeHGlobal(sbuf);
            //Marshal.FreeHGlobal(rbuf);

            return result;
        }

        public static uint SR_EmergencyClose(UInt32 IEmergencyHandle, UInt32 userid)
        {
            return _SR_EmergencyClose(IEmergencyHandle, userid, new IntPtr(0), new IntPtr(0));
        }

        public static uint SR_SetVolume(UInt32 userid, UInt32 volume)
        {
            return _SR_SetVolume(userid, volume);
        }

        public static uint SR_Apply(UInt32 userid)
        {
            return _SR_Apply(userid);
        }
    }
}
