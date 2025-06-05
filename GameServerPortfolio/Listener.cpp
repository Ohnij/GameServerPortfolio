#include "Listener.h"
#include "IocpServer.h"
#include "Client.h"
#include "Util.h"
#include "ClientManager.h"

//AccpetEX�� Accept�� �ٸ��� ����� ���� �ʰ� IOCP�� GQCP�� ���� ����� �뺸 �޴´�.
//AcceptEx �Լ��� ����Ϸ��� MSWSock ������� �ʿ�
// 
//��ó https://learn.microsoft.com/ko-kr/windows/win32/api/mswsock/nf-mswsock-acceptex
//
//BOOL AcceptEx(
//	  [in]  SOCKET       sListenSocket,			//listen����
//	  [in]  SOCKET       sAcceptSocket,			//�޾Ƽ��� ���� (Ŭ���̾�Ʈ)
//	  [in]  PVOID        lpOutputBuffer,		//����� ��� �ּ� + ���� ������ ���� ����
//	  [in]  DWORD        dwReceiveDataLength,	//���� �� ó�� ������ ������ ���� (0�� ����)
//	  [in]  DWORD        dwLocalAddressLength,	//�ּ� ���� (>= sizeof(SOCKADDR_IN) + 16) (�����ּ�)
//	  [in]  DWORD        dwRemoteAddressLength,	//�ּ� ���� (>= sizeof(SOCKADDR_IN) + 16) (�����ּ�)
//	  [out] LPDWORD      lpdwBytesReceived,		//���� ���ŵ� ����Ʈ �� (�񵿱�� ����)
//	  [in]  LPOVERLAPPED lpOverlapped			//IOCP �Ϸ� ������ ���� OVERLAPPED ������
//	);
//
//
//�Լ������͸� ����ؾ��ϴ� ����
//https://learn.microsoft.com/en-us/archive/msdn-magazine/2000/october/windows-sockets-2-0-write-scalable-winsock-apps-using-completion-ports
// �Լ� �����͸� �̸� ���� �ʰ� �Լ��� ȣ���ϴ� ��(��, mswsock.lib�� �����ϰ� AcceptEx�� ���� ȣ���ϴ� ��)��
// AcceptEx�� Winsock2�� ������ ��Ű��ó �ܺο� �ֱ� ������ ����� ���� ��ϴ�.
// AcceptEx�� ���ø����̼��� ������ mswsock ���� ����ȭ�� �����ڿ��� AcceptEx�� ȣ���Ϸ��� �ϴ� ���( �׸� 3 ���� )
// ��� ȣ�⿡ ���� WSAIoctl�� ����Ͽ� �Լ� �����͸� ��û�ؾ� �մϴ�.
// �� ȣ�⿡�� �̷��� ����� ���� ���ϸ� �����Ϸ��� �̷��� API�� ����Ϸ��� ���ø����̼��� WSAIoctl�� ȣ���Ͽ�
// ����ȭ�� �����ڿ��� ���� �̷��� �Լ��� ���� �����͸� ���� �մϴ�.


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

	//� �ּ��� �����Ǵ� �ش� ��Ʈ�� ���°� �ްڴٴ� �ǹ�
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(ADDR_ANY);


	//���� ��û�� ���Ͽ� ����
	if (0 != bind(_listen_socket, (sockaddr*)&addr, sizeof(SOCKADDR_IN)))
	{
		WSAError("Bind ����!! >> ", ::WSAGetLastError());
		return false;
	}

	if (0 != listen(_listen_socket, SOMAXCONN))
	{
		WSAError("listen�Լ� ����!! >> ", ::WSAGetLastError());
		return false;
	}

	//�Լ������͸� �޾ƿ��� (����)

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
	//��ȯ������ ��������
	_iocp = iocp;

	return true;
}

void Listener::InitAcceptSocket(SOCKET& socket)
{
	socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
}

