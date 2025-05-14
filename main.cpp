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

	
	SOCKADDR clientAddr;
	SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	int addrLen = sizeof(SOCKADDR);
	char clientBuffer[1024];
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
	*/
	

	const size_t ADDRESS_LEN = sizeof(SOCKADDR_IN) + 16;
	DWORD transferredByte = 0;
	OVERLAPPED overlapped;
	ZeroMemory(&overlapped, sizeof(OVERLAPPED));

	//함수포인터를 받아오자 (권장)


	AcceptEx(
		_listenSocket,
		clientSocket,
		&clientBuffer,
		0,				 // Accept시 수신 데이터 없음
		ADDRESS_LEN,
		ADDRESS_LEN,
		&transferredByte,
		&overlapped);



	//라이브러리 반환
	WSACleanup();
	return 0;
}


