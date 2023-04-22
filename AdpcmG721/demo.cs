using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace adpcm
{
    public class demo
    {

        public byte[] bytesss { get; set; }


        public byte[] getsuijibyte()
        {
            Random rd = new Random();
            int i = rd.Next(1, 20);
            byte[] bytess = new byte[i];
            return bytess;
        }





    }
}
