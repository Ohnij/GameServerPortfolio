using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DummyClient.Network
{
    internal class RecvBuffer
    {
        private byte[] _buffer;
        private int _read_pos;
        private int _write_pos;

        public RecvBuffer(int buffer_size = 8192)
        {
            _buffer = new byte[buffer_size];
            _read_pos = _write_pos = 0;
        }

        public int DataSize =>  _write_pos - _read_pos;
	    //남은 공간
	    public int FreeSize => _buffer.Length - _write_pos;


        //데이터 받은크기
        //BYTE* GetWritePos() { return &_buffer[_write_pos]; }
        public Span<byte> GetWriteBuffer() => new Span<byte>(_buffer, _write_pos, FreeSize);

        //BYTE* GetReadPos() { return &_buffer[_read_pos]; }
        public Span<byte> GetReadBuffer() => new Span<byte>(_buffer, _read_pos, DataSize);
        

        //쓰기 -> Register
        public void Write(int size)
        {
            _write_pos += size;
        }
        //읽기 -> Process
        public void Read(int size)
        {
            _read_pos += size;
            if (_read_pos == _write_pos)
            {
                //읽고나니 남은 데이터가없으면 복사비용 0으로 초기화한다.
                _read_pos = _write_pos = 0;
            }
            else if (0 < _read_pos)
            {
                int data_size = DataSize;
                //::memmove(&_buffer[0], &_buffer[_read_pos], data_size);
                Array.Copy(_buffer, _read_pos, _buffer, 0, DataSize);
                _read_pos = 0;
                _write_pos = data_size;
            }
        }
    }
}
