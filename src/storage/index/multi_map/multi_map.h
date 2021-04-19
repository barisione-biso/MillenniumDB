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
#include "storage/index/multi_map/multi_bucket.h"

using MultiPair = std::pair<std::vector<ObjectId>, std::vector<ObjectId>>;


//template <class T>
class MultiMap {
public:
    static uint32_t instances_count;
    static constexpr uint_fast32_t MAX_BUCKETS = 512;

    MultiMap(std::size_t _key_size, std::size_t _value_size);
    ~MultiMap();

    void begin();
    void insert(std::vector<ObjectId> key, std::vector<ObjectId> value);
    uint_fast32_t get_bucket_size(uint_fast32_t current_bucket);
    MultiPair get_pair(uint_fast32_t current_bucket, uint_fast32_t current_pos);

private:
    const std::size_t key_size;
    const std::size_t value_size;
    const uint32_t MAX_TUPLES;

    std::vector<FileId>                       buckets_files;
    std::vector<uint_fast32_t>                buckets_sizes;
    std::vector<std::unique_ptr<MultiBucket>> last_buckets_pages;
    std::vector<std::unique_ptr<MultiBucket>> current_buckets_pages;  // for reading
};

#endif // STORAGE__MULTI_MAP_H_
