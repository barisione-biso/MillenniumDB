#pragma once

#include <chrono>

struct ThreadInfo {
    bool interruption_requested = false;
    bool finished               = false; // TODO: delete?

    size_t worker_index;

    std::chrono::system_clock::time_point timeout;
    std::chrono::system_clock::time_point time_start;
};
