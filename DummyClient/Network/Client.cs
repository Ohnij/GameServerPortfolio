using Jhnet;
using System.Net.Sockets;
using DummyClient.Network;

namespace DummyClient
{
    public class Client
    {
        private static readonly Dictionary<PacketId, Func<Client, byte[], bool>> _packet_function = new()
        {
            { PacketId.S2CPing, RecvPcket_SCP_PING },
            { PacketId.S2CEcho, RecvPcket_SCP_ECHO },
            { PacketId.S2CLogin, RecvPcket_SCP_LOGIN },
            { PacketId.S2CCreateChar, RecvPcket_SCP_CREATE_CHAR },
            { PacketId.S2CCharList, RecvPcket_SCP_CHAR_LIST },
        };

        enum CLIENT_STATE
        { 
            CREATED,
            DISCONNECTED,
            CONNECTING,
            CONNECTED,          //연결됨
            AUTHED,             //로그인완료
            CONNECTION_GAME,    //게임들어가는중
            INGAME,             //게임에 들어감
        }

        private TcpClient _tcp_client = new TcpClient();
        private NetworkStream _stream;

        private RecvBuffer _recv_buffer = new RecvBuffer();
        private SendBuffer _send_buffer = new SendBuffer();

        private CLIENT_STATE _state = CLIENT_STATE.CREATED;
        public bool IsConnected => _state >= CLIENT_STATE.CONNECTED;
        
        //public User = new User;

        public async Task<bool> Connect()
        {
            try
            {
                _state = CLIENT_STATE.CONNECTING;
                await _tcp_client.ConnectAsync("127.0.0.1", 7777);
                Console.WriteLine("서버에 연결 되었습니다.");
                _state = CLIENT_STATE.CONNECTED;
                _stream = _tcp_client.GetStream();
                return true;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"서버 연결 실패 : {ex.Message}");
                return false;
            }
        }



