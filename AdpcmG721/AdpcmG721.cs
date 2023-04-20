using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static adpcm.Program;

namespace adpcm
{
    public class AdpcmG721
    {


        public unsafe static byte[] encode721(byte[] bytes)
        {
            RuntimeTypeHandle byteshandle = bytes.GetType().TypeHandle;
            IntPtr buff = (IntPtr)Marshal.AllocHGlobal(bytes.Length / 4);
            try
            {
                setadpcm_coder(bytes, buff, bytes.Length, new G72x_STATE().GetType().TypeHandle.Value);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.ToString());
            }
            byte[] result = new byte[(bytes.Length / 4)];
            for (int i = 0; i < result.Length; i++)
            {
                result[i] = Marshal.ReadByte(buff, i);
            }
            Marshal.FreeHGlobal(buff);//Close
            return result;
        }

        public unsafe static byte[] decode721(byte[] bytes)
        {
            IntPtr buff = (IntPtr)Marshal.AllocHGlobal(bytes.Length * 5);
            try
            {
                getadpcm_decoder(bytes, buff, bytes.Length * 2, new G72x_STATE().GetType().TypeHandle.Value);

            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
            byte[] result = new byte[(bytes.Length * 4)];
            for (int i = 0; i < result.Length; i++)
            {
                result[i] = Marshal.ReadByte(buff, i);
            }
            Marshal.FreeHGlobal(buff);//Close
            return result;
        }

        [DllImport("adpcm.dll")]
        private static extern void getadpcm_decoder(byte[] indata, IntPtr outdata, int len, IntPtr state);


        [DllImport("adpcm.dll", CharSet = CharSet.Auto)]
        private static extern void setadpcm_coder(byte[] indata, IntPtr outdata, int len, IntPtr state);


    }
}
