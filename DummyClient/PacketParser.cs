using Google.Protobuf;
using Jhnet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DummyClient
{
    public static class PacketParser
    {
        //패킷아이디로 생성자함수 매핑
        private static readonly Dictionary<PacketId, Func<IMessage>> _message_factory = new()
        {
            { PacketId.ScEcho, () => new SC_Echo() },
        };

        
        public static (PacketId, IMessage) Parse(byte[] packet)
        {
            if (packet.Length < 4)
                throw new ArgumentException("패킷크기가 헤더보다 작음");

            //패킷길이
            ushort packet_size = BitConverter.ToUInt16(packet, 0);

            //패킷아이디
            ushort packet_id = BitConverter.ToUInt16(packet, 2);
            PacketId id = (PacketId)packet_id;

            //body
            int body_offset = 4;
            int body_length = packet.Length - body_offset;
            byte[] body_data = new byte[body_length];
            Array.Copy(packet, body_offset, body_data, 0, body_length);

            //생성자 함수 가져오기
            if (!_message_factory.TryGetValue(id, out var constructor))
                throw new InvalidOperationException($"매핑이 되지않은 패킷 PacketId: {id}");
            //생성자 실행
            IMessage message = constructor();
            message.MergeFrom(body_data); //프로토버프 역직렬화
            //반환
            return (id, message);
        }
    }
}
