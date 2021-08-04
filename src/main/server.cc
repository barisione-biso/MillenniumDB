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
#include <queue>
#include <memory>
#include <mutex>
#include <ostream>
#include <random>
#include <thread>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "base/exceptions.h"
#include "base/binding/binding.h"
#include "base/binding/binding_iter.h"
#include "base/graph/graph_model.h"
#include "base/exceptions.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/query_parser.h"
#include "base/thread/thread_key.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"
#include "server/tcp_buffer.h"

using namespace std;
using boost::asio::ip::tcp;
namespace po = boost::program_options;

// For random generation
std::uniform_int_distribution<uint64_t> random_uint64;
std::random_device rd;
std::mt19937_64 gen(rd());

std::queue<ThreadKey> running_threads_queue;
std::unordered_map<ThreadKey, ThreadInfo, ThreadKeyHasher> running_threads;
std::mutex running_threads_mutex;

std::chrono::seconds query_timeout;


void finish_thread(const ThreadKey thread_key) {
    std::lock_guard<std::mutex> guard(running_threads_mutex);
    auto it = running_threads.find(thread_key);
    if (it != running_threads.end()) {
        running_threads.erase(it);
    }
}


void execute_query(unique_ptr<BindingIter> binding_iter, std::ostream& os) {
    // prepare to start the execution
    auto start = chrono::system_clock::now();
    unsigned int result_count = 0;

    binding_iter->begin();
    auto& binding = binding_iter->get_binding();

    // get all results
    while (binding_iter->next()) {
        os << binding << endl;
        result_count++;
    }

    chrono::duration<float, std::milli> duration = chrono::system_clock::now() - start;

    // print execution stats in server console
    cout << "\nPlan Executed:\n";
    binding_iter->analyze(2);
    cout << "\nResults:" << result_count << "\n";

    // write execution stats in output stream
    os << "Found " << std::to_string(result_count) << " results.\n";
    os << "Execution time: " << std::to_string(duration.count()) << " ms.\n";
}


void session(ThreadKey thread_key, ThreadInfo* thread_info, tcp::socket sock, GraphModel* model) {
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
        cout << "--------------------------\n";
        cout << query << "\n";
        cout << "--------------------------\n";

        TcpBuffer tcp_buffer = TcpBuffer(sock);
        tcp_buffer.begin(db_server::MessageType::plain_text);
        std::ostream os(&tcp_buffer);

        // without this line ConnectionException won't be catched propertly
        os.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        unique_ptr<BindingIter> physical_plan;

        // start timer
        auto start = chrono::system_clock::now();
        try {
            auto logical_plan = QueryParser::get_query_plan(query);
            physical_plan = model->exec(*logical_plan, thread_info);
        }
        // catch (QueryParsingException& e) {
        //     // Try with manual plan
        //     try {
        //         auto manual_plan = QueryParser::get_manual_plan(query);
        //         physical_plan = model->exec(manual_plan);
        //     }
        //     catch (QueryException& e) {
        //         os << "(Manual Plan) Query Parsing Exception: " << e.what() << "\n";
        //         tcp_buffer.set_error();
        //     }
        // }
        catch (const QueryException& e) {
            os << "Query Exception: " << e.what() << "\n";
            tcp_buffer.set_error();
            finish_thread(thread_key);
            return;
        }
        chrono::duration<float, std::milli> parser_duration = chrono::system_clock::now() - start;
        try {
            execute_query(move(physical_plan), os);
        }
        catch (const InterruptedException& e) {
            std::cerr << "QueryInterrupted" << endl;
            auto t = chrono::system_clock::now() - start;
            os << "\nTimeout thrown after "
               << std::chrono::duration_cast<std::chrono::milliseconds>(t).count()
               << " milliseconds.\n";
            os << "Query Parser/Optimizer time: " << parser_duration.count() << " ms.\n";
            tcp_buffer.set_error();
            finish_thread(thread_key);
            return;
        }
        os << "Query Parser/Optimizer time: " << parser_duration.count() << " ms.\n";
    }
    catch (const ConnectionException& e) {
        std::cerr << "Lost connection with client: " << e.what() << endl;
        finish_thread(thread_key);
    }
    catch (...) {
        std::cerr << "Unknown exception." << endl;
        finish_thread(thread_key);
    }
    finish_thread(thread_key);
}


