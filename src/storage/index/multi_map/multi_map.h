#ifndef STORAGE__MULTI_MAP_H_
#define STORAGE__MULTI_MAP_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <utility>

#include "base/ids/var_id.h"
#include "storage/file_id.h"
#include "storage/index/object_file/object_file.h"

using MultiPair = std::pair<std::vector<ObjectId>, std::vector<ObjectId>>;
using MultiBucket = std::vector<MultiPair>;
//template <class T>
class MultiMap {
public:
    static constexpr uint_fast32_t MAX_BUCKETS = 512;

    MultiMap(std::size_t _key_size, std::size_t _value_size);
    ~MultiMap();

    void insert(std::vector<ObjectId> key, std::vector<ObjectId> value);
    std::uint_fast32_t bucket_size(std::uint_fast32_t current_bucket);
    MultiPair& get_pair(std::uint_fast32_t current_bucket, std::uint_fast32_t current_pos);

private:
    const std::size_t key_size;
    const std::size_t value_size;
    std::vector<MultiBucket> buckets;

    //const FileId buckets_file_id;
};

#endif // STORAGE__MULTI_MAP_H_
