#include "pch.h"
#include "RecvBuffer.h"


RecvBuffer::RecvBuffer(const int iBufferSize)
{
	m_iBufferSize = iBufferSize;
	m_buffer = new BYTE[iBufferSize];
}

RecvBuffer::~RecvBuffer()
{
	delete[] m_buffer;
}

void RecvBuffer::Write(int iSize)
{
	m_iWritePos += iSize;
}

void RecvBuffer::Read (int iSize)
{
	m_iReadPos += iSize;
	if (m_iReadPos == m_iWritePos)
	{
		//읽고나니 남은 데이터가없으면 복사비용 0으로 초기화한다.
		m_iReadPos = m_iWritePos = 0;
	}
	else if (0 < m_iReadPos)
	{
		int iRemain = DataSize();
		::memmove(&m_buffer[0], &m_buffer[m_iReadPos], iRemain);
		m_iReadPos = 0;
		m_iWritePos = iRemain;
	}

}

void RecvBuffer::Reset()
{
	m_iReadPos = 0;
	m_iWritePos = 0;
}