void execute_timeouts() {
    std::chrono::nanoseconds sleep_time;
    while (true) {
        bool should_sleep = false;
        if (running_threads_queue.empty()) {
            // cout << "running_threads_queue empty\n";
            should_sleep = true;
            sleep_time = query_timeout;
        } else {
            auto thread_key = running_threads_queue.front();
            std::lock_guard<std::mutex> guard(running_threads_mutex);
            auto it = running_threads.find(thread_key);
            if (it == running_threads.end()) {
                // if thread_key is not in running_threads, it means the thread already finished
                running_threads_queue.pop();
            } else {
                auto now = chrono::system_clock::now();
                if (it->second.timeout <= now) {
                    it->second.interruption_requested = true;
                    // cout << "Requested cancelation for thread (" << thread_key.timestamp << ", " << thread_key.salt << ")\n";
                    // cout << "timeout: " << it->second.timeout.time_since_epoch().count() << ")\n";
                    // cout << "now:     " << now.time_since_epoch().count() << ")\n";
                    running_threads_queue.pop();
                }
                else {
                    // can't sleep here because the mutex is active
                    // cout << "Not canceling\n";
                    auto remaining = it->second.timeout - now;
                    // cout << "timeout:   " << it->second.timeout.time_since_epoch().count() << ")\n";
                    // cout << "now:       " << now.time_since_epoch().count() << ")\n";
                    // cout << "remaining: " << std::chrono::duration_cast<std::chrono::milliseconds>(remaining).count() << "ms.\n";
                    should_sleep = true;
                    sleep_time = remaining;
                }
            }
        }
        if (should_sleep) {
            // std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::this_thread::sleep_for(sleep_time);
        }
    }
}


void server(boost::asio::io_context& io_context, unsigned short port, GraphModel* model) {
    std::thread(execute_timeouts).detach();

    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    cout << "Server running." << endl;
    while (true) {
        tcp::socket sock(io_context);
        acceptor.accept(sock);

        auto now = chrono::system_clock::now();

        auto timeout = now + query_timeout;
        uint64_t timestamp = now.time_since_epoch().count();
        uint64_t rand = random_uint64(gen);

        ThreadKey thread_key(timestamp, rand);
        ThreadInfo thread_info(timeout);

        // cout << "Created new thread (" << timestamp << ", " << rand << ")\n";

        running_threads_queue.push(thread_key);
        auto insertion = running_threads.insert({thread_key, thread_info});
        std::thread(session, thread_key, &insertion.first->second, std::move(sock), model).detach();
    }
}


int main(int argc, char **argv) {
    int seconds_timeout;
    int port;
    int shared_buffer_size;
    int private_buffer_size;
    int max_threads;
    string db_folder;

    try {
        // Parse arguments
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "show this help message")
            ("db-folder,d", po::value<string>(&db_folder)->required(), "set database folder path")
            ("buffer-size,b", po::value<int>(&shared_buffer_size)->default_value(BufferManager::DEFAULT_SHARED_BUFFER_POOL_SIZE),
                "set shared buffer pool size")
            ("private-buffer-size,", po::value<int>(&private_buffer_size)->default_value(BufferManager::DEFAULT_PRIVATE_BUFFER_POOL_SIZE),
                "set private buffer pool size for each thread")
            ("max-threads,", po::value<int>(&max_threads)->default_value(8),
                "set max threads")
            ("port,p", po::value<int>(&port)->default_value(db_server::DEFAULT_PORT), "database server port")
            ("timeout", po::value<int>(&seconds_timeout)->default_value(60), "timeout (in seconds)")
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

        // Validate params
        if (port < 0) {
            cerr << "Port cannot be a negative number.\n";
            return 1;
        }

        if (seconds_timeout < 0) {
            cerr << "Timeout cannot be a negative number.\n";
            return 1;
        }

        if (shared_buffer_size < 0) {
            cerr << "Buffer size cannot be a negative number.\n";
            return 1;
        }

        if (private_buffer_size < 0) {
            cerr << "Private buffer size cannot be a negative number.\n";
            return 1;
        }

        // Initialize model
        QuadModel model(db_folder, shared_buffer_size, private_buffer_size, max_threads);

        cout << "Initializing server...\n";
        model.catalog().print();

        query_timeout = std::chrono::seconds(seconds_timeout);
        boost::asio::io_context io_context;
        boost::asio::deadline_timer t(io_context, boost::posix_time::seconds(5));

        server(io_context, port, &model);
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
