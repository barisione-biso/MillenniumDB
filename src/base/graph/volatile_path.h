#ifndef BASE__VOLATILE_PATH_H_
#define BASE__VOLATILE_PATH_H_

#include <cstdint>
#include <cstring>
#include <ostream>
#include "base/graph/path_printer.h"

/*
TODO: Explain class
*/

class VolatilePath {
public:
    static PathPrinter* path_printer;
    uint64_t path_id;

    VolatilePath(uint64_t path_id) :
        path_id (path_id) { }

    ~VolatilePath() = default;

    inline bool operator==(const VolatilePath& rhs) const noexcept {
        return this->path_id == rhs.path_id;
    }

    inline bool operator!=(const VolatilePath& rhs) const noexcept {
        return this->path_id != rhs.path_id;
    }

    inline bool operator<=(const VolatilePath& rhs) const noexcept {
        return this->path_id <= rhs.path_id;
    }

    inline bool operator>=(const VolatilePath& rhs) const noexcept {
        return this->path_id >= rhs.path_id;
    }

    inline bool operator<(const VolatilePath& rhs) const noexcept {
        return this->path_id < rhs.path_id;
    }

    inline bool operator>(const VolatilePath& rhs) const noexcept {
        return this->path_id > rhs.path_id;
    }
};

#endif // BASE__VOLATILE_PATH_H_
