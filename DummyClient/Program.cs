using System;


namespace DummyClient
{
    class Program
    {

        enum DummyClientMode
        {
            NONE,   //설정전.
            MANUAL, // 수동
            AUTO    // 자동
        }

        public const int CLIENT_COUNT = 100;
        public static void Main(string[] args)
        {
           
            //메인은 무조건 입력처리만 담당 한다!!!.
            DummyClientMode mode = DummyClientMode.NONE;
            Task main_task;
            while (true)
            {
                Console.WriteLine("클라이언트 모드를 선택하세요: manual / auto");
                Console.Write(">");
                string input = Console.ReadLine()?.Trim().ToLower(); //Null아니면 공백제거, 소문자로.
                if (input == null)
                    continue;

                if (input == "manual")
                {
                    mode = DummyClientMode.MANUAL;
                    main_task = RunManualMode();
                    break;
                }
                else if (input == "auto")
                {
                    mode = DummyClientMode.AUTO;
                    main_task = RunAutoMode();
                    break;
                }
                else
                    Console.WriteLine("잘못된 입력입니다.");
            }
           
            Task.WaitAll(main_task); //main_task끝날때까지 RunManualMode or RunAutoMode
        }

        static async Task RunManualMode()
        {
            while (true)
            {
                var client = new Client();
                bool is_connect = await client.Connect();
                if (is_connect)
                {
                    //Receive 루프
                    Task recv_task = Task.Run(client.ReceiveLoop);
                    // 입력 루프 시작
                    Task input_task = Task.Run(() => InputLoop(client));

                    await Task.WhenAll(recv_task, input_task); //두개 Task(함수)가 끝날때까지 기다리기 (종료안됨)
                }

                while (true)
                {
                    Console.WriteLine("연결이 끊어졌습니다. 다시 연결하시겠습니까? (y/n)");
                    Console.Write("> ");
                    var input = Console.ReadLine()?.Trim().ToLower();
                    if (input == "y")
                        break; // 다시 루프 시작
                    else if (input == "n")
                        return;
                    else
                        Console.WriteLine("[y 또는 n 중 하나로 입력해주세요]");
                }

            }
        }
        static void InputLoop(Client client)
        {
            while (client.IsConnected)
            {
                Console.Write(">");
                string command = Console.ReadLine()?.Trim().ToLower();
                if (command == null || false == client.IsConnected)
                    continue;

                Command.ExecuteCommand(command, client);
            }
        }


        static async Task RunAutoMode()
        {
            //Task를 여러개 생성하니까 Task가 한번에 생성이 안되는 문제발생
            //ThreadPool.SetMinThreads(CLIENT_COUNT*2, CLIENT_COUNT*2);
            var tasks = new List<Task>();
            Client[] clients = new Client[CLIENT_COUNT];
            for (int i = 0; i < clients.Length; i++)
            {
                clients[i] = new Client();
                await clients[i].Connect();
                tasks.Add(Task.Run(clients[i].ReceiveLoop));
            }
            //Task input_task = Task.Run(() => InputLoop_Auto(clients));

            Task.WaitAll(tasks.ToArray());
        }

       

        static void InputLoop_Auto(ref Client[] clients)
        {
            //클라이언트 개별동작 or 전체 동작

            while (true)
            {
                Console.Write("> ");
                string command = Console.ReadLine()?.Trim().ToLower();
                if (command == null)
                    continue;

                switch (command)
                {
                    case "login":
                        break;
                    case "quit":
                        Environment.Exit(0);
                        break;
                }
            }
        }
    }

}
