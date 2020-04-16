#include "tcp_buffer.h"

TcpBuffer::TcpBuffer(tcp::socket& sock)
    : sock(sock) { }


TcpBuffer::~TcpBuffer() = default;


void TcpBuffer::begin(MessageType msg_type) {
    buffer[0] = static_cast<unsigned char>(msg_type);
    // skip first byte for type and 2 bytes for length
    current_pos = 3;
}


TcpBuffer& TcpBuffer::operator<<(std::string msg) {
    auto msg_len = msg.size();
    unsigned available_space = buffer_size - current_pos; // unsigned instead of auto to avoid warning in comparison
    if (msg_len < available_space) { // msg fits in buffer
        memcpy( &buffer[current_pos], msg.data(), msg_len );
        current_pos += msg_len;
        if (current_pos == buffer_size) {
            send();
        }
    } else { // msg needs to be splitted, could need multiple sends
        for (auto it = msg.begin(); it != msg.end(); ++it) {
            buffer[current_pos++] = *it;
            if (current_pos == buffer_size) {
                send();
            }
        }
    }
    return *this;
}


void TcpBuffer::end() {
    buffer[0] += 1; // convention says end_type should be type + 1
    send();
}


void TcpBuffer::send() {
    unsigned char b1 = current_pos & 0xFF;
    unsigned char b2 = (current_pos >> 8) & 0xFF;

    buffer[1] = b1;
    buffer[2] = b2;

    boost::asio::write(sock, boost::asio::buffer(buffer, buffer_size));
    current_pos = 3;
}
