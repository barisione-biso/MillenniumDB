#ifndef STORAGE__FILE_ID_H_
#define STORAGE__FILE_ID_H_

#include <cstdint>

class FileId {
public:
    static const uint_fast32_t UNASSIGNED = -1;

    uint_fast32_t id;

    FileId(uint_fast32_t id)
        : id(id) {}

    bool operator<(const FileId other) const {
        return this->id < other.id;
    }
};

#endif //STORAGE__FILE_ID_H_