        private async void SendLoop()
        {
            //Console.WriteLine($"send 실행됨 at {DateTime.Now:HH:mm:ss.fff}");
            //Console.WriteLine("서버에 보낼 메시지를 입력하세요. (exit 입력 시 종료)");
            //나중에 랜덤숫자를 지정해서 숫자별 행동 ex 1. 무브, 2,채팅 .. 이런식으로
            try
            {
                Random random = new Random();
                while (true)
                {
                    await Task.Delay(1000);
                    //Console.Write("> ");
                    //string input = Console.ReadLine();

                    string input = random.Next(1000).ToString();
                    //입력 없을시 다시처음부터
                    if (string.IsNullOrEmpty(input))
                        continue;
                    if (input.ToLower() == "exit")
                        break;

                    Jhnet.CSP_Echo packet = new Jhnet.CSP_Echo { Message = input, Number = 1229 };
                    byte[] buffer = PacketBuilder.Build(PacketId.C2SEcho, packet);
                    //Span<byte> buffer = PacketBuilder.Build(PacketId.CsEcho, packet);
                    //Console.WriteLine($"[Send]");
                    _stream.Write(buffer.ToArray(), 0, buffer.Length);

                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[SendTask 종료] {ex.Message}");
            }
        }


        public void ReceiveLoop()
        {
            //Console.WriteLine($"Recevie 실행됨 at {DateTime.Now:HH:mm:ss.fff}");
            //우선 메모리 단편화 생각안하고 짜보자..
            try
            {
                while (_state == CLIENT_STATE.CONNECTED)
                {
                    //header > size(16) > id(16) 
                    Span<byte> write_span = _recv_buffer.GetWriteBuffer();
                    int recv_bytes = _stream.Read(write_span);

                    if (recv_bytes == 0)
                    {
                        _state = CLIENT_STATE.DISCONNECTED;
                        break; // 서버 종료됨
                    }
                    _recv_buffer.Write(recv_bytes);

                    while (true)
                    {
                        //헤더도 파싱 못함
                        Span<byte> read_span = _recv_buffer.GetReadBuffer();
                        if (read_span.Length < 4)
                            break;
                        UInt16 packet_size = BitConverter.ToUInt16(read_span.Slice(0, 2)); // packet_length

                        //패킷 사이즈 덜도착
                        if (read_span.Length < packet_size)
                            break;

                        byte[] recv_packet = read_span.Slice(0, packet_size).ToArray();
                        //0~파싱데이터만큼 삭제 (메모리비용)
                        _recv_buffer.Read(packet_size);
                        //recv_buffer.RemoveRange(0, packet_size);
                        //var (packet_id, parse_packet) = PacketParser.Parse(recv_packet);
                        var packet_id = PacketParser.GetPacketId(recv_packet);
                        if(_packet_function.TryGetValue(packet_id, out var func))
                        {
                            func(this, recv_packet[4..]); //4바이트 이후 건네주기
                        }
                        else
                        {
                            Console.WriteLine($"[비정상 패킷] {packet_id}");
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[수신 오류] {ex.Message}");
                _state = CLIENT_STATE.DISCONNECTED;
            }
        }


        ///======================================= Packet Function ===============================================///
        private static bool RecvPcket_SCP_PING(Client client, byte[] packet_data)
        {
            var packet = SCP_Ping.Parser.ParseFrom(packet_data);
            Console.WriteLine($"[ping] n:{packet.Number} / t:{packet.Timestamp}");
            return true;
        }

        private static bool RecvPcket_SCP_ECHO(Client client, byte[] packet_data)
        {
            var packet = SCP_Echo.Parser.ParseFrom(packet_data);
            Console.WriteLine($"[echo] n:{packet.Number} / m:{packet.Message}");
            return true;
        }

        private static bool RecvPcket_SCP_LOGIN(Client client, byte[] packet_data)
        {
            var packet = SCP_Login.Parser.ParseFrom(packet_data);
            Console.WriteLine($"[login] ok:{packet.LoginOk} / e:{packet.ErrorMessage}");
            if (packet.LoginOk)
                client.SendCharList();
            return true;
        }

        private static bool RecvPcket_SCP_CREATE_CHAR(Client client, byte[] packet_data)
        {
            var packet = SCP_CreateChar.Parser.ParseFrom(packet_data);
            Console.WriteLine($"[create_char] ok:{packet.CreateOk} / e:{packet.ErrorMessage}");

            if (packet.CreateOk)
                client.SendCharList();
            return true;
        }

        private static bool RecvPcket_SCP_CHAR_LIST(Client client, byte[] packet_data)
        {
            var packet = SCP_CharList.Parser.ParseFrom(packet_data);
            int i = 1;
            if(packet.Characters.Count ==0)
            {
                Console.WriteLine($"[char_list] character가 없습니다.");
                return true;
            }

            Console.WriteLine($"[char_list]");
            foreach (var c in packet.Characters)
            {
                Console.WriteLine($"[{i++}번 캐릭터]{c.Nickname}({c.CharacterUid}) job:{c.JobCode} level:{c.Level}");
            }
            return true;
        }

        ///======================================= Command Function ===============================================///

        public void SendEcho(string message)
        {
            var packet = new CSP_Echo
            {
                Message = message,
                Number = 1229 // 테스트용 숫자 (필요하면 고정 or 랜덤)
            };

            byte[] buffer = PacketBuilder.Build(PacketId.C2SEcho, packet);
            _stream.Write(buffer, 0, buffer.Length);
        }

        public void SendLogin(string id, string pw)
        {
            var packet = new CSP_Login
            {
                LoginId = id,
                LoginPw = pw
            };

            byte[] buffer = PacketBuilder.Build(PacketId.C2SLogin, packet);
            _stream.Write(buffer, 0, buffer.Length);
        }

        public void SendCharList()
        {
            var packet = new CSP_CharList { };

            byte[] buffer = PacketBuilder.Build(PacketId.C2SCharList, packet);
            _stream.Write(buffer, 0, buffer.Length);
        }

        public void SendCreateChar(string nickname, int job_code)
        {
            var packet = new CSP_CreateChar 
            { 
                JobCode = job_code,
                Name = nickname,
            };

            byte[] buffer = PacketBuilder.Build(PacketId.C2SCreateChar, packet);
            _stream.Write(buffer, 0, buffer.Length);
        }
    }
}
