#include "sparql_server.h"

#include <array>

#include <boost/asio.hpp>

#include "network/sparql/listener.h"

using namespace boost;
using namespace SPARQL_SERVER;
using namespace std;

// TODO: std::queue< ThreadInfo&, start_time, end_time >
// si start_time es distinto a thread_info.start_time quiere decir que ya termino la consulta

void SparqlServer::execute_timeouts() {
    while (!shutdown_server) {
        running_threads_queue_mutex.lock();
        if (running_threads_queue.empty()) {
            running_threads_queue_mutex.unlock();
            std::this_thread::sleep_for(chrono::milliseconds(1000));
        } else {
            auto thread_info = running_threads_queue.front();
            auto now = chrono::system_clock::now();

            if (thread_info->timeout <= now || thread_info->finished) {
                thread_info->interruption_requested = true; // if thread finished this don't matter
                running_threads_queue.pop();
                running_threads_queue_mutex.unlock();
            } else {
                running_threads_queue_mutex.unlock();
                std::this_thread::sleep_for(chrono::milliseconds(1000));
            }
        }
    }
}


void SparqlServer::run(unsigned short port,
                       int number_of_workers,
                       chrono::seconds timeout)
{
    shutdown_server = false;

    // The io_context is required for all I/O
    asio::io_context io_context(number_of_workers);

    // Create and launch a listening port
    auto listener = std::make_shared<SPARQL_SERVER::Listener>(
        *this,
        io_context,
        asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port),
        timeout
    );
    listener->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait(
        [&](const beast::error_code& /*ec*/, int /*signal_number*/) {
            shutdown_server = true;
            std::cout << "\nShutting down server..." << std::endl;

            // Stop the `io_context`. This will cause `run()`
            // to return immediately, eventually destroying the
            // `io_context` and all of the sockets in it.
            io_context.stop();
        }
    );

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> threads;
    threads.reserve(number_of_workers);
    for (auto i = 0; i < number_of_workers; ++i) {
        thread_info_vec.push_back(ThreadInfo()); // TODO: pass i as param
        threads.emplace_back([&] {
            {
                std::lock_guard<std::mutex> guard(running_threads_queue_mutex);
                thread_id_map.insert({std::this_thread::get_id(), i});
            }
            io_context.run();
        });
    }
    cout << "SPARQL Server running on port " << port << endl;
    cout << "To terminate press CTRL-C" << endl;

    execute_timeouts();

    // If we get here, it means we got a SIGINT or SIGTERM

    // Block until all the threads exit
    for (auto& thread : threads) {
        thread.join();
    }
}