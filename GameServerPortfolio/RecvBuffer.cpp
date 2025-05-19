#include "RecvBuffer.h"



RecvBuffer::RecvBuffer(int buffer_size)
{
	_buffer.resize(buffer_size, 0 );
}

void RecvBuffer::Write(int size)
{
	_write_pos += size;
}

void RecvBuffer::Read(int size)
{
	_read_pos += size;
	if (_read_pos == _write_pos)
	{
		//읽고나니 남은 데이터가없으면 복사비용 0으로 초기화한다.
		_read_pos = _write_pos = 0;
	}
	else if(0 < _read_pos)
	{
		int data_size = DataSize();
		::memmove(&_buffer[0], &_buffer[_read_pos], data_size);
		_read_pos = 0;
		_write_pos = data_size;
	}

}
