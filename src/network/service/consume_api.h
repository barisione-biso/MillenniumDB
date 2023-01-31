#pragma once

#include <string>

std::string consume(std::string const& host, std::string const& port, std::string const& target,
                    std::string const& format, bool const& https);