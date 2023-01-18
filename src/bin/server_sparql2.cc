#include <boost/beast/core.hpp>
#include <boost/url/src.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "base/exceptions.h"
#include "base/binding/binding_iter.h"
#include "base/thread/thread_info.h"
#include "network/tcp_buffer.h"
#include "parser/query/grammar/error_listener.h"
//#include "parser/query/sparql_query_parser.h"
#include "parser/query/mdb_query_parser.h"
#include "query_optimizer/quad_model/quad_model.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"
#include "storage/filesystem.h"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

std::queue<std::shared_ptr<ThreadInfo>> running_threads_queue;
std::mutex running_threads_queue_mutex;
bool shutdown_server = false;

// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type(beast::string_view path) {
    using beast::iequals;
    if(iequals(path, "json")) return "application/json";
    if(iequals(path, "xml"))  return "application/xml";

    return "application/text";
}


std::string execute_query_json(std::string query, std::shared_ptr<ThreadInfo> thread_info) {
    // Execute query and stores bindings in response_body with json format

    antlr4::MyErrorListener error_listener;
    std::unique_ptr<BindingIter> physical_plan;

    std::stringbuf response_body;
    std::ostream os(&response_body);

    os << "{";

    // Exceptions are handled in handle_request

    auto logical_plan = MDB::QueryParser::get_query_plan(query, &error_listener);
    physical_plan = quad_model.exec(*logical_plan, thread_info.get());
    os << "\"results\": [";

    // Buffer to store results
    // Execute query
    physical_plan->begin(os);
    while (physical_plan->next()) {
        os << ",";
    }
    os.seekp(-1, os.cur); // Delete last ,
    // Send results in response
    os << "]}";
    // TODO: Ver si es posible evitar retornar una copia de string
    return response_body.str();

}

std::string execute_query_xml(std::string query, std::shared_ptr<ThreadInfo> thread_info) {
    // TODO:
    return "";
}


std::string execute_query_csv (std::string query, std::shared_ptr<ThreadInfo> thread_info) {
    // TODO:
    return "";
}


template <class Body, class Allocator>
http::message_generator handle_request(std::shared_ptr<ThreadInfo> thread_info,
                                       http::request<Body, http::basic_fields<Allocator>>&& req)
{
    // Returns a bad request response
    auto const request_error = [&req](beast::string_view why) {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type("json"));
        res.keep_alive(req.keep_alive());
        res.body() = "{\"error\": \"" + std::string(why) + "\"}";
        res.prepare_payload();
        return res;
    };

    std::string content_type;

    for (auto& header : req) {
        if (to_string(header.name()) == "Content-Type") {
            content_type = header.value();
        }
    }

    std::string params_string_encoded = req.target();

    // In post method add /sparql? to allow to decode with url view
    if (req.method() == http::verb::post && content_type != "application/sparql-query") {
        params_string_encoded = "/sparql?" + req.body();
    } else if (req.method() != http::verb::get && content_type != "application/sparql-query" ){
        return request_error("Illegal request method");
    }
    boost::urls::url_view uv;
    // Parse string to can access params values
    try {
        uv = boost::urls::url_view(params_string_encoded);
    } catch (...) {
        return request_error("Parsing params error");
    }
    std::string sparql_query;
    std::string output_format;

    // Post directly does not send query param in url or encoded in body
    if (content_type != "application/sparql-query") {
        // Check if query and output format is in params
        if (uv.params().find("query") == uv.params().end())
            return request_error("Not found 'query' param in message, check format");

        sparql_query = (*uv.params().find("query")).value;
    } else {
        sparql_query = req.body();
    }
    if (uv.params().find("format") == uv.params().end())
        return request_error("Not found 'format' param in message, check format");

    output_format = (*uv.params().find("format")).value;

    http::response<http::string_body> res{http::status::ok, req.version()};

    try {
        if (output_format == "json") {
            res.body() = execute_query_json(sparql_query, thread_info);
        } else if (output_format == "xml") {
            res.body() = execute_query_xml(sparql_query, thread_info);
        } else if (output_format == "csv") {
            res.body() = execute_query_csv(sparql_query, thread_info);
        }// TODO: RDF format else if (output == "rdf")
        else {
            return request_error("Non correct ouput format");
        }
    } catch (const std::exception& e) {
        return  request_error(e.what());
    }

    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type("json"));
    res.keep_alive(req.keep_alive());
    res.prepare_payload();
    return res;
    

}

void fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session> {
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    std::shared_ptr<ThreadInfo> thread_info;

public:
    // Take ownership of the stream
    session(tcp::socket&& socket, std::shared_ptr<ThreadInfo> thread_info) :
            stream_     (std::move(socket)),
            thread_info (thread_info) { }

    // Start the asynchronous operation
    void run() {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(stream_.get_executor(),
                      beast::bind_front_handler(
                          &session::do_read,
                          shared_from_this()));
    }

    void do_read() {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(30));

        // Read a request
        http::async_read(stream_, buffer_, req_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if(ec == http::error::end_of_stream)
            return do_close();

        if(ec)
            return fail(ec, "read");

        // Send the response
        send_response(handle_request(thread_info, std::move(req_)));
    }

    void send_response(http::message_generator&& msg) {
        bool keep_alive = msg.keep_alive();

        // Write the response
        beast::async_write(stream_,
                           std::move(msg),
                           beast::bind_front_handler(&session::on_write,
                                                     shared_from_this(),
                                                     keep_alive)
                            );
    }

    void on_write(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if(ec)
            return fail(ec, "write");

        if(! keep_alive) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // Read another request
        do_read();
    }

    void do_close() {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};


void execute_timeouts(std::chrono::seconds timeout_duration) {
    std::chrono::nanoseconds sleep_time;
    while (!shutdown_server) {
        bool should_sleep = false;
        {
            std::lock_guard<std::mutex> guard(running_threads_queue_mutex);
            if (running_threads_queue.empty()) {
                should_sleep = true;
                sleep_time = timeout_duration;
            } else {
                auto thread_info = running_threads_queue.front();
                auto now = std::chrono::system_clock::now();

                if (thread_info->timeout <= now || thread_info->finished) {
                    thread_info->interruption_requested = true; // if thread finished this don't matter
                    running_threads_queue.pop();
                } else {
                    // should't sleep here because the mutex is locked
                    sleep_time = thread_info->timeout - now;
                    should_sleep = true;
                }
            }
        }
        if (should_sleep) {
            std::this_thread::sleep_for(sleep_time);
        }
    }
}


//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener> {
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::chrono::seconds timeout_seconds;

public:
    listener(net::io_context& ioc, tcp::endpoint endpoint, std::chrono::seconds timeout_seconds) :
             ioc_               (ioc),
             acceptor_          (net::make_strand(ioc)),
             timeout_seconds    (timeout_seconds)
    {
        beast::error_code ec;
        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec) {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec) {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec) {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if(ec) {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void run() {
        do_accept();
    }

private:
    void do_accept() {
        // The new connection gets its own strand
        acceptor_.async_accept(net::make_strand(ioc_),
                               beast::bind_front_handler(&listener::on_accept,
                                                         shared_from_this())
                                );
    }

    void on_accept(beast::error_code ec, tcp::socket socket) {
        if(ec) {
            fail(ec, "accept");
            return; // To avoid infinite loop
        } else {
            //auto now = std::chrono::system_clock::now();
            //auto timeout = now + timeout_seconds;
            //uint64_t timestamp = now.time_since_epoch().count();

            auto thread_info = std::make_shared<ThreadInfo>();

            {
                // TODO: stablish private buffer pos? what happens if there is no available private buffer?
                thread_info->timeout = std::chrono::system_clock::now() + timeout_seconds;
                std::lock_guard<std::mutex> guard(running_threads_queue_mutex);
                running_threads_queue.push(thread_info);
            }

            // Create the session and run it
            std::make_shared<session>(std::move(socket), thread_info)->run();
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    // Check command line arguments.
    if (argc != 6) {
        std::cerr <<
            "Usage: http-server-async <address> <port> <threads> <bd> <timeout_seconds>\n" <<
            "Example:\n" <<
            "    http-server-async 0.0.0.0 8080 5 bd\n";
        return EXIT_FAILURE;
    }

    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = std::max<int>(1, std::atoi(argv[3]));
    auto const db_folder = (std::string) argv[4];
    auto const timeout_seconds = std::chrono::seconds(atoi(argv[5]));

    uint_fast32_t shared_buffer_size = 1024 * 256;
    uint_fast32_t private_buffer_size = 1024 * 16;

    auto model_destroyer = QuadModel::init(db_folder,
                                          shared_buffer_size,
                                          private_buffer_size,
                                          threads);

    std::cout << "Initializing server..." << std::endl;
    quad_model.catalog().print();

    std::thread(execute_timeouts, timeout_seconds).detach();


    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<listener>(ioc, tcp::endpoint{address, port}, timeout_seconds)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] {
                        ioc.run();
                    });
    ioc.run();

    return EXIT_SUCCESS;
}