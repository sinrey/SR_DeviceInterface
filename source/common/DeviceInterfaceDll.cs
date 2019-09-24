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
        public enum MSG_TYPE
        {
            MSGTYPE_NONE = 0,
            MSGTYPE_CONNECTED,
            MSGTYPE_DISCONNECTED,
            MSGTYPE_DEVICE_LOGIN,
            MSGTYPE_DEVICE_LOGOUT,
        };

        public const uint RC_OK = 0;
        public const uint RC_UNKNOWN = 1;

        public class InterfaceMsg
        {
            public MSG_TYPE msg;
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

        private delegate Int32 DelegateExpectionCallBack(MSG_TYPE MsgType, UInt32 WParam, UInt32 LParam, IntPtr OutputParam);
        private static int fExpectionCallBack(MSG_TYPE MsgType, UInt32 WParam, UInt32 LParam, IntPtr pUser)
        {
            if ((MsgType == MSG_TYPE.MSGTYPE_CONNECTED) || (MsgType == MSG_TYPE.MSGTYPE_DISCONNECTED))
            {
                InterfaceMsg obj = new InterfaceMsg();
                obj.msg = MsgType;
                obj.WParam = WParam;//lUserID
                obj.LParam = LParam;//lHandle
                parent.BeginInvoke(EventConnect, obj);
            }
            else if ((MsgType == MSG_TYPE.MSGTYPE_DEVICE_LOGIN) || (MsgType == MSG_TYPE.MSGTYPE_DEVICE_LOGOUT))
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
        private static extern UInt32 _SR_Init(UInt32 port);

        [DllImport(DLL_NAME, EntryPoint = "SR_Cleanup")]
        private static extern UInt32 _SR_Cleanup();

        [DllImport(DLL_NAME, EntryPoint = "SR_Login")]
        private static extern UInt32 _SR_Login(IntPtr pLoginInfo, IntPtr lpDeviceInfo);

        [DllImport(DLL_NAME, EntryPoint = "SR_SetExceptionCallBack_V30")]
        private static extern UInt32 _SR_SetExceptionCallBack_V30(DelegateExpectionCallBack pCallBack);

        [DllImport(DLL_NAME, EntryPoint = "SR_GetCapacity")]
        private static extern UInt32 _SR_GetCapacity(UInt32 lUserID, IntPtr lpJsonBuffer, UInt32 nJsonBufferSize, ref UInt32 nRecvBytes, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_GetCapacity(UINT32 lUserID, PCHAR lpJsonBuffer, UINT32 nJsonBufferSize, UINT32 *nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_UploadFile_V40")]
        private static extern UInt32 _SR_UploadFile_V40(ref UInt32 lUploadHandle, UInt32 lUserID, IntPtr sFileName, UInt32 dwUploadType, bool bCover, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_UploadFile_V40(UINT32* lUploadHandle, UINT32 lUserID, CONST CHAR* sFileName, UINT32 dwUploadType, BOOL bCover, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_Upload_Process")]
        private static extern UInt32 _SR_Upload_Process(UInt32 lUploadHandle, IntPtr lpInBuffer, UInt32 BufferSize, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_Upload_Process(UINT32* lUploadHandle, PCHAR lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_UploadClose")]
        private static extern UInt32 _SR_UploadClose(UInt32 lUploadHandle, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_UploadClose(UINT32 lUploadHandle, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_DeleteFile")]
        private static extern UInt32 _SR_DeleteFile(UInt32 lUserID, IntPtr sFileName, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_DeleteFile(UINT32 lUserID, CONST PCHAR sFileName, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_PutFile")]
        private static extern UInt32 _SR_PutFile(UInt32 lUserID, IntPtr sFileName, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_PutFile(UINT32 lUserID, CONST CHAR* sFileName, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_PutFile_Status")]
        private static extern UInt32 _SR_PutFile_Status(UInt32 lUserID, ref UInt32 nProcess, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_SetPutFile_Status(UINT32 lUserID, UINT32* nProcess, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_PutFile_Stop")]
        private static extern UInt32 _SR_PutFile_Stop(UInt32 lUserID, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_PutFile_Stop(UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)

        [DllImport(DLL_NAME, EntryPoint = "SR_StartVoiceCom_V30")]
        private static extern UInt32 _SR_StartVoiceCom_V30(ref UInt32 IVoiceComHandle, UInt32 lUserID, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_StartVoiceCom_V30(UINT32* IVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);

        [DllImport(DLL_NAME, EntryPoint = "SR_VoiceCom_Data")]
        private static extern UInt32 _SR_VoiceCom_Data(UInt32 IVoiceComHandle, IntPtr pSendDataBuffer, UInt32 nSendBufferSize, IntPtr pRecvDataBuffer,  UInt32 nRecvBufferSize, ref UInt32 nRecvBytes, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_VoiceCom_Data(UINT32 IVoiceComHandle, PCHAR pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam);

        [DllImport(DLL_NAME, EntryPoint = "SR_StopVoiceCom")]
        private static extern UInt32 _SR_StopVoiceCom(UInt32 IVoiceComHandle, UInt32 lUserID, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_StopVoiceCom(UINT32 iVoiceComHandle, LPVOID lpInputParam, LPVOID lpOutputParam);

        [DllImport(DLL_NAME, EntryPoint = "SR_StartEmergency")]
        private static extern UInt32 _SR_StartEmergency(ref UInt32 IVoiceComHandle, UInt32 lUserID, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_StartVoiceCom_V30(UINT32* IVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);

        [DllImport(DLL_NAME, EntryPoint = "SR_Emergency_Data")]
        private static extern UInt32 _SR_Emergency_Data(UInt32 IVoiceComHandle, IntPtr pSendDataBuffer, UInt32 nSendBufferSize, IntPtr pRecvDataBuffer, UInt32 nRecvBufferSize, ref UInt32 nRecvBytes, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_VoiceCom_Data(UINT32 IVoiceComHandle, PCHAR pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam);

        [DllImport(DLL_NAME, EntryPoint = "SR_StopEmergency")]
        private static extern UInt32 _SR_StopEmergency(UInt32 IVoiceComHandle, UInt32 lUserID, IntPtr lpInputParam, IntPtr lpOutputParam);
        //UINT32 _stdcall SR_StopVoiceCom(UINT32 iVoiceComHandle, LPVOID lpInputParam, LPVOID lpOutputParam);


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
        public static uint SR_Init(Form f, int port)
        {
            parent = f;
            pExpectionCallBack = new DelegateExpectionCallBack(fExpectionCallBack);
            _SR_SetExceptionCallBack_V30(pExpectionCallBack);
            return _SR_Init((UInt32)port);
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
        public static uint SR_UploadFile_V40(out UInt32 lUploadHandle, UInt32 lUserID, string sFileName, UInt32 dwUploadType, bool bCover)
        {
            UInt32 Handle = 0;
            IntPtr fn = Marshal.StringToHGlobalAnsi(sFileName);
            uint ret = _SR_UploadFile_V40(ref Handle, lUserID, fn, dwUploadType, bCover,new IntPtr(0),new IntPtr(0));
            lUploadHandle = Handle;
            Marshal.FreeHGlobal(fn);
            return ret;
        }

        //private static extern int SR_Upload_Process(UInt32 lUploadHandle, IntPtr lpInBuffer, UInt32 BufferSize, IntPtr lpInputParam, IntPtr lpOutputParam);
        public static uint SR_Upload_Process(UInt32 lUploadHandle, byte[] lpInBuffer, int nSize)
        {
            //_SR_Upload_Process(UInt32 lUploadHandle, IntPtr lpInBuffer, UInt32 BufferSize, IntPtr lpInputParam, IntPtr lpOutputParam);
            IntPtr pbuf = Marshal.AllocHGlobal(nSize);
            Marshal.Copy(lpInBuffer, 0, pbuf, nSize);
            return _SR_Upload_Process(lUploadHandle, pbuf, (UInt32)nSize, new IntPtr(0), new IntPtr(0));
        }

        //private static extern int _SR_UploadClose(UInt32 lUploadHandle, IntPtr lpInputParam, IntPtr lpOutputParam);
        public static uint SR_UploadClose(UInt32 lUploadHandle)
        {
            return _SR_UploadClose(lUploadHandle, new IntPtr(0), new IntPtr(0));
        }

        public static uint SR_DeleteFile(UInt32 lUserID, string sFileName)
        {
            IntPtr fn = Marshal.StringToHGlobalAnsi(sFileName);
            uint ret = _SR_DeleteFile(lUserID, fn, new IntPtr(0), new IntPtr(0));
            Marshal.FreeHGlobal(fn);
            return ret;
        }

        public static uint SR_PutFile(UInt32 lUserID, string sFileName)
        {
            IntPtr fn = Marshal.StringToHGlobalAnsi(sFileName);
            uint ret = _SR_PutFile(lUserID, fn, new IntPtr(0), new IntPtr(0));
            Marshal.FreeHGlobal(fn);
            return ret;
        }

        public static uint SR_PutFile_Status(UInt32 lUserID, out UInt32 nProcess)
        {
            UInt32 process = 0 ;
            uint ret = _SR_PutFile_Status(lUserID, ref process, new IntPtr(0), new IntPtr(0));
            nProcess = process;
            return ret;
        }

        public static uint SR_PutFile_Stop(UInt32 lUserID)
        {
            uint ret = _SR_PutFile_Stop(lUserID, new IntPtr(0), new IntPtr(0));
            return ret;
        }

        public static uint SR_StartVoiceCom_V30(out UInt32 IVoiceComHandle, UInt32 lUserID)
        {
            UInt32 handle = 0 ;
            uint ret = _SR_StartVoiceCom_V30(ref handle, lUserID, new IntPtr(0), new IntPtr(0));
            IVoiceComHandle = handle;
            return ret;
        }

        //将sendbuf数据发送给设备，返回从设备接收到的数据
        public static short[] SR_VoiceCom_Data(UInt32 IVoiceComHandle, short[] sendbuf)
        {
            short[] result = null;
            UInt32 nRecvBytes = 0;
            Int32 bufsize = sendbuf.Length;
            IntPtr sbuf = Marshal.AllocHGlobal(2*bufsize);
            IntPtr rbuf = Marshal.AllocHGlobal(2*bufsize);
            Marshal.Copy(sendbuf, 0, sbuf, sendbuf.Length);
            uint ret = _SR_VoiceCom_Data(IVoiceComHandle, sbuf, (UInt32)(2*bufsize), rbuf, (UInt32)(2 * bufsize), ref nRecvBytes, new IntPtr(0), new IntPtr(0));
            if (ret > 0)
            {
                result = new short[nRecvBytes/2];
                Marshal.Copy(rbuf, result, 0, (int)nRecvBytes/2);
            }
            Marshal.FreeHGlobal(sbuf);
            Marshal.FreeHGlobal(rbuf);

            return result;
        }

        public static uint SR_StopVoiceCom(UInt32 IVoiceComHandle, UInt32 lUserID)
        {
            return _SR_StopVoiceCom(IVoiceComHandle, lUserID, new IntPtr(0), new IntPtr(0));
        }

        public static uint SR_StartEmergency(out UInt32 IEmergencyHandle, UInt32 lUserID)
        {
            UInt32 handle = 0;
            uint ret = _SR_StartEmergency(ref handle, lUserID, new IntPtr(0), new IntPtr(0));
            IEmergencyHandle = handle;
            return ret;
        }

        //将sendbuf数据发送给设备，返回从设备接收到的数据
        public static short[] SR_Emergency_Data(UInt32 IEmergencyHandle, short[] sendbuf)
        {
            short[] result = null;
            UInt32 nRecvBytes = 0;
            Int32 bufsize = sendbuf.Length;
            IntPtr sbuf = Marshal.AllocHGlobal(2 * bufsize);
            //IntPtr rbuf = Marshal.AllocHGlobal(2 * bufsize);
            Marshal.Copy(sendbuf, 0, sbuf, sendbuf.Length);
            uint ret = _SR_Emergency_Data(IEmergencyHandle, sbuf, (UInt32)(2 * bufsize), new IntPtr(0), 0, ref nRecvBytes, new IntPtr(0), new IntPtr(0)) ;
            //if (ret > 0)
            //{
            //    result = new short[nRecvBytes / 2];
            //    Marshal.Copy(rbuf, result, 0, (int)nRecvBytes / 2);
            //}
            Marshal.FreeHGlobal(sbuf);
            //Marshal.FreeHGlobal(rbuf);

            return result;
        }

        public static uint SR_StopEmergency(UInt32 IEmergencyHandle, UInt32 userid)
        {
            return _SR_StopEmergency(IEmergencyHandle, userid, new IntPtr(0), new IntPtr(0));
        }
    }
}
