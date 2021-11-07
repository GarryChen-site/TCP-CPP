#include "wrapping_integers.hh"


// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {

    return isn + (static_cast<uint32_t>(n)) ;
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint32_t offset = n - isn;
    // checkpoint is the last sequence number that was received, so we need to move last 32 bits of the sequence number
    // add the offset to the last 32 bits of the sequence number
    uint64_t t = (checkpoint & 0xFFFFFFFF00000000) + offset;
    uint64_t bigger = t + (1ul << 32) - checkpoint;
    uint64_t smaller = t - (1ul << 32) - checkpoint;
    uint64_t normal = t-checkpoint;
    if(abs(int64_t(bigger)) < abs(int64_t(normal)))
        t += 1ul<<32;
    // make sure smaller not overflow
    if(t >= (1ul<<32) && abs(int64_t(smaller)) < abs(int64_t(normal)))
        t -= 1ul<<32;
    return t;
}
