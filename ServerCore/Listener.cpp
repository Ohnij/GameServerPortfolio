#include "pch.h"
#include "Listener.h"
#include "IocpServer.h"
#include "NetUtil.h"
#include "SessionManager.h"
#include "Session.h"

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



bool Listener::Init(USHORT port)
{
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	//���� ����Ǿ��µ� Listien ��Ʈ��ȣ�� ���� ������ ���ᰡ ���� �������·� �������� ��� 
	//bind�Լ��� !=0 �� ���� ���������� return false ���ͼ� ������� ���Ҽ�����
	BOOL opt = TRUE;
	setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

	//� �ּ��� �����Ǵ� �ش� ��Ʈ�� ���°� �ްڴٴ� �ǹ�
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ADDR_ANY);


	//���� ��û�� ���Ͽ� ����
	if (0 != bind(m_Socket, (sockaddr*)&addr, sizeof(SOCKADDR_IN)))
	{
		WSAError("Listener..Failed\nBind ����!! >> ", ::WSAGetLastError());
		return false;
	}

	if (0 != listen(m_Socket, SOMAXCONN))
	{
		WSAError("Listener..Failed\nlisten�Լ� ����!! >> ", ::WSAGetLastError());
		return false;
	}

	//�Լ������͸� �޾ƿ��� (����)

	DWORD bytes = 0;
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidConnectEx = WSAID_CONNECTEX;
	GUID guidDisconnectEx = WSAID_DISCONNECTEX;
	WSAIoctl(m_Socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx, sizeof(guidAcceptEx),
		&g_AcceptEx, sizeof(g_AcceptEx),
		&bytes, NULL, NULL);
	WSAIoctl(m_Socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidConnectEx, sizeof(guidConnectEx),
		&g_ConnectEx, sizeof(g_ConnectEx),
		&bytes, NULL, NULL);
	WSAIoctl(m_Socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidDisconnectEx, sizeof(guidDisconnectEx),
		&g_DisconnectEx, sizeof(g_DisconnectEx),
		&bytes, NULL, NULL);

	return true;
}

void Listener::PrepareAcceptSocket(SOCKET& socket)
{
	socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
}


void Listener::Register()
{
	// Ŭ���̾�Ʈ���� �����û
	// �ٲ����� : Ŭ���̾�Ʈ ��ü�� �ǹ� ������ ���� �ʾҴµ��� ����ϴ´����̶� ������.

	OVERLAPPED_ACCEPT* accept_event = new OVERLAPPED_ACCEPT;
	ZeroMemory(accept_event, sizeof(OVERLAPPED_ACCEPT));
	accept_event->type = IOCP_WORK::IOCP_ACCEPT;
	PrepareAcceptSocket(accept_event->socket);

	DWORD dummyDword = 0;
	g_AcceptEx(
		m_Socket,						//��������(Listen �Լ��� �̹� ������ �����뵵�� ������ ����־�� ����������)
		accept_event->socket,				//InitAcceptSocket���� ������ ���� ����� Socket�� Ŭ���̾�Ʈ�� �����Ұ�
		accept_event->accept_buffer,		//����� Ŭ���̾�Ʈ�� �����ּҿ� �����ּ��� ������ �޾ƿ� ���� (���� Ŭ���̾�Ʈ�� �Ľ��ϰų� �Ҽ�����
		0,									//�ܼ������̹Ƿ� 0���� �ص� ���� Accept�� ���� ������ ����
		sizeof(SOCKADDR_IN) + 16,			//���� ���� �е� ����
		sizeof(SOCKADDR_IN) + 16,			//
		&dummyDword,						//���۵� ����Ʈ ����
		static_cast<LPOVERLAPPED>(accept_event));		//�񵿱� Ŀ���� ���� + Ȯ��������� �����͸� �Ѱܼ� �߰������� �Ľ��ؼ� ����Ҽ��ְ� �ٽ� �ޱ�����

}

Session* Listener::Process(OVERLAPPED_ACCEPT* acceptData)
{
	//OVERLAPPED_ACCEPT* acceptData = reinterpret_cast<OVERLAPPED_ACCEPT*>(overlappedPtr);

	//�����̶� ���۷� ��������.
	//acceptData->accept_buffer; //->�ּ� �Ľ�

	
	Session* pSession = SESSION_MANAGER.GetEmpty();
	if (pSession == nullptr)
	{
		//acceptData->socket; ���� �������.
		closesocket(acceptData->socket);
		std::cerr << "[������ ����á���ϴ�. close socket!! ]" << std::endl;
		
		Register(); //�ٽ� Accept���
		return pSession;
	}

	//�������ϰ� ����?
	//���ҹ����� ��α׸� ã�ƺ��� �� �ϴ��� ��Ȯ�ϰ� �˼���������
	//ê����Ƽ���� ����ô�.
	//�ּ�����, �����ּ�����, ���ϻ�������, �� os���� �ش� ������ � �������κ��� ����Ǿ������� �������� ���ٰ��Ѵ�.
	//���� ������ �⺻���ϵ���( getpeername(), getsockname(), shutdown()�� )�� ������ ������ �Ҽ��ִٰ� �Ѵ�.
	setsockopt(acceptData->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_Socket, sizeof(m_Socket));
	//Ŭ���̾�Ʈ ���̱� ���� (������ ��Ŷ ��ƽ�� �Ⱦ�!!)
	int flag = 1;
	setsockopt(acceptData->socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));


	//�ٷ� ����� �ְ� ����
	LINGER linger;
	linger.l_onoff = 1;	//���ſɼǻ��
	linger.l_linger = 0; //���ۺ�������� ���close 
	setsockopt(acceptData->socket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

	std::cerr << "[Ŭ�� " << pSession->GetSessionID() << " ����]" << std::endl;


	pSession->PrepareSesseion(acceptData->socket); //���ϼ��� �� �����ʱ�ȭ
	
	delete acceptData;
	Register(); //�ٽ� Accept���
	return pSession;
}





