/*
 * server is a executable that listens for tcp conections asking for queries,
 * and it send the results to the client.
 *
 * There are 4 methods:
 *
 * 1) main: parses the program options (e.g: buffer size, port, database folder).
 *    Creates the proper GraphModel and calls the method `server` to start the server.
 *
 * 2) server: infinite loop, waiting for new TCP connections. When a connection
 *    is established, it calls the `session` method in a different thread, and the loop
 *    starts again in the main thread, waiting for another connection.
 *
 * 3) session: Creates a TcpBuffer to send the query, parses the query, getting a logical
 *    plan and then a physical plan. The physical plan and the TcpBuffer are passed to
 *    `execute_query`.
 *
 * 4) execute_query: receives a physical plan and a TcpBufer. Executes the physical plan
 *    enumerating all results, writing them into the TcpBuffer.
 */

#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <thread>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "base/binding/binding.h"
#include "base/binding/binding_iter.h"
#include "base/graph/graph_model.h"
#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/query_parser.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"
#include "server/tcp_buffer.h"

using namespace std;
using boost::asio::ip::tcp;
namespace po = boost::program_options;

void execute_query(unique_ptr<BindingIter> root, TcpBuffer& tcp_buffer) {
    // prepare to start the execution
    auto start = chrono::system_clock::now();
    unsigned int count = 0;
    auto& binding = root->get_binding();

    // get all results
    while (root->next()) {
        // TODO: uncomment/comment to enable/disable printing results
        tcp_buffer << binding.to_string();
        count++;
    }

    // print execution stats
    cout << "\nPlan Executed:\n";
    root->analyze(2);
    cout << "\n";

    auto end = chrono::system_clock::now();
    chrono::duration<float, std::milli> duration = end - start;
    tcp_buffer << "Found " << std::to_string(count) << " results.\n";
    tcp_buffer << "Execution time: " << std::to_string(duration.count()) << " ms.\n";
}


void session(tcp::socket sock, GraphModel* model) {
    try {
        unsigned char query_size_b[db_server::BYTES_FOR_QUERY_LENGTH];
        boost::asio::read(sock, boost::asio::buffer(query_size_b, db_server::BYTES_FOR_QUERY_LENGTH));

        int query_size = 0;
        for (int i = 0, offset = 0; i < db_server::BYTES_FOR_QUERY_LENGTH; i++, offset += 8) {
            query_size += query_size_b[i] << offset;
        }
        std::string query;
        query.resize(query_size);
        boost::asio::read(sock, boost::asio::buffer(query.data(), query_size));
        cout << "--------------------------\n";
        cout << " Query received:\n";
        cout << query << "\n";
        cout << "--------------------------\n";

        TcpBuffer tcp_buffer = TcpBuffer(sock);
        tcp_buffer.begin(db_server::MessageType::plain_text);

        // start timer
        auto start = chrono::system_clock::now();
        try {
            auto logical_plan = QueryParser::get_query_plan(query);
            auto physical_plan = model->exec(*logical_plan);

            auto end = chrono::system_clock::now();
            chrono::duration<float, std::milli> duration = end - start;
            execute_query(move(physical_plan), tcp_buffer);
            tcp_buffer << "Query Parser/Optimizer time: " << std::to_string(duration.count()) << " ms.\n";
        }
        catch (QueryParsingException& e) {
            // Try with manual plan
            try {
                auto manual_plan = QueryParser::get_manual_plan(query);
                auto physical_plan = model->exec(manual_plan);
                auto end = chrono::system_clock::now();
                chrono::duration<float, std::milli> duration = end - start;
                tcp_buffer << "Manual Plan Optimizer time: " << std::to_string(duration.count()) << " ms.\n";
                execute_query(move(physical_plan), tcp_buffer);
            }
            catch (QueryException& e) {
                tcp_buffer << "(Manual Plan) Query Parsing Exception: " << e.what() << "\n";
                tcp_buffer.set_error();
            }
        }
        catch (QueryException& e) {
            tcp_buffer << "Query Exception: " << e.what() << "\n";
            tcp_buffer.set_error();
        }
        tcp_buffer.end();
    }
    catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}


void server(boost::asio::io_service& io_service, unsigned short port, GraphModel* model) {
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
    while (true) {
        tcp::socket sock(io_service);
        a.accept(sock);
        std::thread(session, std::move(sock), model).detach();
    }
}


int main(int argc, char **argv) {
    int port;
    int buffer_size;
    string db_folder;

    try {
        // Parse arguments
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "show this help message")
            ("db-folder,d", po::value<string>(&db_folder)->required(), "set database folder path")
            ("buffer-size,b", po::value<int>(&buffer_size)->default_value(BufferManager::DEFAULT_BUFFER_POOL_SIZE),
                "set buffer pool size")
            ("port,p", po::value<int>(&port)->default_value(db_server::DEFAULT_PORT), "database server port")
        ;

        po::positional_options_description p;
        p.add("db-folder", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

        if (vm.count("help")) {
            cout << "Usage: server [options] DB_FOLDER\n";
            cout << desc << "\n";
            return 0;
        }
        po::notify(vm);

        unique_ptr<GraphModel> model = make_unique<QuadModel>(db_folder, buffer_size);

        boost::asio::io_service io_service;
        boost::asio::deadline_timer t(io_service, boost::posix_time::seconds(5));

        server(io_service, port, model.get());
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        cerr << "Exception of unknown type!\n";
        return 1;
    }
    return 0;
}
