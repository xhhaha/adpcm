using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Linq.Expressions;
using System.Net;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Windows.Forms;
using System.Xml.Linq;

namespace adpcm
{   /*
     author：35924908@qq.com
     Revision date 2023-04-20 am
     */
    internal class Program
    {

        #region Main Function
        unsafe static void Main(string[] args)
        {
            Stopwatch stopwatch = new Stopwatch();
            string url = "http://192.168.1.12:8080/?cmd=audio_data_request&voice_id=444169194790036434";
            //url = "http://192.168.1.12:8080/?cmd=audio_data_request&voice_id=15081149429123375989";
            string readpath = "C:\\Users\\Administrator\\Desktop\\daijiema.g721";
            string savepath = @"C:\Users\Administrator\Documents\Downloads\Music\";//  解码后保存地址
            
            for (int i = 0; i < 99999; i++)
            {
                stopwatch.Restart();
                byte[] data = HttpGetBytes(url);
                //byte[] data = localpath2bytes(readpath); // 读取用户计算机本地文件 已测 无问题                          
                #region 结构体 重新分配内存初始化
                var init = private_init_state();
                int size = Marshal.SizeOf(init);
                byte[] byteobj = new byte[size];
                IntPtr intPtr = Marshal.AllocHGlobal(size);
                Marshal.StructureToPtr(init, intPtr, false);
                Marshal.Copy(intPtr, byteobj, 0, size);
                #endregion

                adpcmg721towav(data, intPtr, savepath + Guid.NewGuid().ToString() + ".wav");// 如果文件写入失败 则需要查看 Windwos 当前登录的用户权限问题 和目录权限 NTFS 拒绝大于允许
                Console.WriteLine(string.Format("循环第{0}次执行完毕", i + 1));
                stopwatch.Stop();
                Console.WriteLine(string.Format("耗时:{0}ms", stopwatch.ElapsedMilliseconds));
            }
           
            Console.WriteLine("执行完毕请查看");
            Console.ReadKey();
        }
        #endregion

        #region localpath2bytes
        public static byte[] localpath2bytes(string path)
        {
            FileStream fsRead = new FileStream(path, FileMode.Open, FileAccess.Read);
            byte[] array = new byte[fsRead.Length];
            fsRead.Read(array, 0, array.Length);
            fsRead.Close();
            return array;
        }
        #endregion

        #region 默认结构体 实例化 
        /// <summary>
        /// 默认结构体 实例化 
        /// </summary>
        /// <returns></returns>
        public static G72x_STATE private_init_state()
        {
            G72x_STATE state_ptr = new G72x_STATE();
            int cnta;

            state_ptr.yl = 34816;
            state_ptr.yu = 544;
            state_ptr.dms = 0;
            state_ptr.dml = 0;
            state_ptr.ap = 0;
            for (cnta = 0; cnta < 2; cnta++)
            {
                state_ptr.a[cnta] = 0;
                state_ptr.pk[cnta] = 0;
                state_ptr.sr[cnta] = 32;
            }
            for (cnta = 0; cnta < 6; cnta++)
            {
                state_ptr.b[cnta] = 0;
                state_ptr.dq[cnta] = 32;
            }
            state_ptr.td = (char)0;
            return state_ptr;
        } 
        #endregion

        #region 默认结构体
        public struct G72x_STATE
        {
            public long yl;    /* Locked or steady state step size multiplier. */
            public short yu;   /* Unlocked or non-steady state step size multiplier. */
            public short dms;  /* Short term energy estimate. */
            public short dml;  /* Long term energy estimate. */
            public short ap;   /* Linear weighting coefficient of 'yl' and 'yu'. */

            public short[] a = new short[2];// a[2]; /* Coefficients of pole portion of prediction filter. */
            public short[] b = new short[6];// b[6]; /* Coefficients of zero portion of prediction filter. */
            public short[] pk = new short[2];// pk[2]; 
            /*
            ** Signs of previous two samples of a partially
            ** reconstructed signal.
            **/
            public short[] dq = new short[6];// dq[6]; 
            /*
            ** Previous 6 samples of the quantized difference
            ** signal represented in an internal floating point
            ** format.
            **/
            public short[] sr = new short[2];//
                                             //sr[2];    
            /*
            ** Previous 2 samples of the quantized difference
            ** signal represented in an internal floating point
            ** format.
            */
            public char td;    /* delayed tone detect, new in 1988 version */

