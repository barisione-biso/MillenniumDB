#include "session.h"

#include "base/exceptions.h"
#include "parser/query/grammar/error_listener.h"
#include "parser/query/sparql_query_parser.h"
#include "network/http_buffer.h"
#include "network/sparql/request_handler.h"
#include "network/sparql/sparql_server.h"

using namespace boost;
using namespace SPARQL_SERVER;
using namespace std;

Session::Session(SparqlServer& sparql_server, boost::asio::ip::tcp::socket&& socket, std::chrono::seconds timeout) :
    sparql_server (sparql_server),
    stream        (std::move(socket)),
    timeout       (timeout)
{
    auto thread_index = sparql_server.thread_id_map[std::this_thread::get_id()];
    thread_info = &sparql_server.thread_info_vec[thread_index];
}

void Session::run() {
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    asio::dispatch(stream.get_executor(),
                   beast::bind_front_handler(&Session::do_read, shared_from_this()));
}


void Session::do_read() {
    // Make the request empty before reading, otherwise the operation behavior is undefined.
    req = {};

    // Set the timeout.
    stream.expires_after(timeout); // TODO: not query timeout, should be different?

    // Read a request
    beast::http::async_read(stream,
                            buffer,
                            req,
                            beast::bind_front_handler(&Session::on_read, shared_from_this()));
}


unique_ptr<Op> Session::create_logical_plan(const string& query,
                                            chrono::duration<float, std::milli>* parser_duration)
{
    auto start_parser = chrono::system_clock::now();
    try {
        antlr4::MyErrorListener error_listener;
        auto logical_plan = SPARQL::QueryParser::get_query_plan(query, &error_listener);
        *parser_duration = chrono::system_clock::now() - start_parser;
        return logical_plan;
    }
    catch (const QueryException& e) {
        std::cout << "---------------------------------------\n";
        std::cout << "Query Exception: " << e.what() << "\n";
        std::cout << "---------------------------------------" << std::endl;
        // TODO: send error to client
        throw e;
    }
    catch (const LogicException& e) {
        std::cout << "---------------------------------------\n";
        std::cout << "Logic Exception: " << e.what() << "\n";
        std::cout << "---------------------------------------" << std::endl;
        // TODO: send error to client
        throw e;
    }
}


void Session::execute_plan(BindingIter& physical_plan,
                           ostream& os,
                           chrono::duration<float, std::milli>* execution_duration)
{
    auto execution_start = chrono::system_clock::now();
    try {
        os << "HTTP/1.1 200 OK\r\n"
           << "Server: MillenniumDB\r\n"
           << "Content-Type: application/json; charset=utf-8\r\n" // TODO: depends
           << "\r\n";

        uint64_t result_count = 0;
        physical_plan.begin(os);
        // get all results
        while (physical_plan.next()) {
            result_count++;
        }
        *execution_duration = chrono::system_clock::now() - execution_start;

        std::cout << "\nPlan Executed:\n";
        physical_plan.analyze(std::cout);
        std::cout << "\nResults: " << result_count << "\n";
        thread_info->finished = true; // TODO: do something more?
    }
    catch (const InterruptedException& e) {
        std::cerr << "QueryInterrupted" << std::endl;
        // TODO: send error to client?
    }
}


unique_ptr<BindingIter> Session::create_readonly_physical_plan(Op& logical_plan,
                                                               chrono::duration<float, std::milli>* optimizer_duration)
{
    // std::shared_lock s_lock(execution_mutex); // TODO:
    auto start_optimizer = chrono::system_clock::now();
    try {
        auto physical_plan = rdf_model.exec(logical_plan, thread_info);
        *optimizer_duration = chrono::system_clock::now() - start_optimizer;
        return physical_plan;
    }
    catch (const QueryException& e) {
        std::cout << "---------------------------------------\n";
        std::cout << "Query Exception: " << e.what() << "\n";
        std::cout << "---------------------------------------" << std::endl;
        // TODO: send error to client
        throw e;
    }
    catch (const LogicException& e) {
        std::cout << "---------------------------------------\n";
        std::cout << "Logic Exception: " << e.what() << "\n";
        std::cout << "---------------------------------------" << std::endl;
        // TODO: send error to client
        throw e;
    }
}


void Session::on_read(beast::error_code ec, std::size_t /*bytes_transferred*/) {
    // This means they closed the connection
    if (ec == beast::http::error::end_of_stream) {
        stream.socket().shutdown(asio::ip::tcp::socket::shutdown_send, ec);
        return;
    }

    if (ec)
        return fail(ec, "read");

    HttpBuffer http_buffer(stream.socket());
    std::ostream os(&http_buffer);

    if (req.target().rfind("/sparql", 0) == std::string::npos) {
         os << "HTTP/1.1 404 Not Found\r\n"
            << "\r\n";
        http_buffer.finish();
        return;
    }

    auto query = RequestHandler::parse_request(req);

    // without this line ConnectionException won't be caught properly
    os.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    chrono::duration<float, std::milli> parser_duration;
    chrono::duration<float, std::milli> optimizer_duration;
    chrono::duration<float, std::milli> execution_duration;

    try {
        auto logical_plan = create_logical_plan(query, &parser_duration);

        if (logical_plan->read_only()) {
            auto physical_plan = create_readonly_physical_plan(*logical_plan, &optimizer_duration);
            execute_plan(*physical_plan, os, &execution_duration);
            http_buffer.finish();

            std::cout << "Parser duration:" << parser_duration.count() << "ms.\n";
            std::cout << "Optimizer duration:" << optimizer_duration.count() << "ms.\n";
            std::cout << "Execution duration:" << execution_duration.count() << "ms.\n";
            std::cout.flush();
        } else {
            // TODO: execute update when its supported
        }
    }
    catch (const QueryException& e) {
        os << "HTTP/1.1 400 Bad Request\r\n"
           << "Content-Type text/plain"
           << "\r\n"
           << std::string(e.what());
    }
    catch (const LogicException& e) {
        os << "HTTP/1.1 500 Internal Server Error\r\n"
           << "Content-Type text/plain"
           << "\r\n"
           << std::string(e.what());
    }
    catch (const InterruptedException& e) {
        http_buffer.finish();
        // TODO: what to do?
        // os << "Query Parser time: " << parser_duration.count() << " ms. \n"
        //    << "Optimizer time: " << optimizer_duration.count() << " ms." << std::endl;
    }
    catch (const ConnectionException& e) {
        std::cout << "---------------------------------------\n";
        std::cout << "Connection Exception: " << e.what() << "\n";
        std::cout << "---------------------------------------" << std::endl;
    }
}
