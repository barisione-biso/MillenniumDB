#include "tcp_buffer.h"

#include <cassert>

TcpBuffer::TcpBuffer(tcp::socket& sock) :
    sock (sock) { }


TcpBuffer::~TcpBuffer() = default;


void TcpBuffer::begin(db_server::MessageType msg_type) {
    buffer[0] = static_cast<unsigned char>(msg_type);
    // skip first byte for type and 2 bytes for length
    current_pos = 3;
}


int TcpBuffer::overflow(int i) {
    char c = static_cast<char>(i);
    assert(current_pos < db_server::BUFFER_SIZE);
    buffer[current_pos] = c;
    ++current_pos;
    if (current_pos == db_server::BUFFER_SIZE) {
        send(); // send() will reset current_pos
    }
    return i;
}


void TcpBuffer::set_error() {
    error = true;
}


void TcpBuffer::end() {
    if (error) {
        buffer[0] = static_cast<unsigned char>(db_server::MessageType::end_fail);
    } else {
        buffer[0] = static_cast<unsigned char>(db_server::MessageType::end_success);
    }
    send();
}


void TcpBuffer::send() {
    unsigned char b1 = current_pos & 0xFF;
    unsigned char b2 = (current_pos >> 8) & 0xFF;

    buffer[1] = b1;
    buffer[2] = b2;

    boost::asio::write(sock, boost::asio::buffer(buffer, db_server::BUFFER_SIZE));
    current_pos = 3;
}
