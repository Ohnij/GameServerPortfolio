#pragma once
#include "NetDefine.h"

class RecvBuffer
{
public:
	RecvBuffer(const int iBufferSize = RECV_BUFFER_SIZE);
	~RecvBuffer();

	inline BYTE* GetWritePos() const { return &m_buffer[m_iWritePos]; }
	inline BYTE* GetReadPos() const { return &m_buffer[m_iReadPos]; }

	//데이터 받은크기
	inline int DataSize() const { return m_iWritePos - m_iReadPos; }
	//남은 공간
	inline int FreeSize() const { return m_iBufferSize - m_iWritePos; }

	//쓰기 -> Register
	void Write(int iSize);
	//읽기 -> Process
	void Read(int iSize);

	void Reset();
private:
	BYTE* m_buffer;
	int m_iBufferSize = 0;
	int m_iReadPos = 0;
	int m_iWritePos = 0;
};

