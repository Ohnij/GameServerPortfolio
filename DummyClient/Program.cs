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

            TcpClient tcpClient = new TcpClient();
            try
            {
                tcpClient.Connect("127.0.0.1", 7777);
                Console.WriteLine("서버에 연결 되었습니다.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"서버 연결 실패 : {ex.Message}");
            }

            Console.WriteLine("서버에 보낼 메시지를 입력하세요. (exit 입력 시 종료)");
            NetworkStream stream = tcpClient.GetStream();
            while (true)
            {
                Console.Write("> ");
                string input = Console.ReadLine();
                //입력 없을시 다시처음부터
                if (string.IsNullOrEmpty(input)) 
                    continue;
                if (input.ToLower() == "exit")
                    break;

                //문자->바이트 배열
                byte[] buffer = Encoding.UTF8.GetBytes(input);

                //서버에 전송
                stream.Write(buffer, 0, buffer.Length);


                //에코받기
                byte[] readbuffer = new byte[1000];
                int transferredBytes = stream.Read(readbuffer);
                string echoMsg = Encoding.UTF8.GetString(readbuffer);

                Console.WriteLine($"서버에서 온 메세지 : {echoMsg} ({transferredBytes} bytes)");
            }

        }
    }
}

