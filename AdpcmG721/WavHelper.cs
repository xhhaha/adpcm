using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace adpcm
{
    public class WavHelper 
    {

        #region 变量
        private MyWaveFormat mWavFormat;
        private int mSampleCount = 0;
        private string mFileName = "";
        private FileStream mWaveFile = null;
        private BinaryWriter mWriter = null;
        private int _sampleRate = 0;
        private short _channels = 0;
        private string _filePath = "";
        #endregion

        public WavHelper(string audioFileName, int sampleRate, short channels)
        {
            _filePath = audioFileName;
            _sampleRate = sampleRate;
            _channels = channels;
            this.mFileName = audioFileName;
            mWavFormat = new MyWaveFormat();
            mWavFormat.SamplesPerSecond = _sampleRate;
            mWavFormat.BitsPerSample = 16;
            mWavFormat.Channels = _channels;
            mWavFormat.BlockAlign = (short)(mWavFormat.Channels * (mWavFormat.BitsPerSample / 8));
            mWavFormat.AverageBytesPerSecond = mWavFormat.BlockAlign * mWavFormat.SamplesPerSecond;
            Start();
        }
        /// <summary>
        /// 结束写入并关闭文件
        /// </summary>
        private void Stop()
        {
            // 写WAV文件尾  
            mWriter.Seek(4, SeekOrigin.Begin);
            mWriter.Write((int)(mSampleCount + 36));
            mWriter.Seek(40, SeekOrigin.Begin);
            mWriter.Write(mSampleCount);

            mWriter.Close();
            mWaveFile.Close();
            mWriter = null;
            mWaveFile = null;
        }

        /// <summary>
        /// 存放的录音文件路径
        /// </summary>
        public string FilePath
        {
            get { return _filePath; }
        }

        public void Dispose()
        {
            Stop();
        }

        /// <summary>  
        /// 创建保存的波形文件,并写入必要的文件头.  
        /// </summary>  
        private void Start()
        {
            // Open up the wave file for writing.  
            mWaveFile = new FileStream(mFileName, FileMode.Create);
            mWriter = new BinaryWriter(mWaveFile);
            /**************************************************************************  
               Here is where the file will be created. A  
               wave file is a RIFF file, which has chunks  
               of data that describe what the file contains.  
               A wave RIFF file is put together like this:  
               The 12 byte RIFF chunk is constructed like this:  
               Bytes 0 - 3 :  'R' 'I' 'F' 'F'  
               Bytes 4 - 7 :  Length of file, minus the first 8 bytes of the RIFF description.  
                                 (4 bytes for "WAVE" + 24 bytes for format chunk length +  
                                 8 bytes for data chunk description + actual sample data size.)  
                Bytes 8 - 11: 'W' 'A' 'V' 'E'  
                The 24 byte FORMAT chunk is constructed like this:  
                Bytes 0 - 3 : 'f' 'm' 't' ' '  
                Bytes 4 - 7 : The format chunk length. This is always 16.  
                Bytes 8 - 9 : File padding. Always 1.  
                Bytes 10- 11: Number of channels. Either 1 for mono,  or 2 for stereo.  
                Bytes 12- 15: Sample rate.  
                Bytes 16- 19: Number of bytes per second.  
                Bytes 20- 21: Bytes per sample. 1 for 8 bit mono, 2 for 8 bit stereo or  16 bit mono, 4 for 16 bit stereo.  
                Bytes 22- 23: Number of bits per sample.  
                The DATA chunk is constructed like this:  
                Bytes 0 - 3 : 'd' 'a' 't' 'a'  
                Bytes 4 - 7 : Length of data, in bytes.  
                Bytes 8 -: Actual sample data.  
              ***************************************************************************/
            // Set up file with RIFF chunk info.  
            char[] ChunkRiff = { 'R', 'I', 'F', 'F' };
            char[] ChunkType = { 'W', 'A', 'V', 'E' };
            char[] ChunkFmt = { 'f', 'm', 't', ' ' };
            char[] ChunkData = { 'd', 'a', 't', 'a' };

            short shPad = 1;                // File padding  
            int nFormatChunkLength = 0x10;  // Format chunk length.  
            int nLength = 0;                // File length, minus first 8 bytes of RIFF description. This will be filled in later.
            short shBytesPerSample = 0;     // Bytes per sample.  

            // 一个样本点的字节数目  
            if (8 == mWavFormat.BitsPerSample && 1 == mWavFormat.Channels)
                shBytesPerSample = 1;
            else if ((8 == mWavFormat.BitsPerSample && 2 == mWavFormat.Channels) || (16 == mWavFormat.BitsPerSample && 1 == mWavFormat.Channels))
                shBytesPerSample = 2;
            else if (16 == mWavFormat.BitsPerSample && 2 == mWavFormat.Channels)
                shBytesPerSample = 4;

            // RIFF 块  
            mWriter.Write(ChunkRiff);
            mWriter.Write(nLength);
            mWriter.Write(ChunkType);

            // WAVE块  
            mWriter.Write(ChunkFmt);
            mWriter.Write(nFormatChunkLength);
            mWriter.Write(shPad);
            mWriter.Write(mWavFormat.Channels);
            mWriter.Write(mWavFormat.SamplesPerSecond);
            mWriter.Write(mWavFormat.AverageBytesPerSecond);
            mWriter.Write(shBytesPerSample);
            mWriter.Write(mWavFormat.BitsPerSample);

            // 数据块  
            mWriter.Write(ChunkData);
            mWriter.Write((int)0);   // The sample length will be written in later.
        }

        /// <summary>
        /// 每当采集到音频数据时，将其写入
        /// </summary>
        /// <param name="data"></param>
        public void WriteAudioData(byte[] data)
        {
            mWriter.Write(data, 0, data.Length);
            mSampleCount += data.Length;
        }



        public class MyWaveFormat
        {
            /// <summary>
            /// 采样率
            /// </summary>
            public int SamplesPerSecond;

            /// <summary>
            /// 采样位数
            /// </summary>
            public short BitsPerSample;

            /// <summary>
            /// 通道数
            /// </summary>
            public short Channels;

            /// <summary>
            /// 单位采样点的字节数
            /// </summary>
            public short BlockAlign;

            /// <summary>
            /// 每秒平均码率
            /// </summary>
            public int AverageBytesPerSecond;
        }



    }
}
