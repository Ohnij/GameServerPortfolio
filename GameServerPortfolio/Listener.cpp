#include "Listener.h"
#include "IocpServer.h"
#include "Client.h"
#include "Util.h"
#include "ClientManager.h"

//AccpetEX는 Accept와 다르게 블락이 되지 않고 IOCP의 GQCP를 통해 결과를 통보 받는다.
//AcceptEx 함수를 사용하려면 MSWSock 헤더파일 필요
// 
//출처 https://learn.microsoft.com/ko-kr/windows/win32/api/mswsock/nf-mswsock-acceptex
//
//BOOL AcceptEx(
//	  [in]  SOCKET       sListenSocket,			//listen소켓
//	  [in]  SOCKET       sAcceptSocket,			//받아서쓸 소켓 (클라이언트)
//	  [in]  PVOID        lpOutputBuffer,		//연결된 상대 주소 + 수신 데이터 받을 버퍼
//	  [in]  DWORD        dwReceiveDataLength,	//연결 시 처음 수신할 데이터 길이 (0도 가능)
//	  [in]  DWORD        dwLocalAddressLength,	//주소 길이 (>= sizeof(SOCKADDR_IN) + 16) (로컬주소)
//	  [in]  DWORD        dwRemoteAddressLength,	//주소 길이 (>= sizeof(SOCKADDR_IN) + 16) (원격주소)
//	  [out] LPDWORD      lpdwBytesReceived,		//실제 수신된 바이트 수 (비동기면 무시)
//	  [in]  LPOVERLAPPED lpOverlapped			//IOCP 완료 통지를 위한 OVERLAPPED 포인터
//	);
//
//
//함수포인터를 사용해야하는 이유
//https://learn.microsoft.com/en-us/archive/msdn-magazine/2000/october/windows-sockets-2-0-write-scalable-winsock-apps-using-completion-ports
// 함수 포인터를 미리 얻지 않고 함수를 호출하는 것(즉, mswsock.lib와 연결하고 AcceptEx를 직접 호출하는 것)은
// AcceptEx가 Winsock2의 계층적 아키텍처 외부에 있기 때문에 비용이 많이 듭니다.
// AcceptEx는 애플리케이션이 실제로 mswsock 위에 계층화된 공급자에서 AcceptEx를 호출하려고 하는 경우( 그림 3 참조 )
// 모든 호출에 대해 WSAIoctl을 사용하여 함수 포인터를 요청해야 합니다.
// 각 호출에서 이러한 상당한 성능 저하를 방지하려면 이러한 API를 사용하려는 애플리케이션은 WSAIoctl을 호출하여
// 계층화된 공급자에서 직접 이러한 함수에 대한 포인터를 얻어야 합니다.


LPFN_ACCEPTEX g_AcceptEx = nullptr;
LPFN_CONNECTEX g_ConnectEx = nullptr;
LPFN_DISCONNECTEX g_DisconnectEx = nullptr;

Listener::~Listener()
{
	_iocp.reset();
}

bool Listener::Init(std::shared_ptr<IocpServer> iocp)
{
	_listen_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	//어떤 주소의 아이피던 해당 포트로 오는걸 받겠다는 의미
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(ADDR_ANY);


	//연결 요청용 소켓에 묶기
	if (0 != bind(_listen_socket, (sockaddr*)&addr, sizeof(SOCKADDR_IN)))
	{
		WSAError("Bind 오류!! >> ", ::WSAGetLastError());
		return false;
	}

	if (0 != listen(_listen_socket, SOMAXCONN))
	{
		WSAError("listen함수 오류!! >> ", ::WSAGetLastError());
		return false;
	}

	//함수포인터를 받아오자 (권장)

	DWORD bytes = 0;
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidConnectEx = WSAID_CONNECTEX;
	GUID guidDisconnectEx = WSAID_DISCONNECTEX;
	WSAIoctl(_listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx, sizeof(guidAcceptEx),
		&g_AcceptEx, sizeof(g_AcceptEx),
		&bytes, NULL, NULL);
	WSAIoctl(_listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidConnectEx, sizeof(guidConnectEx),
		&g_ConnectEx, sizeof(g_ConnectEx),
		&bytes, NULL, NULL);
	WSAIoctl(_listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidDisconnectEx, sizeof(guidDisconnectEx),
		&g_DisconnectEx, sizeof(g_DisconnectEx),
		&bytes, NULL, NULL);

	if (iocp == nullptr)
		return false;
	//순환잠조를 막기위해
	_iocp = iocp;

	return true;
}

