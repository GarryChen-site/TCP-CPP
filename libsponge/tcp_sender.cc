#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _retransmission_timeout(retx_timeout){}

uint64_t TCPSender::bytes_in_flight() const { return {}; }

void TCPSender::fill_window() 
{
    size_t win = _window_size > 0 ? _window_size : 1;
    // already sending
    size_t already_send = _next_seqno - _abs_recv_ackno;
    size_t remain_win_size = win - already_send;
    
    if(remain_win_size >= 0)
    {
        // send as much as possible
        size_t send_size = min(TCPConfig::MAX_PAYLOAD_SIZE, remain_win_size);
        TCPSegment seg;
        string data = _stream.read(send_size);
        seg.payload() = Buffer(std::move(data));

        
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) 
{ 
    size_t abs_ackno = unwrap(ackno,_isn,_abs_recv_ackno);

    // already acked
    if(abs_ackno < _abs_recv_ackno)
    {
        return;
    }

    _abs_recv_ackno = abs_ackno;

    _retransmission_timeout = _initial_retransmission_timeout;

    _consecutive_retransmission = 0;

    // if the sender has any outstanding data
    if(!_segments_outstanding.empty())
    {
        _timer = 0;
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) 
{ 
  _timer += ms_since_last_tick;
  if(_timer >=_retransmission_timeout)    
  {
      _segments_out.push(_segments_outstanding.front());
      _consecutive_retransmission++;
      _retransmission_timeout *= 2;
      _timer = 0;
  }
}

unsigned int TCPSender::consecutive_retransmissions() const 
{ 
    return _consecutive_retransmission; 
}

void TCPSender::send_empty_segment() 
{
    TCPSegment seg;
    // set the sequence number for an empty segment correctly
    seg.header().seqno = wrap(_next_seqno,_isn);
    _segments_out.push(seg);
}
