#include "stdafx.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(int buffer_size)
	:_buffer_size(buffer_size), _write_size(0)
{
	_buffer = std::make_unique<BYTE[]>(_buffer_size);
}

bool SendBuffer::Write(BYTE* data, int size)
{
	if (size + _write_size > _buffer_size)
		return false;
	memcpy(_buffer.get() + _write_size, data, size);
	_write_size += size;
	return true;
}