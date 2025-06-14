#pragma once
#include "NetDefine.h"

class RecvBuffer
{
public:
	RecvBuffer(const int iBufferSize = RECV_BUFFER_SIZE);
	~RecvBuffer();

	inline BYTE* GetWritePos() const { return &m_buffer[m_iWritePos]; }
	inline BYTE* GetReadPos() const { return &m_buffer[m_iReadPos]; }

	//������ ����ũ��
	inline int DataSize() const { return m_iWritePos - m_iReadPos; }
	//���� ����
	inline int FreeSize() const { return m_iBufferSize - m_iWritePos; }

	//���� -> Register
	void Write(int iSize);
	//�б� -> Process
	void Read(int iSize);

	void Reset();
private:
	BYTE* m_buffer;
	int m_iBufferSize = 0;
	int m_iReadPos = 0;
	int m_iWritePos = 0;
};

