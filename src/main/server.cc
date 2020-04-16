#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <thread>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "base/parser/logical_plan/exceptions.h"
#include "base/binding/binding.h"
#include "base/binding/binding_iter.h"
#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/query_optimizer/physical_plan_generator.h"
#include "relational_model/relational_model.h"
#include "server/tcp_buffer.h"
#include "storage/buffer_manager.h"

using namespace std;
using boost::asio::ip::tcp;
namespace po = boost::program_options;

const int max_length = 1024;

void session(tcp::socket sock) {
    try {
        while (true) {
            char data[max_length];

            boost::system::error_code error;
            size_t length = sock.read_some(boost::asio::buffer(data), error); // TODO: porder leer consultas más largas que el buffer
            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            string query = string(data, length);

            cout << "Query received:\n";
            cout << query << "\n";

            TcpBuffer tcp_buffer = TcpBuffer(sock);
            tcp_buffer.begin(MessageType::plain_text);

            // start timer
            auto start = chrono::system_clock::now();
            try {
                auto select_plan = Op::get_select_plan(query);

                PhysicalPlanGenerator plan_generator { };
                auto root = plan_generator.exec(*select_plan);

                root->begin();
                auto binding = root->next();
                int count = 0;
                while (binding != nullptr) {
                    tcp_buffer << binding->to_string();
                    cout << binding->to_string();
                    binding = root->next();
                    count++;
                }

                auto end = chrono::system_clock::now();
                chrono::duration<float, std::milli> duration = end - start;
                tcp_buffer << "Found " << std::to_string(count) << " results.\n";
                tcp_buffer << "Execution time: " << std::to_string(duration.count()) << " milliseconds.\n";
            } catch (QueryException& e) {
                tcp_buffer << "Query exception: " << e.what() << "\n";
            }
            tcp_buffer.end();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}


void server(boost::asio::io_service& io_service, unsigned short port) {
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
    while (true) {
        tcp::socket sock(io_service);
        a.accept(sock);
        std::thread(session, std::move(sock)).detach();
    }
}


int main(int argc, char **argv) {
    try {
        // Parse arguments
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "show this help message")
            ("db-folder,d", po::value<string>()->required(), "set database folder path")
            ("buffer-size,b", po::value<int>(), "set buffer pool size")
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

        file_manager.init(vm["db-folder"].as<string>());

        if (vm.count("buffer-size")) {
            buffer_manager.init(vm["buffer-size"].as<int>());
        } else {
            buffer_manager.init();
        }
        RelationalModel::init();

        boost::asio::io_service io_service;
        boost::asio::deadline_timer t(io_service, boost::posix_time::seconds(5));

        server(io_service, 8080); // TODO: make port as param and define DEFAULT port
    }
    catch(exception& e) {
        cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }
}
