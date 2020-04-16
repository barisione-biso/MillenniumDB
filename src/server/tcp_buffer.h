#ifndef SERVER__TCP_BUFFER_H_
#define SERVER__TCP_BUFFER_H_

#include <ostream>
#include <string>
#include <boost/asio.hpp>

#include "server/message_type.h"

using boost::asio::ip::tcp;

class TcpBuffer {
public:
    static constexpr int buffer_size = 1024;
    TcpBuffer(tcp::socket& sock);
    ~TcpBuffer();

    TcpBuffer& operator<<(std::string msg);
    void begin(MessageType msg_type);
    void end();

private:
    int current_pos;
    unsigned char buffer[buffer_size];
    tcp::socket& sock;
    void send();
};

#endif // SERVER__TCP_BUFFER_H_
