using DummyClient.Network;
using Google.Protobuf;
using Jhnet;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace DummyClient
{
    public class Client
    {
        private static readonly Dictionary<PacketId, Func<byte[], bool>> _packet_function = new()
        {
            { PacketId.S2CEcho, RecvPcket_SCP_ECHO },
        };

        enum CLIENT_STATE
        { 
            CREATED,
            CONNECTING,
            CONNECTED,
            DISCONNECTED,
        }



        private Task _send_task;
        private Task _receive_task;
        private TcpClient _tcp_client = new TcpClient();
        private NetworkStream _stream;
        private CLIENT_STATE _state = CLIENT_STATE.CREATED;

        //private RecvBuffer _recv_buffer;
        //private SendBuffer _send_buffer;
        private RecvBuffer _recv_buffer = new RecvBuffer();
        private SendBuffer _send_buffer = new SendBuffer();

        public Task RunClient()
        {
            try
            {
                _state = CLIENT_STATE.CONNECTING;
                _tcp_client.Connect("127.0.0.1", 7777);
                Console.WriteLine("서버에 연결 되었습니다.");
                _state = CLIENT_STATE.CONNECTED;
                _stream = _tcp_client.GetStream();


                _send_task = Task.Run(SendTask);
                _receive_task = Task.Run(ReceiveTask);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"서버 연결 실패 : {ex.Message}");
            }


            
            //_send_task = SendTask();
           

            return Task.WhenAll(_send_task, _receive_task);
         
        }

        private async void SendTask()
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


        private void ReceiveTask()
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
                        break; // 서버 종료됨
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
                            func(recv_packet[4..]);
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
            }
        }


        ///======================================= packet function ===============================================///
        private static bool RecvPcket_SCP_ECHO(byte[] packet_data)
        {
            var packet = SCP_Echo.Parser.ParseFrom(packet_data);
            Console.WriteLine($"[echo] n:{packet.Number} / m:{packet.Message}");
            return false;
        }
    }
}
