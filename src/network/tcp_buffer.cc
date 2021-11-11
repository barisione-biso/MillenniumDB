#include "tcp_buffer.h"

#include <cassert>

#include "base/exceptions.h"

using boost::asio::ip::tcp;

TcpBuffer::TcpBuffer(tcp::socket& sock) :
    sock (sock)
{
    // default status is unexpected_error because in that case is hard to handle and set the status
    // If the query is executed without exceptions or if we catch an expected exception this will change
    buffer[0] = static_cast<unsigned char>(CommunicationProtocol::StatusCodes::unexpected_error);
    // skip first byte for status code and 2 bytes for length
    current_pos = 1;
}


TcpBuffer::~TcpBuffer() {
    if (sock.is_open()) {
        buffer[0] |= CommunicationProtocol::end_mask;
        send();
    }
}


int TcpBuffer::overflow(int i) {
    char c = static_cast<char>(i);
    assert(current_pos < CommunicationProtocol::BUFFER_SIZE);
    buffer[current_pos] = c;
    ++current_pos;
    if (current_pos == CommunicationProtocol::BUFFER_SIZE) {
        send(); // send() will reset current_pos
    }
    return i;
}


void TcpBuffer::set_status(CommunicationProtocol::StatusCodes error) {
    buffer[0] = static_cast<unsigned char>(error);
}


int TcpBuffer::sync() {
    send();
    return 0;
}


void TcpBuffer::send() {
    unsigned char b1 = current_pos & 0xFF;
    unsigned char b2 = (current_pos >> 8) & 0xFF;

    buffer[1] = b1;
    buffer[2] = b2;

    boost::system::error_code ec;
    boost::asio::write(sock, boost::asio::buffer(buffer, CommunicationProtocol::BUFFER_SIZE), ec);
    current_pos = 3;
    if (ec) {
        sock.close();
        throw ConnectionException(ec.message());
    }
}