void Listener::InitAcceptSocket(SOCKET& socket)
{
	socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
}

void Listener::RegisterAccept()
{
	// 클라이언트없이 연결요청
	// 바꾼이유 : 클라이언트 객체를 의미 연결이 되지 않았는데도 사용하는느낌이라 삭제함.

	OVERLAPPED_ACCEPT* accept_event = new OVERLAPPED_ACCEPT;
	//ZeroMemory(&_accept_event, sizeof(OVERLAPPED_ACCEPT));
	//_accept_event.type = IOCP_WORK::IOCP_ACCEPT;
	//InitAcceptSocket(_accept_event.socket);
	ZeroMemory(accept_event, sizeof(OVERLAPPED_ACCEPT));
	accept_event->type = IOCP_WORK::IOCP_ACCEPT;
	InitAcceptSocket(accept_event->socket);


	DWORD dummyDword = 0;
	g_AcceptEx(
		_listen_socket,						//리슨소켓(Listen 함수를 이미 설정한 리슨용도의 소켓을 집어넣어야 받을수있음)
		accept_event->socket,				//InitAcceptSocket에서 세팅후 이후 연결시 Socket을 클라이언트에 연결할것
		accept_event->accept_buffer,		//연결된 클라이언트의 로컬주소와 원격주소의 정보를 받아올 버퍼 (이후 클라이언트에 파싱하거나 할수있음
		0,									//단순연결이므로 0으로 해도 무방 Accept시 수신 데이터 없음
		sizeof(SOCKADDR_IN) + 16,			//공식 문서 패딩 목적
		sizeof(SOCKADDR_IN) + 16,			//
		&dummyDword,						//전송된 바이트 정보
		static_cast<LPOVERLAPPED>(accept_event));		//비동기 커널의 정보 + 확장버전으로 포인터를 넘겨서 추가정보를 파싱해서 사용할수있게 다시 받기위함

}

void Listener::ProcessAccept(LPOVERLAPPED overlappedPtr)
{
	OVERLAPPED_ACCEPT* acceptData = reinterpret_cast<OVERLAPPED_ACCEPT*>(overlappedPtr);

	//소켓이랑 버퍼로 변경했음.

	//acceptData->accept_buffer; //->주소 파싱
	std::shared_ptr<Client> p_client = ClientManager::Instance().CreateClient(acceptData->socket);
	if (p_client == nullptr)
	{
		//acceptData->socket; 연결 끊어야함.

		closesocket(acceptData->socket);
		std::cerr << "[서버가 가득찼습니다. close socket!! ]" << std::endl;
		RegisterAccept();
		return;
	}

	//리슨소켓과 연결?
	//마소문서나 블로그를 찾아봐도 왜 하는지 정확하게 알수가없었음
	//챗지피티에게 물어봤다.
	//주소정보, 원격주소정보, 소켓상태정보, 등 os에서 해당 소켓이 어떤 소켓으로부터 연결되었는지의 정보들이 담긴다고한다.
	//하지 않으면 기본소켓동작( getpeername(), getsockname(), shutdown()등 )에 비정상 동작을 할수있다고 한다.
	setsockopt(acceptData->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&_listen_socket, sizeof(_listen_socket));
	//클라이언트 네이글 끄기 (자잘한 패킷 모아쏘기 안씀!!)
	int flag = 1;
	setsockopt(acceptData->socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

	std::cerr << "[클라 " << p_client->GetClientID() << " 연결]" << std::endl;

	//클라이언트 소켓을 IOCP에 등록 (비동기 Recv / Send하기위해)
	_iocp->RegisterIOCP((HANDLE)p_client->GetSocket());
	p_client->RegisterRecv();


	RegisterAccept();
}

