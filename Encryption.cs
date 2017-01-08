using System;
using System.IO;
using System.Collections.Generic;
using System.Data;
using System.Text;

namespace SafeSkype
{
    public class SkypeEncryptor
    {
        #region Encryption and Decryption

        public static byte[] Encrypt(byte[] data, string password)
        {
            if (data == null || data.Length == 0)
            {
                return null;
            }

            byte[] passwordbytes = System.Text.Encoding.Unicode.GetBytes(password);

            byte[] result = new byte[data.Length];
            for (int i = 0; i < data.Length; i++)
            {
                result[i] = (byte)(data[i] ^ i ^ passwordbytes[i % passwordbytes.Length]);
            }

            return result;
        }

        public static byte[] Decrypt(byte[] data, string password)
        {
            // simple demo
            return Encrypt(data, password);
        }


        #endregion

    }
}
