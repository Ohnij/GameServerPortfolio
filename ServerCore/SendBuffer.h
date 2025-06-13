#pragma once
#include "IOCPDefine.h"
#include "mutex"

class SendBuffer
{
public:
	SendBuffer(int iBufferSize = SEND_BUFFER_SIZE);
	~SendBuffer();

	//데이터 
	//BYTE* GetBuffer() { return m_buffer; }
	//int GetWriteSize() { return _write_size; }
	int GetDataSize();

	int GetFreeSize();
	WSABUF GetWSABUF(); //보낼때 한번에 보내기 보낼정보 캐싱 락필요

	//쓰기 -> Register
	bool Write(const BYTE* pData, uint32_t iSize);
	//읽기 ->완료
	bool Read(uint32_t iSize);

	void Reset();
private:
	std::mutex m_mutex;
	BYTE* m_buffer;
	int m_iBufferSize = 0;

	uint32_t m_iWritePos = 0;
	uint32_t m_iReadPos = 0;
};



