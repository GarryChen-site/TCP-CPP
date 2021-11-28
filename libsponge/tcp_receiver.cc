#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {

    if(!_isn.has_value())
    {
        // init _isn
        if(seg.header().syn)
        {
            _isn = seg.header().seqno + 1;
        }
        else
        {
            return;
        }
    }

    bool eof = seg.header().fin;

    std::string data = seg.payload().copy();

    size_t index = unwrap(seg.header().seqno + (seg.header().syn ? 1 : 0),
                          _isn.value(), _reassembler.head_index());
    
    _reassembler.push_substring(data, index, eof);    
    
}

optional<WrappingInt32> TCPReceiver::ackno() const 
{ 

    if(_isn.has_value())
    {
        return wrap(_reassembler.head_index(), _isn.value()) + (_reassembler.input_ended() ? 1 : 0);
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
