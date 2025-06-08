using Google.Protobuf;
using Jhnet;

namespace DummyClient.Network
{
    public static class PacketBuilder
    {
        //프로토 버프에서 받은 내용 (proto 에서 선언한 Enum / 프로토버프에서 구조화된 객체 )
        //패킷의 형태로 헤더 붙여서 만들기
        public static byte[] Build(PacketId id, IMessage data)
        {
            //패킷데이터 직렬화
            byte[] body_data = data.ToByteArray();
            //헤더 id / size 2 + 2 + 나머지
            ushort packet_size = (ushort)(2 + 2 + body_data.Length);


            byte[] packet = new byte[packet_size];
            //패킷헤더
            //총크기       buffer 0번주소
            BitConverter.GetBytes(packet_size).CopyTo(packet, 0);    // packet_length
            //패킷id      buffer 2번주소 ->CopyTo(시작주소,offset);
            BitConverter.GetBytes((ushort)id).CopyTo(packet, 2);   // packet_id
            //패킷데이터
            Array.Copy(body_data, 0, packet, 4, body_data.Length);
            //리틀엔디안 기준
            return packet;
        }
    }
}