void Listener::RegisterAccept()
{
	// Ŭ���̾�Ʈ���� �����û
	// �ٲ����� : Ŭ���̾�Ʈ ��ü�� �ǹ� ������ ���� �ʾҴµ��� ����ϴ´����̶� ������.

	OVERLAPPED_ACCEPT* accept_event = new OVERLAPPED_ACCEPT;
	//ZeroMemory(&_accept_event, sizeof(OVERLAPPED_ACCEPT));
	//_accept_event.type = IOCP_WORK::IOCP_ACCEPT;
	//InitAcceptSocket(_accept_event.socket);
	ZeroMemory(accept_event, sizeof(OVERLAPPED_ACCEPT));
	accept_event->type = IOCP_WORK::IOCP_ACCEPT;
	InitAcceptSocket(accept_event->socket);


	DWORD dummyDword = 0;
	g_AcceptEx(
		_listen_socket,						//��������(Listen �Լ��� �̹� ������ �����뵵�� ������ ����־�� ����������)
		accept_event->socket,				//InitAcceptSocket���� ������ ���� ����� Socket�� Ŭ���̾�Ʈ�� �����Ұ�
		accept_event->accept_buffer,		//����� Ŭ���̾�Ʈ�� �����ּҿ� �����ּ��� ������ �޾ƿ� ���� (���� Ŭ���̾�Ʈ�� �Ľ��ϰų� �Ҽ�����
		0,									//�ܼ������̹Ƿ� 0���� �ص� ���� Accept�� ���� ������ ����
		sizeof(SOCKADDR_IN) + 16,			//���� ���� �е� ����
		sizeof(SOCKADDR_IN) + 16,			//
		&dummyDword,						//���۵� ����Ʈ ����
		static_cast<LPOVERLAPPED>(accept_event));		//�񵿱� Ŀ���� ���� + Ȯ��������� �����͸� �Ѱܼ� �߰������� �Ľ��ؼ� ����Ҽ��ְ� �ٽ� �ޱ�����

}

void Listener::ProcessAccept(LPOVERLAPPED overlappedPtr)
{
	OVERLAPPED_ACCEPT* acceptData = reinterpret_cast<OVERLAPPED_ACCEPT*>(overlappedPtr);

	//�����̶� ���۷� ��������.

	//acceptData->accept_buffer; //->�ּ� �Ľ�
	std::shared_ptr<Client> p_client = ClientManager::Instance().CreateClient(acceptData->socket);
	if (p_client == nullptr)
	{
		//acceptData->socket; ���� �������.

		closesocket(acceptData->socket);
		std::cerr << "[������ ����á���ϴ�. close socket!! ]" << std::endl;
		RegisterAccept();
		return;
	}

	//�������ϰ� ����?
	//���ҹ����� ��α׸� ã�ƺ��� �� �ϴ��� ��Ȯ�ϰ� �˼���������
	//ê����Ƽ���� ����ô�.
	//�ּ�����, �����ּ�����, ���ϻ�������, �� os���� �ش� ������ � �������κ��� ����Ǿ������� �������� ���ٰ��Ѵ�.
	//���� ������ �⺻���ϵ���( getpeername(), getsockname(), shutdown()�� )�� ������ ������ �Ҽ��ִٰ� �Ѵ�.
	setsockopt(acceptData->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&_listen_socket, sizeof(_listen_socket));
	//Ŭ���̾�Ʈ ���̱� ���� (������ ��Ŷ ��ƽ�� �Ⱦ�!!)
	int flag = 1;
	setsockopt(acceptData->socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

	std::cerr << "[Ŭ�� " << p_client->GetClientID() << " ����]" << std::endl;

	//Ŭ���̾�Ʈ ������ IOCP�� ��� (�񵿱� Recv / Send�ϱ�����)
	_iocp->RegisterIOCP((HANDLE)p_client->GetSocket());
	p_client->RegisterRecv();


	RegisterAccept();
}

