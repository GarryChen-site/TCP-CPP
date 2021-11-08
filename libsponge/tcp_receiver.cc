#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    
    if(seg.header().syn)
    {
        _isn_set = true;
        _ackno = seg.header().seqno.raw_value()+1;
        
    }

    if(seg.header().fin)
    {
        _ackno += 1;
        _reassembler.stream_out().end_input();
    }

    
}

optional<WrappingInt32> TCPReceiver::ackno() const 
{ 
    if(_isn_set)
    {
        return WrappingInt32(_ackno);
    }
    else
    {
        return nullopt;
    }

}

size_t TCPReceiver::window_size() const 
{
     return _capacity - _reassembler.stream_out().buffer_size();
}