            public G72x_STATE()
            {
                //short[] a = new short[2];// a[2]; /*
                //short[] b = new short[6];// b[6]; /*
                //short[] pk = new short[2];// pk[2]; 
            }
            /*	The following struct members were added for libsndfile. The original
            **	code worked by calling a set of functions on a sample by sample basis
            **	which is slow on architectures like Intel x86. For libsndfile, this
            **	was changed so that the encoding and decoding routines could work on
            **	a block of samples at a time to reduce the function call overhead.
            */
            //	short		(*encoder) (short, struct private_g72x* state) ;
            //	short		(*decoder) (short, struct private_g72x* state) ;

            //	int		codec_bits ;
            //	int		byte_index, sample_index ;
        }
        #endregion

        #region 未实现 无效函数
        /// <summary>
        ///  未实现 无效函数
        /// </summary>
        /// <param name="path"></param>
        /// <param name="savepath"></param>
        public unsafe static void encoder(string path, string savepath)
        {
            FileStream fs = new FileStream(path, FileMode.Open);// 实例初始化
            BinaryReader binaryReader = new BinaryReader(fs);
            byte[] bytes;
            bytes = binaryReader.ReadBytes((int)fs.Length);
            RuntimeTypeHandle byteshandle = bytes.GetType().TypeHandle;
            object a = new object();
            RuntimeTypeHandle charshandle = a.GetType().TypeHandle;
            setadpcm_coder(byteshandle.Value, charshandle.Value, bytes.Length, new G72x_STATE());
            char[] adas = (char[])a;
        }
        #endregion

        #region 定义wav文件生成类
        private static WavHelper microphoneWav = null;
        #endregion

        #region adpcmg721towav
        public unsafe static void adpcmg721towav(byte[] bytes, IntPtr state, string savepath)
        {
            byte[] result = AdpcmG721.decode721(bytes, state);
            microphoneWav = new WavHelper(savepath, 8000, 1);
            microphoneWav.WriteAudioData(result);
            microphoneWav.Dispose();
            microphoneWav = null;
        }
        #endregion

        #region adpcm api
        [DllImport("adpcm.dll", CallingConvention = CallingConvention.Cdecl)]
        static extern public int test();

        [DllImport("adpcm.dll")]
        public static extern void getadpcm_decoder(byte[] indata, IntPtr outdata, int len, IntPtr state);

        [DllImport("adpcm.dll", CharSet = CharSet.Auto)]
        public static extern void setadpcm_coder(IntPtr indata, IntPtr outdata, int len, G72x_STATE state);
        #endregion

        #region HttpGetBytes
        public static byte[] HttpGetBytes(string url)
        {
            byte[] result = new demo().getsuijibyte();
            // string dizhi=result.GetType().TypeHandle.Value.ToString();
            using (WebClient MyWebClient = new WebClient())
            {
                MyWebClient.Headers["User-Agent"] = "blah";
                MyWebClient.Credentials = CredentialCache.DefaultCredentials;
                result = MyWebClient.DownloadData(url);
                //return MyWebClient.DownloadData(url);
                //result =new byte[]{ 0, 1,2,3,4,56,7,8,9};
                MyWebClient.Dispose();
            }

            //try
            //{   // 说明 如果页面返回的是二进制 内容  不是唤起浏览器下载 则使用 WebClient 如果 服务端返回的是base64 或者其他直接可取数据
            //    // 则需要通过 HttpWebRequest 
            //    WebClient MyWebClient = new WebClient();

            //}
            //catch (Exception ex)
            //{
            //    Debug.WriteLine(ex.ToString());
            //}
            return result;
        }
        #endregion

        #region Stream2Bytes
        public static byte[] Stream2Bytes(Stream stream)
        {
            byte[] bytes = new byte[stream.Length];
            stream.Read(bytes, 0, bytes.Length);
            stream.Seek(0, SeekOrigin.Begin);
            return bytes;
        }
        #endregion

        #region StreamToMemoryStream
        public static MemoryStream StreamToMemoryStream(Stream instream)
        {
            MemoryStream outstream = new MemoryStream();
            const int bufferLen = 4096;
            byte[] buffer = new byte[bufferLen];
            int count = 0;
            while ((count = instream.Read(buffer, 0, bufferLen)) > 0)
            {
                outstream.Write(buffer, 0, count);
            }
            return outstream;
        }
        #endregion

    }
}
