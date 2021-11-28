#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const {
    //  return _bytes_in_flight;
    return _next_seqno - _expect_ack;
}

void TCPSender::fill_window() {

    // -------------------- new -------------------- //

    if (!_syn_flag) {
        TCPSegment seg;
        seg.header().syn = true;
        seg.header().seqno = wrap(0, _isn);
        _segments_out.push(seg);
        _segments_outstanding.push(seg);
        _next_seqno = 1;
        _retransmission_timeout = _timer + _initial_retransmission_timeout;
        _syn_flag = true;
        return;
    }

    size_t remain = _window_size - bytes_in_flight();
    bool send = false;
    if (_expect_ack != 0) {
        while (remain > 0 && _stream.buffer_size() > 0) {
            // send segment
            size_t send_size = min(TCPConfig::MAX_PAYLOAD_SIZE, remain);
            TCPSegment seg;
            string data = _stream.read(send_size);
            seg.payload() = Buffer(std::move(data));
            seg.header().seqno = wrap(_next_seqno, _isn);
            _next_seqno += seg.length_in_sequence_space();
            remain = _window_size - bytes_in_flight();
            if (_stream.eof() && remain > 0 && !_fin_flag) {
                seg.header().fin = true;
                _next_seqno += 1;
                _fin_flag = true;
            }
            _segments_out.push(seg);
            _segments_outstanding.push(seg);
            send = true;
        }
    }

    if (_stream.eof() && remain > 0 && !_fin_flag) {
        TCPSegment seg;
        seg.header().fin = true;
        seg.header().seqno = wrap(_next_seqno, _isn);
        _segments_out.push(seg);
        _segments_outstanding.push(seg);
        _next_seqno++;
        _fin_flag = true;
        send = true;
    }

    if (send && _retransmission_timeout == 0) {
        // open timer
        _retransmission_timeout = _timer + _initial_retransmission_timeout;
        _consecutive_retransmission = 0;
        _rto_back_off = 0;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    // --------- new code ----------

    _window_size = window_size;
    _do_back_off = 1;
    if (_window_size == 0) {
        _window_size = 1;
        _do_back_off = 0;
    }

    uint64_t ack = unwrap(ackno, _isn, _expect_ack);

    // out of window
    if (ack > _next_seqno) {
        return;
    }

    if (ack <= _next_seqno && ack > _expect_ack) {

        _expect_ack = ack;
        if (bytes_in_flight() == 0) {
            // close timer
            // _timer_running = false;
            _retransmission_timeout = 0;
            _consecutive_retransmission = 0;
            _rto_back_off = 0;
        }
        // event: ACK received,with ACK field value of y
        // if(there are currently any not-yet-acknowledged segments)
        // start timer
        else{
            // reopen timer
            _retransmission_timeout = _initial_retransmission_timeout + _timer;
            _consecutive_retransmission = 0;
            _rto_back_off = 0;
        }
    }
    // remove all segments that have been acknowledged
    while (!_segments_outstanding.empty()) {
        TCPSegment seg = _segments_outstanding.front();
        size_t seg_abs_ackno = unwrap(seg.header().seqno, _isn, _next_seqno);
        // the ackno is greater than all of the sequence numbers in the segment
        if ((seg_abs_ackno + seg.length_in_sequence_space()) <= _expect_ack) {
            _segments_outstanding.pop();
        } else {
            break;
        }
    }

}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    // ---------- new code ----------

    _timer += ms_since_last_tick;

    if (_timer >= _retransmission_timeout && !_segments_outstanding.empty()) {
        _segments_out.push(_segments_outstanding.front());
        _consecutive_retransmission++;
        _rto_back_off += _do_back_off;
        _retransmission_timeout = _timer + (_initial_retransmission_timeout << _rto_back_off);
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmission; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    // set the sequence number for an empty segment correctly
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}

