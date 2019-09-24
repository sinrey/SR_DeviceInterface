using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace G711
{
    public class g711
    {
        private const string DLL_NAME = "g711.dll";

        [DllImport(DLL_NAME, EntryPoint = "alaw2linear")]
        private static extern int _alaw2linear(byte val);

        [DllImport(DLL_NAME, EntryPoint = "linear2alaw")]
        private static extern byte _linear2alaw(int val);

        [DllImport(DLL_NAME, EntryPoint = "ulaw2linear")]
        private static extern int _ulaw2linear(byte val);

        [DllImport(DLL_NAME, EntryPoint = "linear2ulaw")]
        private static extern byte _linear2ulaw(int val);

        public static short[] g711Decode_alaw(byte[] a)
        {
            short[] pcm = new short[a.Length];
            for (int i = 0; i < a.Length; i++)
            {
                pcm[i] = (short)_alaw2linear(a[i]);
            }
            return pcm;
        }

        public static byte[] g711Encode_alwa(short[] pcm)
        {
            byte[] a = new byte[pcm.Length];
            for (int i = 0; i < a.Length; i++)
            {
                a[i] = _linear2alaw(pcm[i]);
            }
            return a;
        }

        public static short[] g711Decode_ulaw(byte[] a)
        {
            short[] pcm = new short[a.Length];
            for (int i = 0; i < a.Length; i++)
            {
                pcm[i] = (short)_ulaw2linear(a[i]);
            }
            return pcm;
        }

        public static byte[] g711Encode_ulwa(short[] pcm)
        {
            byte[] a = new byte[pcm.Length];
            for (int i = 0; i < a.Length; i++)
            {
                a[i] = _linear2ulaw(pcm[i]);
            }
            return a;
        }
    }
}
