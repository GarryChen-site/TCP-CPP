#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const 
{ 
    return _sender.stream_in().remaining_capacity(); 
}

size_t TCPConnection::bytes_in_flight() const 
{ 
    return _sender.bytes_in_flight(); 
}

size_t TCPConnection::unassembled_bytes() const 
{ 
    return _receiver.unassembled_bytes(); 
}

size_t TCPConnection::time_since_last_segment_received() const 
{ 
    return _time_since_last_segment_received; 
}

void TCPConnection::segment_received(const TCPSegment &seg) 
{ 
    if(!_active)
    {
        return;
    }
    if(seg.header().syn)
    {
        _syn_flag = true;
    }

    _time_since_last_segment_received = 0;

    // bool send_empty = false;

    if(seg.header().rst)
    {
        unclean_shutdown(false);
        return;
    }

    _receiver.segment_received(seg);

    // when in closed(next_seqno_absolute == 0),ack is not allowed
    if(seg.header().ack && _sender.next_seqno_absolute() > 0)
    {
        _sender.ack_received(seg.header().ackno,seg.header().win);
        // how to send empty segment?
        // if(seg.length_in_sequence_space() == 1 && seg.header().syn)
        // {
        //     send_empty = true;
        // }
        
    }
    if(_syn_flag)
    {
        _sender.fill_window();
    }

    // if the incoming segment occupied any sequence numbers
    // if(seg.length_in_sequence_space() > 0)
    // {
    //     send_empty = true;
    // }

    // if(send_empty)
    // {
    //     if(_receiver.ackno().has_value() && _sender.segments_out().empty())
    //     {
    //         _sender.send_empty_segment();
    //     }
    // }

    if(_sender.segments_out().empty())
    {
        if(seg.length_in_sequence_space() == 0)
        {

        }
        else
        {
            _sender.send_empty_segment();
        }
    }

    send_segments();

}

bool TCPConnection::active() const 
{ 
    return _active; 
}

size_t TCPConnection::write(const string &data) {
    size_t ret = _sender.stream_in().write(data);
    _sender.fill_window();
    send_segments();
    return ret;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) 
{ 
    if(!_active)
    {
        return;
    }
    _time_since_last_segment_received += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    if(_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS)
    {
        unclean_shutdown(true);
    }
    send_segments();

}

void TCPConnection::end_input_stream() 
{
    _sender.stream_in().end_input();
}

void TCPConnection::connect() 
{
    _sender.fill_window();
    send_segments();
}


TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            unclean_shutdown(true);
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::send_segments()
{
    // fill_window decide to send syn
    // _sender.fill_window();

    while(!_sender.segments_out().empty())
    {
        TCPSegment seg = _sender.segments_out().front();
        _sender.segments_out().pop();
        if(_receiver.ackno().has_value())
        {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
            seg.header().win = _receiver.window_size();
        }
        if(_need_send_rst)
        {
            _need_send_rst = false;
            seg.header().rst = true;
        }

        _segments_out.push(seg);
        
    }

}

void TCPConnection::unclean_shutdown(bool send_rst)
{
    _receiver.stream_out().set_error();
    _sender.stream_in().set_error();
    _active = false;
    if(send_rst)
    {
        _need_send_rst = true;
        send_segments();
    }
}

void TCPConnection::clean_shutdown()
{
    if(_receiver.stream_out().input_ended() && !(_sender.stream_in().eof()))
    {
        _linger_after_streams_finish = false;
    }

    if(_sender.stream_in().eof() && _sender.bytes_in_flight() == 0 && _receiver.stream_out().input_ended())
    {
        if(!_linger_after_streams_finish || _time_since_last_segment_received >= 10* _cfg.rt_timeout)
        {
            _active = false;
        }
    }
}




