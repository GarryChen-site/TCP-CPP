#include "stream_reassembler.hh"

#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {
}

//! merge elm2 to elm1, return merged bytes
// elm2 is already in Set
long StreamReassembler::merge_block(block_node &elm1, const block_node &elm2) {
    block_node x, y;
    if (elm1.begin > elm2.begin) {
        x = elm2;
        y = elm1;
    } else {
        x = elm1;
        y = elm2;
    }
    // x in front of y

    if (x.begin + x.length < y.begin) {
        return -1;  // no intersection, couldn't merge
    } else if (x.begin + x.length >= y.begin + y.length) {
        elm1 = x;
        return y.length;
    } else {
        elm1.begin = x.begin;
        elm1.data = x.data + y.data.substr(x.begin + x.length - y.begin);
        elm1.length = elm1.data.length();
        return x.begin + x.length - y.begin;
    }
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // stream start is 0
    if (index >= _head_index + _capacity) {
        // capacity over
        // after red
        return;
    }

    block_node elm;
    // couldn't = , because data have empty substing,
    // the end
    if (index + data.length() <= _head_index) {
        goto JUDGE_EOF;
    }
    // index ... _head_index
    // truncate the front segment
    else if (index < _head_index) {
        size_t offset = _head_index - index;
        elm.begin = index + offset;
        elm.data.assign(data.begin() + offset, data.end());
        elm.length = elm.data.length();
    }
    // the first init or index == _head_index
    else {
        elm.begin = index;
        elm.length = data.length();
        elm.data = data;
    }

    _unassembled_byte += elm.length;

    do {
        // green
        long merged_bytes = 0;
        auto iter = _blocks.lower_bound(elm);
        // when elm > the last block,then iter == end()
        // merge next
        while ((iter != _blocks.end()) && (merged_bytes = merge_block(elm, *iter)) >= 0) {
            _unassembled_byte -= merged_bytes;
            _blocks.erase(iter);

            iter = _blocks.lower_bound(elm);
        }
        // merge next

        // merge prev
        if (iter == _blocks.begin()) {
            // this is why "while(false)"
            break;
        }

        iter--;
        while((merged_bytes = merge_block(elm, *iter)) >= 0)
        {
            _unassembled_byte -= merged_bytes;
            _blocks.erase(iter);
            iter = _blocks.lower_bound(elm);
            if(iter == _blocks.begin())
            {
                break;
            }
            iter--;
        }
        // merge prev

    } while (false);
    _blocks.insert(elm);

    // write to ByteStream
    // as soon as possible
    if (!_blocks.empty() && _blocks.begin()->begin == _head_index) {
        const block_node head_block = *_blocks.begin();
        size_t write_bytes = _output.write(head_block.data);
        _head_index += write_bytes;
        _unassembled_byte -= write_bytes;
        _blocks.erase(_blocks.begin());
    }

    JUDGE_EOF:
    if(eof)
    {
        _eof_flag = true;
    }
    if(_eof_flag && empty())
    {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_byte; }

bool StreamReassembler::empty() const { return _unassembled_byte == 0; }
