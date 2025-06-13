#include "pch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(int iBufferSize)
    : m_buffer(new BYTE[iBufferSize]), m_iBufferSize(iBufferSize)
{

}

SendBuffer::~SendBuffer()
{
    delete[] m_buffer;
}

int SendBuffer::GetDataSize()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_iWritePos == m_iReadPos)
        return 0;
    if (m_iReadPos < m_iWritePos)
        return (m_iWritePos - m_iReadPos);
    
    return m_iBufferSize- (m_iReadPos - m_iWritePos);
}

int SendBuffer::GetFreeSize()
{
    //1�� �Ȼ��� rw�� ������ ������0 �ΰŶ� �˼�������

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_iReadPos <= m_iWritePos)
        return m_iBufferSize - (m_iWritePos - m_iReadPos) - 1;
    else
        return m_iReadPos - m_iWritePos - 1;
}

WSABUF SendBuffer::GetWSABUF()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    //������ �ѹ��� �ټ��� ������ ���ϰ�� ���� ������ ������ �ٽú����½�����
    WSABUF wsabuf;
    if (m_iReadPos <= m_iWritePos)
    {
        wsabuf.buf = reinterpret_cast<CHAR*>(m_buffer+m_iReadPos);
        wsabuf.len = m_iWritePos - m_iReadPos;
    }
    else
    {
        wsabuf.buf = reinterpret_cast<CHAR*>(m_buffer + m_iReadPos);
        wsabuf.len = m_iBufferSize - m_iReadPos;
    }

    return wsabuf;
}

bool SendBuffer::Write(const BYTE* pData, uint32_t iSize)
{
    if (GetFreeSize() < iSize)
    {
        //log
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    int endsize = m_iBufferSize - m_iWritePos;
    if (iSize < endsize)
    {
        memcpy(m_buffer + m_iWritePos, pData, iSize);
    }
    else
    {
        memcpy(m_buffer + m_iWritePos, pData, endsize);
        memcpy(m_buffer, pData + endsize, iSize - endsize);
    }
    m_iWritePos = (m_iWritePos + iSize) % m_iBufferSize;

    return true;
}

bool SendBuffer::Read(uint32_t iSize)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_iReadPos = (m_iReadPos + iSize) % m_iBufferSize;

    return true;
}



void SendBuffer::Reset()
{
    m_iWritePos = 0;
    m_iReadPos = 0;
}
