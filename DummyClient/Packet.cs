
using System.Text;

namespace DummyClient
{

    public enum PacketType : ushort
    {
        CS_PING = 1001,
        CS_ECHO = 1002,


        SC_PING = 2001,
        SC_ECHO = 2002,
    }

    public class CSP_ECHO
    {
        public int number;
        public string message;

        //직렬화
        public byte[] Serialize()
        {
            byte[] msg_bytes = Encoding.UTF8.GetBytes(message);
            //Min 둘중 작은값반환.
            int msg_length = Math.Min(msg_bytes.Length, 128);

            //header 2+2 / int 4 / char 128 / 
            byte[] buffer = new byte[2 + 2 + 4 + 128];

            //패킷헤더
            //총크기       buffer 0번주소
            BitConverter.GetBytes((ushort)(2 + 2 + 4 + 128)).CopyTo(buffer, 0);    // packet_length
            //패킷id      buffer 2번주소 ->CopyTo(시작주소,offset);
            BitConverter.GetBytes((ushort)PacketType.CS_ECHO).CopyTo(buffer, 2);   // packet_id

            //패킷데이터
            //int       buffer의4번주소(헤더2+2끝)
            BitConverter.GetBytes(number).CopyTo(buffer, 4);                        // int number
            //char[]    buffer의 8번주소 (헤더2+2+int(4))
            Array.Copy(msg_bytes, 0, buffer, 8, msg_length);                          // message (max 128)

            return buffer;
        }
    }

    public class SCP_ECHO
    {
        public int number;
        public string message;

        public static SCP_ECHO Deserialize(byte[] buffer)
        {
            if (buffer.Length < 136)
                throw new Exception("패킷 길이 오류!");

            //ToUint16 -> buffer+0 에서부터 uint16으로 변환하기
            ushort data_size = BitConverter.ToUInt16(buffer, 0);
            //ToUint16 -> buffer+2 에서부터 uint16으로 변환하기
            ushort data_id = BitConverter.ToUInt16(buffer, 2);
            //ToInt32 -> buffer+4 에서부터 int32으로 변환하기
            int data_number = BitConverter.ToInt32(buffer, 4);
            //ToString -> buffer+8 에서부터 128만큼 string으로 변환하기, 마지막 널문자.
            //string data_message = Encoding.UTF8.GetString(buffer, 8, 128).TrimEnd('\0');
            string data_message = Encoding.UTF8.GetString(buffer, 8, 128).Split('\0')[0];

            return new SCP_ECHO { number = data_number, message = data_message};
        }
    }
}
