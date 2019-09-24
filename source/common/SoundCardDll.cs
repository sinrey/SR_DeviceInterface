using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace Sinrey.SoundCard
{
    public delegate int DelegateSoundCardCallBack(int MsgType, int WParam, int LParam, IntPtr OutputParam);
    
    class SoundCardDLL
    {
        public static string[] RESULT_CODE = {
            "no error",
            "unspecified error",
            "device ID out of range" ,
            "driver failed enable" ,
            "device already allocated",
            "device handle is invalid",
            "no device driver present",
            "memory allocation error",
            "function isn't supported",
            "error value out of range",
            "invalid flag passed",
            "invalid parameter passed",
            "handle being used",
            };

        private const string DLL_NAME = "SoundCardDll.dll";
        private static ManualResetEventSlim mEvent;
        static DelegateSoundCardCallBack SoundCardCallback;

        private static int pSoundCardCallBack(int MsgType, int WParam, int LParam, IntPtr OutputParam)
        {
            mEvent.Set();
            return 0;
        }

        [DllImport(DLL_NAME, EntryPoint = "SoundCardInit")]
        //int __stdcall WaveStart(unsigned int uSamplerate, WaveCallBack pCallBack);
        private static extern int _SoundCardInit(UInt32 nSamplerate, DelegateSoundCardCallBack pCallBack, UInt32 flag);

        [DllImport(DLL_NAME, EntryPoint = "SoundCardClose")]
        //int __stdcall WaveStop();
        private static extern int _SoundCardClose();

        //int __stdcall WaveReadData(char* pOutBuffer, int nOutBufferSize, int* pnReadBytes);
        [DllImport(DLL_NAME, EntryPoint = "SoundCardReadFrom")]
        private static extern int _SoundCardReadFrom(IntPtr pOutBuffer, UInt32 uOutBufferSize, ref UInt32 uReadBytes);

        //int __stdcall WaveWriteData(int nChannel, const char* pInBuffer, int nInBufferLength);
        [DllImport(DLL_NAME, EntryPoint = "SoundCardWriteTo")]
        private static extern int _SoundCardWriteTo(IntPtr pInBuffer, UInt32 uInBufferLength);

        //, ManualResetEventSlim mEvent
        public static int SoundCardInit(uint nSamplerate)
        {
            SoundCardCallback = new DelegateSoundCardCallBack(pSoundCardCallBack);
            mEvent = new ManualResetEventSlim(false);
            return _SoundCardInit(nSamplerate, SoundCardCallback,0);
        }

        public static int SoundCardClose()
        {
            mEvent.Dispose();
            return _SoundCardClose();
        }

        public static bool SoundCardWaitForInputData()
        {
            return mEvent.Wait(1000);
        }

        public static short[] SoundCardReadFrom(int RequestSample)
        {
            UInt32 uReadBytes = 0;
            short[] outbuf = null;
            IntPtr pbuf = Marshal.AllocHGlobal(sizeof(short)* RequestSample);
            int ret = _SoundCardReadFrom(pbuf, (UInt32)(2* RequestSample), ref uReadBytes);

            if ((ret == 0)&&(uReadBytes > 0))
            {
                outbuf = new short[uReadBytes / 2];
                Marshal.Copy(pbuf, outbuf, 0, (Int32)(uReadBytes / 2));
            }
            Marshal.FreeHGlobal(pbuf);
            return outbuf;
        }

        public static int SoundCardWriteTo(short[] inbuf)
        {
            IntPtr pbuf = Marshal.AllocHGlobal(sizeof(short)*inbuf.Length);
            Marshal.Copy(inbuf, 0, pbuf, inbuf.Length);
            int ret = _SoundCardWriteTo(pbuf, (UInt32)(sizeof(short)*inbuf.Length));
            Marshal.FreeHGlobal(pbuf);
            return ret;
        }
    }
}
