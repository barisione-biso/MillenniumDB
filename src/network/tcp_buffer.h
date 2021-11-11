#ifndef SERVER__TCP_BUFFER_H_
#define SERVER__TCP_BUFFER_H_

#include <string>
#include <boost/asio.hpp>

#include "network/communication_protocol.h"

class TcpBuffer : public std::stringbuf {
public:
    TcpBuffer(boost::asio::ip::tcp::socket& sock);
    ~TcpBuffer();

    void set_status(CommunicationProtocol::StatusCodes error);

protected:
    int overflow(int c) override;
    int sync() override;

private:
    int current_pos;
    unsigned char buffer[CommunicationProtocol::BUFFER_SIZE];
    boost::asio::ip::tcp::socket& sock;
    void send();
};

#endif // SERVER__TCP_BUFFER_H_
