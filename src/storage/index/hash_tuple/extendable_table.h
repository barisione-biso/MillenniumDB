#ifndef STORAGE__EXTENDABLE_TABLE_H_
#define STORAGE__EXTENDABLE_TABLE_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "base/ids/var_id.h"
#include "storage/file_id.h"
#include "storage/index/object_file/object_file.h"

template <class T>
class ExtendableTable {
public:
    static constexpr auto DEFAULT_GLOBAL_DEPTH = 10;

    ExtendableTable(std::size_t vars_size);
    ~ExtendableTable();

    bool is_in(std::vector<T> tuple);
    bool is_in_or_insert(std::vector<T> tuple);

private:
    uint_fast8_t global_depth = DEFAULT_GLOBAL_DEPTH;
    std::size_t tuple_size;

    const FileId buckets_file_id;

    // array of size 2^global_depth
    uint_fast32_t* dir;

    void duplicate_dirs();
};

#endif // STORAGE__EXTENDABLE_TABLE_H_