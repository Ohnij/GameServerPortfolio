#pragma once
using BYTE = uint8_t;

class RecvBuffer
{
public:
	RecvBuffer(int buffer_size = 4096);


	BYTE* GetWritePos() { return &_buffer[_write_pos]; }
	BYTE* GetReadPos() { return &_buffer[_read_pos]; }

	//데이터 받은크기
	int DataSize() const { return _write_pos - _read_pos; }
	//남은 공간
	int FreeSize() const { return _buffer.size() - _write_pos; }

	//쓰기 -> Register
	void Write(int size);
	//읽기 -> Process
	void Read(int size);

private:
	std::vector<BYTE> _buffer;
	int _read_pos = 0;
	int _write_pos = 0;
};

