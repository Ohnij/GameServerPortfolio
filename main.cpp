#pragma comment (lib, "ws2_32.lib")
#include <WinSock2.h>
#include <MSWSock.h>

#include <iostream>

int ErrorPrint(const char* ErrorDiscription)
{
	std::cerr << ErrorDiscription << ::WSAGetLastError() << std::endl;
	return -1;
}

#define PORT 7777

LPFN_ACCEPTEX g_AcceptEx = nullptr;
LPFN_CONNECTEX g_ConnectEx = nullptr;
LPFN_DISCONNECTEX g_DisconnectEx = nullptr;

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

	
	SOCKADDR clientAddr;
	SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	int addrLen = sizeof(SOCKADDR);
	char clientBuffer[1024];
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
	*/
	

	const size_t ADDRESS_LEN = sizeof(SOCKADDR_IN) + 16;
	DWORD transferredByte = 0;
	OVERLAPPED overlapped;
	ZeroMemory(&overlapped, sizeof(OVERLAPPED));

	//�Լ������͸� �޾ƿ��� (����)


	AcceptEx(
		_listenSocket,
		clientSocket,
		&clientBuffer,
		0,				 // Accept�� ���� ������ ����
		ADDRESS_LEN,
		ADDRESS_LEN,
		&transferredByte,
		&overlapped);



	//���̺귯�� ��ȯ
	WSACleanup();
	return 0;
}


