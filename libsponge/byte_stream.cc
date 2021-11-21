#include "byte_stream.hh"

#include <algorithm>
#include <iterator>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _buffer(capacity),_capacity(capacity) {}

size_t ByteStream::write(const string &data) {
    size_t len = data.length();
    if(len > _capacity - _length)
    {
        len = _capacity - _length;
    }
    
    for(size_t i=0; i<len; i++)
    {
        _buffer[_tail] = (data[i]);
        _tail = (_tail + 1) % _capacity;
    }
    _write_count += len;
    _length += len;
    return len;
    
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t length = len;
    if(length > _length)
    {
        length = _length;
    }
    string res(length, 0);
    // _head is unmodifiable
    size_t p = _head;
    for(size_t i=0; i<length; i++)
    {
        res[i] = _buffer[p];
        p = (p + 1) % _capacity;
    }
    
    return res;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) 
{ 
    if(len > _length)
    {
        _length = 0;
        _head = this->_tail;
        _read_count +=_length;
    }
    else
    {
        _length -= len;
        _head = (_head + len) % _capacity;
        _read_count += len;
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {

    string data = peek_output(len);

    pop_output(len);

    return data;
}

void ByteStream::end_input() 
{
    _input_ended_flag = true;
}

bool ByteStream::input_ended() const 
{ 
    return _input_ended_flag; 
}

size_t ByteStream::buffer_size() const 
{
     return _length; 
}

bool ByteStream::buffer_empty() const 
{
     return _length == 0; 
}

bool ByteStream::eof() const 
{ 
    return buffer_empty() && input_ended(); 
}

size_t ByteStream::bytes_written() const 
{
     return _write_count; 
}

size_t ByteStream::bytes_read() const 
{ 
    return _read_count; 
}

size_t ByteStream::remaining_capacity() const 
{ 
    return _capacity - _length; 
}