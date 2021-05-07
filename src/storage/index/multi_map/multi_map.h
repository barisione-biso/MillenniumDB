#ifndef STORAGE__MULTI_MAP_H_
#define STORAGE__MULTI_MAP_H_

#include <cstdint>
#include <queue>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/ids/var_id.h"
#include "storage/file_id.h"
#include "storage/index/object_file/object_file.h"
#include "storage/index/multi_map/multi_bucket.h"

using MultiPair = std::pair<std::vector<ObjectId>, std::vector<ObjectId>>;

class MultiMap {
public:
    static uint32_t instances_count; // TODO: remove when temp files are working
    static constexpr uint_fast32_t DEFAULT_INITIAL_DEPTH = 5;  // TODO: test best initial depth for each query

    MultiMap(std::size_t key_size, std::size_t value_size);
    ~MultiMap();

    void begin(uint_fast32_t initial_depth = DEFAULT_INITIAL_DEPTH);
    void reset();
    void insert(const std::vector<ObjectId>& key, const std::vector<ObjectId>& value);

    inline uint_fast32_t get_bucket_size(uint_fast32_t bucket_number) const {
        return buckets_sizes[bucket_number];
    }
    MultiPair get_pair(uint_fast32_t current_bucket, uint_fast32_t current_pos);

    void split(); // split directory and redistribute every bucket
    inline uint_fast32_t get_depth() const noexcept { return depth; }
    uint_fast32_t get_bucket(const std::vector<ObjectId>& key) const;

private:
    const std::size_t  key_size;
    const std::size_t  value_size;
    const uint32_t     max_tuples;

    uint_fast32_t      last_page_number;
    const FileId       buckets_file;

    uint64_t tuples_count;
    uint_fast32_t depth;

    std::vector<std::vector<uint_fast32_t>>   buckets_page_numbers;
    std::vector<uint_fast32_t>                buckets_sizes;

    std::queue<uint_fast32_t> available_pages;

    // NOTE: if we don't use last_bucket_pages we are supposed to insert everything and then read (can't do both at same time)
    std::vector<std::unique_ptr<MultiBucket>> current_buckets_pages;

    inline uint_fast32_t get_split_treshold() const noexcept { return max_tuples * (1 << depth); } // TODO: ajustar parámetros
};

#endif // STORAGE__MULTI_MAP_H_
