#pragma once

#include <chrono>
#include <ostream>
#include <queue>
#include <map>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>

#include "base/thread/thread_info.h"

namespace SPARQL_SERVER {

class SparqlServer {
public:
    static constexpr int DEFAULT_PORT = 8080;

    std::queue<ThreadInfo*> running_threads_queue;
    std::map<std::thread::id, size_t> thread_id_map;
    std::vector<ThreadInfo> thread_info_vec;
    std::mutex running_threads_queue_mutex;
    bool shutdown_server;

    void execute_timeouts();

    void run(unsigned short port,
             int worker_threads,
             std::chrono::seconds timeout);
};

} // namespace SPARQL_SERVER