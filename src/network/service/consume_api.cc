// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
// Official repository: https://github.com/boostorg/beast
#include "consume_api.h"

#include <iostream>
#include <string>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/version.hpp>
#include <boost/lexical_cast.hpp>


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


std::string consume(std::string const& host, std::string const& port, std::string const& target,
                    std::string const& format, bool const& https){
    try {
        int version = 11;
        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;
        // Declare a container to hold the response
        http::response_parser<http::string_body> parser;
        parser.body_limit((std::numeric_limits<std::uint64_t>::max)());

        // Error code
        beast::error_code ec;
        // The io_context is required for all I/O
        boost::asio::io_context ctx;

        if(https){
            // The SSL context is required, and certificates are not validated
            ssl::context ssl_ctx(ssl::context::tlsv12_client);
            ssl_ctx.set_verify_mode(ssl::context::verify_peer | ssl::context::verify_fail_if_no_peer_cert);
            ssl_ctx.set_default_verify_paths();

            boost::asio::ip::tcp::socket socket{ctx};
            tcp::resolver resolver{ctx};
            boost::asio::connect(socket, resolver.resolve(host, port));
            auto stream = boost::make_unique<ssl::stream<boost::asio::ip::tcp::socket>>(std::move(socket), ssl_ctx);
            stream->handshake(ssl::stream_base::handshake_type::client);

            http::request<http::empty_body> request{http::verb::get, target, version};
            request.method(http::verb::get);
            request.target(target);
            request.keep_alive(false);
            request.set(http::field::host, host);
            request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            if (format == "json") request.set("Accept", "application/json");
            else if (format == "xml") request.set("Accept", "application/xml");
            else if (format == "csv") request.set("Accept", "text/csv");
            else if (format == "tsv") request.set("Accept", "text/tab-separated-values");

            http::write(*stream, request);

            http::read(*stream, buffer, parser);
            stream->shutdown(ec);
            stream->next_layer().close(ec);
        } else {
            // These objects perform our I/O
            tcp::resolver resolver{ctx};
            // Look up the domain name
            tcp::resolver::results_type const results = resolver.resolve(host, port);

            beast::tcp_stream stream(ctx);
            // Make the connection on the IP address we get from a lookup
            stream.connect(results);

            // Set up an HTTP GET request message
            http::request<http::string_body> request{http::verb::get, target, version};
            request.set(http::field::host, host);
            request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            if(format == "json") request.set("Accept", "application/sparql-results+json");
            else if (format == "xml") request.set("Accept", "application/sparql-results+xml");
            else if (format == "csv") request.set("Accept", "text/csv");
            else if (format == "tsv") request.set("Accept", "text/tab-separated-values");

            // Send the HTTP request to the remote host
            http::write(stream, request);
            // Receive the HTTP response
            http::read(stream, buffer, parser);
            stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        }

        if(ec && ec != beast::errc::not_connected) throw beast::system_error{ec};

        std::string const string_response = boost::lexical_cast<std::string>(parser.get().body());
        return string_response;

    } catch(std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return "ERR";
    }
}