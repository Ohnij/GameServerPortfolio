#pragma once
#include <vector>
#include <memory>
using BYTE = uint8_t;

class SendBuffer : public std::enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(int buffer_size = 1024);


	//데이터 
	BYTE* GetBuffer() { return _buffer.get(); } //raw pointer
	int GetWriteSize() { return _write_size; }
	int GetBufferSize() { return _buffer_size; }
	int GetFreeSize() { return _buffer_size - _write_size; }

	//쓰기 -> Register
	bool Write(BYTE* data, int size);
	void ResetBuffer() { _write_size = 0; }

private:
	std::unique_ptr<BYTE[]> _buffer;
	int _write_size = 0;
	int _buffer_size = 0;
};



