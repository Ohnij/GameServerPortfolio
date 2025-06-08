using Google.Protobuf;
using Jhnet;

namespace DummyClient.Network
{
    public static class PacketParser
    {
        //패킷아이디로 생성자함수 매핑
        private static readonly Dictionary<PacketId, Func<IMessage>> _message_factory = new()
        {
            { PacketId.S2CPing, () => new SCP_Ping() },
            { PacketId.S2CEcho, () => new SCP_Echo() },
            { PacketId.S2CLogin, () => new SCP_Login() },
            { PacketId.S2CCreateChar, () => new SCP_CreateChar() },
            { PacketId.S2CCharList, () => new SCP_CharList() },
            { PacketId.S2CSelectCharResult, () => new SCP_SelectCharResult() },
            { PacketId.S2CMyCharacterInfo, () => new SCP_MyCharacterInfo() },
            { PacketId.S2CEnterGame, () => new SCP_EnterGame() },
            { PacketId.S2CSpawn, () => new SCP_Spawn() },
            { PacketId.S2CDespawn, () => new SCP_Despawn() },
            { PacketId.S2CMove, () => new SCP_Move() },
        };

        public static PacketId GetPacketId(ReadOnlySpan<byte> data)
        {
            return (PacketId)BitConverter.ToUInt16(data.Slice(2, 2));
        }

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
