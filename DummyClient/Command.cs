using System.Reflection;

namespace DummyClient
{
    static class Command
    {
        private static readonly Dictionary<string, CommandEntry> _table;

        public static void ExecuteCommand(string cmd, Client c)
        {
            if(_table.TryGetValue(cmd, out var entry))
            {
                entry.Handler(c);
                return;
            }
            Console.WriteLine($"[지원하지 않는 명령어입니다] '{cmd}'");
        }

        //_table
        static Command() 
        {
            //미리 정의한 CommandEntry 로 생성해주기
            _table = new Dictionary<string, CommandEntry>();

            //Command class내부의 모든 static 함수 가져온다 (Client class의 함수는 안가져옴)
            var methods = typeof(Command).GetMethods(BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public);

            //
            foreach (var method in methods)
            {
                //메소드의 Attribute값 있는지 확인, 
                //선언한 CommandAttribute(string name, string description)가 붙어있는지 확인
                var attr = method.GetCustomAttribute<CommandAttribute>();
                if (attr == null)
                    continue;

                //파라미터 체크: Client 하나만 받는 함수로 걸러내기
                //메소드의 파라미터를 가져온다
                //파라미터가 1개가 아니면, 1개의 파리미터 타입이 Client가 아니면 캐싱 걸러내기
                var parameters = method.GetParameters();
                if (parameters.Length != 1 || parameters[0].ParameterType != typeof(Client))
                    continue;

                //CommandEntry의 변수 2개 만들어주기 (Action / string -> attribute의 discription  )
                //attribute 는 [Attribute명(내부변수1, 내부변수2....)
                //여기서 CommandAttribute 내부에 Name , Description으로 만들어줬기때문에 값을 가져올수 있음.
                //변수의 순서와 [Attribute(1,2)] 전방선언과 맵핑
                var action = (Action<Client>)Delegate.CreateDelegate(typeof(Action<Client>), method);
                _table[attr.Name] = new CommandEntry { Handler = action, Description = attr.Description };
            }
        }

        
        [CommandAttribute("echo", "서버에 echo 메시지를 보냅니다.")]
        private static void HandleEcho(Client client)
        {
            if (!client.IsConnected)
                return;
            Console.Write("메시지 입력: ");
            string message = Console.ReadLine();

            if (string.IsNullOrWhiteSpace(message))
            {
                Console.WriteLine("[입력된 메시지가 없습니다]");
                return;
            }

            client.SendEcho(message);
        }

        [CommandAttribute("login", "ID와 PW로 로그인 요청을 보냅니다.")]
        private static void HandleLogin(Client client)
        {
            if (!client.IsConnected)
                return;

            Console.Write("id입력: ");
            string id = Console.ReadLine();
            Console.Write("pw입력: ");
            string pw = Console.ReadLine();

            if (string.IsNullOrWhiteSpace(id) || string.IsNullOrWhiteSpace(pw))
            {
                Console.WriteLine("[입력된 id / pw 가 없습니다]");
                return;
            }

            client.SendLogin(id, pw);
        }

        [CommandAttribute("createchar", "캐릭터를 생성합니다")]
        private static void HandleCreateChar(Client client)
        {
            if (!client.IsConnected)
                return;

            Console.Write("nickname입력: ");
            string nickname = Console.ReadLine();
            Console.Write("jobcode입력(숫자): ");
            string job = Console.ReadLine();
        
            if (string.IsNullOrWhiteSpace(nickname) || string.IsNullOrWhiteSpace(job))
            {
                Console.WriteLine("[입력된 nickname / jobcode 가 없습니다]");
                return;
            }
            int jobcode = int.Parse(job);
            if(jobcode < 0)
            {
                Console.WriteLine("[jobcode value가 음수입니다]");
                return;
            }

            client.SendCreateChar(nickname,jobcode);
        }









        [CommandAttribute("help", "명령어 목록을 출력합니다.")]
        private static void HandleHelp(Client client)
        {
            Console.WriteLine("=== 명령어 목록 ===");
            foreach (var cmd in _table)
            {
                Console.WriteLine($"- {cmd.Key.PadRight(10)} : {cmd.Value.Description}");
            }
            Console.WriteLine("===================");
        }

        [CommandAttribute("exit", "프로그램을 종료합니다.")]
        private static void HandleExit(Client client)
        {
            Console.WriteLine("프로그램을 종료합니다.");
            Environment.Exit(0);
        }

    }

    [AttributeUsage(AttributeTargets.Method)]
    public sealed class CommandAttribute : Attribute //sealed 이후 상속 불가 C++ final
    {
        public string Name { get; }
        public string Description { get; }

        public CommandAttribute(string name, string description)
        {
            Name = name.ToLower();
            Description = description;
        }
    }

    public class CommandEntry
    {
        public Action<Client> Handler;
        public string Description;
    }
}
