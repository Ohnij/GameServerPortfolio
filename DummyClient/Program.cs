using System;
using System.IO;
using System.Net.Sockets;
using System.Text;
using Google.Protobuf;
using Jhnet; //프로토버퍼 내가 파싱한 패킷 네임스페이스

namespace DummyClient
{
    class Program
    {
        public static void Main(string[] args)
        {

            TcpClient tcp_client = new TcpClient();
            try
            {
                tcp_client.Connect("127.0.0.1", 7777);
                Console.WriteLine("서버에 연결 되었습니다.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"서버 연결 실패 : {ex.Message}");
            }

            Console.WriteLine("서버에 보낼 메시지를 입력하세요. (exit 입력 시 종료)");
            NetworkStream stream = tcp_client.GetStream();
            while (true)
            {
                Console.Write("> ");
                string input = Console.ReadLine();
                //입력 없을시 다시처음부터
                if (string.IsNullOrEmpty(input)) 
                    continue;
                if (input.ToLower() == "exit")
                    break;

                Jhnet.CS_Echo packet = new Jhnet.CS_Echo { Message = input, Number = 1229 };
                byte[] buffer = PacketBuilder.Build(PacketId.CsEcho, packet);
                stream.Write(buffer, 0, buffer.Length);


                byte[] recv_buffer = new byte[1024];
                int recv_bytes = stream.Read(recv_buffer, 0, recv_buffer.Length);
                byte[] recv_packet = new byte[recv_bytes];
                Array.Copy(recv_buffer,0,recv_packet, 0, recv_bytes);//패킷만큼만 자르기
                var (packet_id, parse_packet) = PacketParser.Parse(recv_packet);
                switch(packet_id)
                {
                    case PacketId.CsEcho:
                    {
                        SC_Echo response = (SC_Echo)parse_packet;
                        Console.WriteLine($"[echo] n:{response.Number} / m:{response.Message}");
                    }
                    break;
                }


                ////패킷으로 보내기
                //CSP_ECHO packet = new CSP_ECHO { number = 1229, message = input };
                //byte[] buffer = packet.Serialize();

                ////문자->바이트 배열
                ////byte[] buffer = Encoding.UTF8.GetBytes(input);

                ////서버에 전송
                //stream.Write(buffer, 0, buffer.Length);


                ////패킷별 분류 필요하고 자동화 필요함..
                ////C#도 스레드 나눠야함.
                ////우선은 하드코딩
                //byte[] recv_buf = new byte[136]; 
                //int len = stream.Read(recv_buf, 0, recv_buf.Length);
                //if (len >= 136)
                //{
                //    SCP_ECHO response = SCP_ECHO.Deserialize(recv_buf);
                //    Console.WriteLine($"[echo] n:{response.number} / m:{response.message}");
                //}


                //에코받기
                //byte[] readbuffer = new byte[1000];
                //int transferredBytes = stream.Read(readbuffer);
                //string echoMsg = Encoding.UTF8.GetString(readbuffer);

                //Console.WriteLine($"서버에서 온 메세지 : {echoMsg} ({transferredBytes} bytes)");
            }

        }
    }
}

