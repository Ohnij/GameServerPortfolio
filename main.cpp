#pragma comment (lib, "ws2_32.lib")
#include <WinSock2.h>
#include <MSWSock.h>

#include <iostream>
#include <thread>
#include <vector>


int ErrorPrint(const char* ErrorDiscription)
{
	std::cerr << ErrorDiscription << ::WSAGetLastError() << std::endl;
	return -1;
}

#define PORT 7777

LPFN_ACCEPTEX g_AcceptEx = nullptr;
LPFN_CONNECTEX g_ConnectEx = nullptr;
LPFN_DISCONNECTEX g_DisconnectEx = nullptr;

enum class IOCP_WORK
{
	IOCP_ACCEPT,
	IOCP_RECV,
	IOCP_SEND,
};

struct OVERLAPPED_EX : public OVERLAPPED
{
	//SOCKET _socket;
	int _clientNumber;
	IOCP_WORK _type;
};

int main()
{
	//Wsa 라이브러리 로딩

	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		return ErrorPrint("WSAStartUp 오류!! >> ");
	}


	//연결 요청용 Listen소켓 세팅
	SOCKET _listenSocket;
	_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);


	//어떤 주소의 아이피던 해당 포트로 오는걸 받겠다는 의미
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(ADDR_ANY);

	//연결 요청용 소켓에 묶기
	if (0 != bind(_listenSocket, (sockaddr*)&addr, sizeof(SOCKADDR_IN)))
	{
		return ErrorPrint("Bind 오류!! >> ");
	}

	if (0 != listen(_listenSocket, SOMAXCONN))
	{
		return ErrorPrint("listen 오류!! >> ");
	}

	
	//SOCKADDR clientAddr;
	//SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	//int addrLen = sizeof(SOCKADDR);
	//char clientBuffer[1024];
	//비동기로 구현하기.
	//AccpetEX는 Accept와 다르게 블락이 되지 않고 IOCP의 GQCP를 통해 결과를 통보 받는다.
	//AcceptEx 함수를 사용하려면 MSWSock 헤더파일 필요
	//SOCKET client = AceeptEx(_listenSocket, &clientAddr, &addrLen);
	/*
	* //출처 https://learn.microsoft.com/ko-kr/windows/win32/api/mswsock/nf-mswsock-acceptex
	* 
	BOOL AcceptEx(
		  [in]  SOCKET       sListenSocket,			//listen소켓
		  [in]  SOCKET       sAcceptSocket,			//받아서쓸 소켓 (클라이언트)
		  [in]  PVOID        lpOutputBuffer,		//연결된 상대 주소 + 수신 데이터 받을 버퍼
		  [in]  DWORD        dwReceiveDataLength,	//연결 시 처음 수신할 데이터 길이 (0도 가능)
		  [in]  DWORD        dwLocalAddressLength,	//주소 길이 (>= sizeof(SOCKADDR_IN) + 16) (로컬주소)
		  [in]  DWORD        dwRemoteAddressLength,	//주소 길이 (>= sizeof(SOCKADDR_IN) + 16) (원격주소)
		  [out] LPDWORD      lpdwBytesReceived,		//실제 수신된 바이트 수 (비동기면 무시)
		  [in]  LPOVERLAPPED lpOverlapped			//IOCP 완료 통지를 위한 OVERLAPPED 포인터
		);
	

	함수포인터를 사용해야하는 이유
	https://learn.microsoft.com/en-us/archive/msdn-magazine/2000/october/windows-sockets-2-0-write-scalable-winsock-apps-using-completion-ports
	 함수 포인터를 미리 얻지 않고 함수를 호출하는 것(즉, mswsock.lib와 연결하고 AcceptEx를 직접 호출하는 것)은
	 AcceptEx가 Winsock2의 계층적 아키텍처 외부에 있기 때문에 비용이 많이 듭니다. 
	 AcceptEx는 애플리케이션이 실제로 mswsock 위에 계층화된 공급자에서 AcceptEx를 호출하려고 하는 경우( 그림 3 참조 )
	 모든 호출에 대해 WSAIoctl을 사용하여 함수 포인터를 요청해야 합니다. 
	 각 호출에서 이러한 상당한 성능 저하를 방지하려면 이러한 API를 사용하려는 애플리케이션은 WSAIoctl을 호출하여 
	 계층화된 공급자에서 직접 이러한 함수에 대한 포인터를 얻어야 합니다.

	*/

	//IOCP객체 생성
	HANDLE _iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	
	//IOCP에 Listen소켓 비동기로 받게 등록
	CreateIoCompletionPort((HANDLE)_listenSocket, _iocpHandle, 0, 0);


	//함수포인터를 받아오자 (권장)

	DWORD bytes = 0;
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidConnectEx = WSAID_CONNECTEX;
	GUID guidDisconnectEx = WSAID_DISCONNECTEX;
	WSAIoctl(_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,	
		&guidAcceptEx, sizeof(guidAcceptEx),
		&g_AcceptEx, sizeof(g_AcceptEx),
		&bytes, NULL, NULL);
	WSAIoctl(_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidConnectEx, sizeof(guidConnectEx),
		&g_ConnectEx, sizeof(g_ConnectEx),
		&bytes, NULL, NULL);
	WSAIoctl(_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidDisconnectEx, sizeof(guidDisconnectEx),
		&g_DisconnectEx, sizeof(g_DisconnectEx),
		&bytes, NULL, NULL);


	
	class Client
	{
	public:
		Client()
		{
			ZeroMemory(_reciveBuffer, 1024);
		}
		SOCKET _socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		OVERLAPPED_EX _recive;
		BYTE _reciveBuffer[1024];

		int _clientNumber;
	};

	//std::shared_ptr<Client>_client = std::make_shared<Client>();
	std::vector<std::shared_ptr<Client>> _clients;
	

	int addrLen = sizeof(SOCKADDR);
	const size_t ADDRESS_LEN = sizeof(SOCKADDR_IN) + 16;
	char dummyBuffer[ADDRESS_LEN * 2];
	DWORD dummyDword = 0;
	//OVERLAPPED overlapped;
	OVERLAPPED_EX* overlapped = new OVERLAPPED_EX();

	ZeroMemory(&overlapped, sizeof(OVERLAPPED_EX));
	//overlapped._socket = _clients.back()->_socket;
	int clientNum = _clients.size();
	overlapped->_clientNumber = clientNum;
	overlapped->_type = IOCP_WORK::IOCP_ACCEPT;
	_clients.emplace_back();
	_clients.back()->_clientNumber = clientNum;

	g_AcceptEx(
		_listenSocket,	//리슨소켓(Listen 함수를 이미 설정한 리슨용도의 소켓을 집어넣어야 받을수있음)
		_clients.back()->_socket,	//받아서 만들어진 소켓정보를 받을 세팅된 신규 소켓 Overlapped 세팅 필수
		&dummyBuffer,		//연결된 클라이언트의 로컬주소와 원격주소의 정보를 받아올 버퍼
		0,				//단순연결이므로 0으로 해도 무방 Accept시 수신 데이터 없음
		ADDRESS_LEN,	//공식 문서 패딩 목적
		ADDRESS_LEN,	//
		&dummyDword,	//전송된 바이트 정보
		overlapped);		//비동기 커널의 정보 + 확장버전으로 포인터를 넘겨서 추가정보를 파싱해서 사용할수있게 다시 받기위함


	std::thread workerThread([&] {

		/*
		BOOL GetQueuedCompletionStatus(
		  [in]  HANDLE       CompletionPort,				//완료 포트에 대한 핸들입니다.
				LPDWORD      lpNumberOfBytesTransferred,	//완료된 I/O 작업에서 전송된 바이트 수를 받는 변수에 대한 포인터입니다.
		  [out] PULONG_PTR   lpCompletionKey,				//I/O 작업이 완료된 파일 핸들과 연결된 완료 키 값을 받는 변수에 대한 포인터입니다.
		  [out] LPOVERLAPPED *lpOverlapped,
		  [in]  DWORD        dwMilliseconds					//완료 패킷이 완료 포트에 나타날 때까지 호출자가 대기하려는 시간(밀리초)입니다.
		);
		*/
		DWORD transferredByte = 0;
		ULONG_PTR key = 0;
		OVERLAPPED_EX* overlappedData = nullptr;

		while (true)
		{
			//여기서 클라이언트 객체로 뭔가가있어야할듯.
			BOOL res = GetQueuedCompletionStatus(_iocpHandle, &transferredByte, &key, reinterpret_cast<LPOVERLAPPED*>(&overlappedData), 1000);
			if (res)
			{
				int clientId = overlappedData->_clientNumber;
				switch (overlappedData->_type)
				{
					case IOCP_WORK::IOCP_ACCEPT:
					{
						std::shared_ptr<Client> pClient = _clients[clientId];
						//클라이언트 소켓을 IOCP에 등록 (RecvEX /  SendEX하기위해)
						CreateIoCompletionPort((HANDLE)pClient->_socket, _iocpHandle, 0, 0);

						WSABUF wsabuf;
						wsabuf.buf = reinterpret_cast<CHAR*>(pClient->_reciveBuffer);
						wsabuf.len = 1024;

						WSARecv(pClient->_socket, &wsabuf, 1, 0, 0, &pClient->_recive, nullptr);


						ZeroMemory(&overlapped, sizeof(OVERLAPPED_EX));
						//overlapped._socket = _clients.back()->_socket;
						int clientNum = _clients.size();
						overlapped->_clientNumber = clientNum;
						overlapped->_type = IOCP_WORK::IOCP_ACCEPT;
						_clients.emplace_back();
						_clients.back()->_clientNumber = clientNum;

						//나중에 함수로 빼던 해야할듯..
						g_AcceptEx(
							_listenSocket,	//리슨소켓(Listen 함수를 이미 설정한 리슨용도의 소켓을 집어넣어야 받을수있음)
							_clients.back()->_socket,	//받아서 만들어진 소켓정보를 받을 세팅된 신규 소켓 Overlapped 세팅 필수
							&dummyBuffer,		//연결된 클라이언트의 로컬주소와 원격주소의 정보를 받아올 버퍼
							0,				//단순연결이므로 0으로 해도 무방 Accept시 수신 데이터 없음
							ADDRESS_LEN,	//공식 문서 패딩 목적
							ADDRESS_LEN,	//
							&dummyDword,	//전송된 바이트 정보
							overlapped);		//비동기 커널의 정보 + 확장버전으로 포인터를 넘겨서 추가정보를 파싱해서 사용할수있게 다시 받기위함

					}
					break;
					case IOCP_WORK::IOCP_RECV:
					{
						std::shared_ptr<Client> pClient = _clients[clientId];

						char recivedata[1024];
						ZeroMemory(recivedata, 1024);
						memcpy( recivedata, pClient->_reciveBuffer, size_t(transferredByte));
						
						std::cerr << "[클라 " << clientId << "]받은 데이터 : " << recivedata << "(" << transferredByte << ")" << std::endl;


						//WSASend(); //내일해야할듯 너무시간이늦음..
						

						ZeroMemory(&pClient->_recive, sizeof(OVERLAPPED));
						WSABUF wsabuf;
						wsabuf.buf = reinterpret_cast<CHAR*>(pClient->_reciveBuffer);
						wsabuf.len = 1024;
						WSARecv(pClient->_socket, &wsabuf, 1, 0, 0, &pClient->_recive, nullptr);


					}
					break;
					case IOCP_WORK::IOCP_SEND:
					{

					}
					break;
				}
			}
			else
			{

			}
		}
	});


	if(workerThread.joinable())
		workerThread.join();

	//라이브러리 반환
	WSACleanup();
	return 0;
}


