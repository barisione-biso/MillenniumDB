#pragma once

#include <chrono>
#include <iostream>
#include <ostream>
#include <memory>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include "base/thread/thread_info.h"

class Op;
class BindingIter;

namespace SPARQL_SERVER {

class SparqlServer;

// Handles an HTTP server connection
class Session : public std::enable_shared_from_this<Session> {
    SparqlServer& sparql_server;
    boost::beast::tcp_stream stream;
    boost::beast::flat_buffer buffer;
    boost::beast::http::request<boost::beast::http::string_body> req;
    std::chrono::seconds timeout;

    ThreadInfo* thread_info;

public:
    // Take ownership of the stream
    Session(SparqlServer& sparql_server, boost::asio::ip::tcp::socket&& socket, std::chrono::seconds timeout);

    // Start the asynchronous operation
    void run();

    void do_read();

    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);

    void fail(boost::beast::error_code ec, char const* what) {
        std::cerr << what << ": " << ec.message() << "\n";
    }

private:
    std::unique_ptr<Op> create_logical_plan(const std::string& query,
                                            std::chrono::duration<float, std::milli>* duration);

    std::unique_ptr<BindingIter> create_readonly_physical_plan(Op& logical_plan,
                                                               std::chrono::duration<float, std::milli>* duration);

    void execute_plan(BindingIter& physical_plan,
                      std::ostream& os,
                      std::chrono::duration<float, std::milli>* duration);
};
}