#pragma once
#include "IOCPDefine.h"
#include "mutex"

class SendBuffer
{
public:
	SendBuffer(int iBufferSize = SEND_BUFFER_SIZE);
	~SendBuffer();

	//������ 
	//BYTE* GetBuffer() { return m_buffer; }
	//int GetWriteSize() { return _write_size; }
	int GetDataSize();

	int GetFreeSize();
	WSABUF GetWSABUF(); //������ �ѹ��� ������ �������� ĳ�� ���ʿ�

	//���� -> Register
	bool Write(const BYTE* pData, uint32_t iSize);
	//�б� ->�Ϸ�
	bool Read(uint32_t iSize);

	void Reset();
private:
	std::mutex m_mutex;
	BYTE* m_buffer;
	int m_iBufferSize = 0;

	uint32_t m_iWritePos = 0;
	uint32_t m_iReadPos = 0;
};



