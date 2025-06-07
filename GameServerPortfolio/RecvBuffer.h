#pragma once
using BYTE = uint8_t;

class RecvBuffer
{
public:
	RecvBuffer(int buffer_size = 4096);


	BYTE* GetWritePos() { return &_buffer[_write_pos]; }
	BYTE* GetReadPos() { return &_buffer[_read_pos]; }

	//������ ����ũ��
	int DataSize() const { return _write_pos - _read_pos; }
	//���� ����
	int FreeSize() const { return _buffer.size() - _write_pos; }

	//���� -> Register
	void Write(int size);
	//�б� -> Process
	void Read(int size);

private:
	std::vector<BYTE> _buffer;
	int _read_pos = 0;
	int _write_pos = 0;
};

