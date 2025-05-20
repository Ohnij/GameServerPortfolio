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

        //나중에 메인은 명령어 입력만 하고
        //클라이언트 자체적으로 로직을 따로구성하기
        public static void Main(string[] args)
        {
            var tasks = new List<Task>();
            Client[] clients = new Client[3];
            for (int i = 0; i < clients.Length; i++)
            {
                clients[i] = new Client();
                tasks.Add(clients[i].RunClient());
            }


            Task.WaitAll(tasks.ToArray());

            return;
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


            Task.Run(() => ReceiveTask(stream));

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
                //byte[] buffer = PacketBuilder.Build(PacketId.CsEcho, packet);
                Span<byte> buffer = PacketBuilder.Build(PacketId.CsEcho, packet);

                stream.Write(buffer.ToArray(), 0, buffer.Length);

            }

        }


        static public void ReceiveTask(NetworkStream stream)
        {
            //우선 메모리 단편화 생각안하고 짜보자..
            List<byte> recv_buffer = new List<byte>();
            try
            {
                while (true)
                {
                    //header > size(16) > id(16) 
                    //(메모리비용)
                    byte[] temp_buffer = new byte[1024];
                    int recv_bytes = stream.Read(temp_buffer, 0, temp_buffer.Length);
                    if (recv_bytes == 0)
                        break; // 서버 종료됨

                    //일단 recv_buffer에 집어넣기
                    recv_buffer.AddRange(temp_buffer.Take(recv_bytes));

                    while(true)
                    {
                        //헤더도 파싱 못함
                        if (recv_buffer.Count < 4)
                            break;
                        UInt16 packet_size = BitConverter.ToUInt16(recv_buffer.ToArray(), 0);   // packet_length

                        //패킷 사이즈 덜도착
                        if (recv_buffer.Count < packet_size)
                            break;
                        byte[] recv_packet = recv_buffer.GetRange(0, packet_size).ToArray();
                        //0~파싱데이터만큼 삭제 (메모리비용)
                        recv_buffer.RemoveRange(0, packet_size);
                        var (packet_id, parse_packet) = PacketParser.Parse(recv_packet);

                        switch (packet_id)
                        {
                            case PacketId.ScEcho:
                            {
                                SC_Echo response = (SC_Echo)parse_packet;
                                Console.WriteLine($"[echo] n:{response.Number} / m:{response.Message}");
                            }
                            break;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[수신 오류] {ex.Message}");
            }
        }
    }
}

