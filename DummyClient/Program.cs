using System;
using System.IO;
using System.Net.Sockets;
using System.Text;


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

                //패킷으로 보내기
                CSP_ECHO packet = new CSP_ECHO { number = 1229, message = input };
                byte[] buffer = packet.Serialize();

                //문자->바이트 배열
                //byte[] buffer = Encoding.UTF8.GetBytes(input);

                //서버에 전송
                stream.Write(buffer, 0, buffer.Length);


                //패킷별 분류 필요하고 자동화 필요함..
                //C#도 스레드 나눠야함.
                //우선은 하드코딩
                byte[] recv_buf = new byte[136]; 
                int len = stream.Read(recv_buf, 0, recv_buf.Length);
                if (len >= 136)
                {
                    SCP_ECHO response = SCP_ECHO.Deserialize(recv_buf);
                    Console.WriteLine($"[echo] n:{response.number} / m:{response.message}");
                }


                //에코받기
                //byte[] readbuffer = new byte[1000];
                //int transferredBytes = stream.Read(readbuffer);
                //string echoMsg = Encoding.UTF8.GetString(readbuffer);

                //Console.WriteLine($"서버에서 온 메세지 : {echoMsg} ({transferredBytes} bytes)");
            }

        }
    }
}

