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
	//Wsa ���̺귯�� �ε�

	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		return ErrorPrint("WSAStartUp ����!! >> ");
	}


	//���� ��û�� Listen���� ����
	SOCKET _listenSocket;
	_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);


	//� �ּ��� �����Ǵ� �ش� ��Ʈ�� ���°� �ްڴٴ� �ǹ�
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(ADDR_ANY);

	//���� ��û�� ���Ͽ� ����
	if (0 != bind(_listenSocket, (sockaddr*)&addr, sizeof(SOCKADDR_IN)))
	{
		return ErrorPrint("Bind ����!! >> ");
	}

	if (0 != listen(_listenSocket, SOMAXCONN))
	{
		return ErrorPrint("listen ����!! >> ");
	}

	
	//SOCKADDR clientAddr;
	//SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	//int addrLen = sizeof(SOCKADDR);
	//char clientBuffer[1024];
	//�񵿱�� �����ϱ�.
	//AccpetEX�� Accept�� �ٸ��� ����� ���� �ʰ� IOCP�� GQCP�� ���� ����� �뺸 �޴´�.
	//AcceptEx �Լ��� ����Ϸ��� MSWSock ������� �ʿ�
	//SOCKET client = AceeptEx(_listenSocket, &clientAddr, &addrLen);
	/*
	* //��ó https://learn.microsoft.com/ko-kr/windows/win32/api/mswsock/nf-mswsock-acceptex
	* 
	BOOL AcceptEx(
		  [in]  SOCKET       sListenSocket,			//listen����
		  [in]  SOCKET       sAcceptSocket,			//�޾Ƽ��� ���� (Ŭ���̾�Ʈ)
		  [in]  PVOID        lpOutputBuffer,		//����� ��� �ּ� + ���� ������ ���� ����
		  [in]  DWORD        dwReceiveDataLength,	//���� �� ó�� ������ ������ ���� (0�� ����)
		  [in]  DWORD        dwLocalAddressLength,	//�ּ� ���� (>= sizeof(SOCKADDR_IN) + 16) (�����ּ�)
		  [in]  DWORD        dwRemoteAddressLength,	//�ּ� ���� (>= sizeof(SOCKADDR_IN) + 16) (�����ּ�)
		  [out] LPDWORD      lpdwBytesReceived,		//���� ���ŵ� ����Ʈ �� (�񵿱�� ����)
		  [in]  LPOVERLAPPED lpOverlapped			//IOCP �Ϸ� ������ ���� OVERLAPPED ������
		);
	

	�Լ������͸� ����ؾ��ϴ� ����
	https://learn.microsoft.com/en-us/archive/msdn-magazine/2000/october/windows-sockets-2-0-write-scalable-winsock-apps-using-completion-ports
	 �Լ� �����͸� �̸� ���� �ʰ� �Լ��� ȣ���ϴ� ��(��, mswsock.lib�� �����ϰ� AcceptEx�� ���� ȣ���ϴ� ��)��
	 AcceptEx�� Winsock2�� ������ ��Ű��ó �ܺο� �ֱ� ������ ����� ���� ��ϴ�. 
	 AcceptEx�� ���ø����̼��� ������ mswsock ���� ����ȭ�� �����ڿ��� AcceptEx�� ȣ���Ϸ��� �ϴ� ���( �׸� 3 ���� )
	 ��� ȣ�⿡ ���� WSAIoctl�� ����Ͽ� �Լ� �����͸� ��û�ؾ� �մϴ�. 
	 �� ȣ�⿡�� �̷��� ����� ���� ���ϸ� �����Ϸ��� �̷��� API�� ����Ϸ��� ���ø����̼��� WSAIoctl�� ȣ���Ͽ� 
	 ����ȭ�� �����ڿ��� ���� �̷��� �Լ��� ���� �����͸� ���� �մϴ�.

	*/

	//IOCP��ü ����
	HANDLE _iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	
	//IOCP�� Listen���� �񵿱�� �ް� ���
	CreateIoCompletionPort((HANDLE)_listenSocket, _iocpHandle, 0, 0);


	//�Լ������͸� �޾ƿ��� (����)

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
		_listenSocket,	//��������(Listen �Լ��� �̹� ������ �����뵵�� ������ ����־�� ����������)
		_clients.back()->_socket,	//�޾Ƽ� ������� ���������� ���� ���õ� �ű� ���� Overlapped ���� �ʼ�
		&dummyBuffer,		//����� Ŭ���̾�Ʈ�� �����ּҿ� �����ּ��� ������ �޾ƿ� ����
		0,				//�ܼ������̹Ƿ� 0���� �ص� ���� Accept�� ���� ������ ����
		ADDRESS_LEN,	//���� ���� �е� ����
		ADDRESS_LEN,	//
		&dummyDword,	//���۵� ����Ʈ ����
		overlapped);		//�񵿱� Ŀ���� ���� + Ȯ��������� �����͸� �Ѱܼ� �߰������� �Ľ��ؼ� ����Ҽ��ְ� �ٽ� �ޱ�����


	std::thread workerThread([&] {

		/*
		BOOL GetQueuedCompletionStatus(
		  [in]  HANDLE       CompletionPort,				//�Ϸ� ��Ʈ�� ���� �ڵ��Դϴ�.
				LPDWORD      lpNumberOfBytesTransferred,	//�Ϸ�� I/O �۾����� ���۵� ����Ʈ ���� �޴� ������ ���� �������Դϴ�.
		  [out] PULONG_PTR   lpCompletionKey,				//I/O �۾��� �Ϸ�� ���� �ڵ�� ����� �Ϸ� Ű ���� �޴� ������ ���� �������Դϴ�.
		  [out] LPOVERLAPPED *lpOverlapped,
		  [in]  DWORD        dwMilliseconds					//�Ϸ� ��Ŷ�� �Ϸ� ��Ʈ�� ��Ÿ�� ������ ȣ���ڰ� ����Ϸ��� �ð�(�и���)�Դϴ�.
		);
		*/
		DWORD transferredByte = 0;
		ULONG_PTR key = 0;
		OVERLAPPED_EX* overlappedData = nullptr;

		while (true)
		{
			//���⼭ Ŭ���̾�Ʈ ��ü�� �������־���ҵ�.
			BOOL res = GetQueuedCompletionStatus(_iocpHandle, &transferredByte, &key, reinterpret_cast<LPOVERLAPPED*>(&overlappedData), 1000);
			if (res)
			{
				int clientId = overlappedData->_clientNumber;
				switch (overlappedData->_type)
				{
					case IOCP_WORK::IOCP_ACCEPT:
					{
						std::shared_ptr<Client> pClient = _clients[clientId];
						//Ŭ���̾�Ʈ ������ IOCP�� ��� (RecvEX /  SendEX�ϱ�����)
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

						//���߿� �Լ��� ���� �ؾ��ҵ�..
						g_AcceptEx(
							_listenSocket,	//��������(Listen �Լ��� �̹� ������ �����뵵�� ������ ����־�� ����������)
							_clients.back()->_socket,	//�޾Ƽ� ������� ���������� ���� ���õ� �ű� ���� Overlapped ���� �ʼ�
							&dummyBuffer,		//����� Ŭ���̾�Ʈ�� �����ּҿ� �����ּ��� ������ �޾ƿ� ����
							0,				//�ܼ������̹Ƿ� 0���� �ص� ���� Accept�� ���� ������ ����
							ADDRESS_LEN,	//���� ���� �е� ����
							ADDRESS_LEN,	//
							&dummyDword,	//���۵� ����Ʈ ����
							overlapped);		//�񵿱� Ŀ���� ���� + Ȯ��������� �����͸� �Ѱܼ� �߰������� �Ľ��ؼ� ����Ҽ��ְ� �ٽ� �ޱ�����

					}
					break;
					case IOCP_WORK::IOCP_RECV:
					{
						std::shared_ptr<Client> pClient = _clients[clientId];

						char recivedata[1024];
						ZeroMemory(recivedata, 1024);
						memcpy( recivedata, pClient->_reciveBuffer, size_t(transferredByte));
						
						std::cerr << "[Ŭ�� " << clientId << "]���� ������ : " << recivedata << "(" << transferredByte << ")" << std::endl;


						//WSASend(); //�����ؾ��ҵ� �ʹ��ð��̴���..
						

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

	//���̺귯�� ��ȯ
	WSACleanup();
	return 0;
}


