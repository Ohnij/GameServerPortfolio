#pragma once
#include "IOCPDefine.h"
#include <MSWSock.h>


//Listener
// 
// accept�� socket ����
// accept�� Ŭ���̾�Ʈ iocp�Ϳ��� 
// Client -> Ŭ���̾�Ʈ ���ϰ��� 
// accept�� OverlappedȮ�� ����ü ����

class Session;
class Listener
{
public:
	Listener() = default;
	~Listener() = default;


	bool Init(USHORT port);						//������ �ʱ⼳��
	SOCKET GetSocket() { return m_Socket; }

	void PrepareAcceptSocket(SOCKET& socket);	//Accept�ɱ��� ��Ĺ�����ϱ� (������ �� ��Ĺ)


	void Register();							//���� 1ȸ ~���� �ڵ�����
	Session* Process(OVERLAPPED_ACCEPT* acceptData); //accept������ ���� ���ǹ�ȯ�ϱ�
private:

	//���� ��û�� Listen���� ����
	SOCKET m_Socket;
};
