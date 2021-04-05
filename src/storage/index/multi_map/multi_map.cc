#include "multi_map.h"

#include <cassert>

#include "base/ids/object_id.h"
#include "storage/file_manager.h"
#include "storage/index/hash_table/murmur3.h"


MultiMap::MultiMap(std::size_t _key_size, std::size_t _value_size) :
    key_size   (_key_size),
    value_size (_value_size)
    {
        for (uint_fast32_t i = 0; i < MAX_BUCKETS; i++) {
            buckets.push_back(MultiBucket());
        }
    }


MultiMap::~MultiMap() {
    //file_manager.remove(buckets_file_id);
}


void MultiMap::insert(std::vector<ObjectId> key, std::vector<ObjectId> value) {
    assert(key.size() == key_size);

    uint64_t hash[2];
    MurmurHash3_x64_128(key.data(), key_size * sizeof(ObjectId), 0, hash);
    // assuming enough memory in each bucket
    uint64_t mask = MAX_BUCKETS - 1;  // (assuming MAX_BUCKETS is power of 2)
    uint64_t suffix = hash[0] & mask;  // suffix = bucket_number in this case
    buckets[suffix].push_back(std::make_pair(key, value));
}


std::uint_fast32_t MultiMap::bucket_size(std::uint_fast32_t current_bucket) {
    return buckets[current_bucket].size();
}


MultiPair& MultiMap::get_pair(std::uint_fast32_t current_bucket, std::uint_fast32_t current_pos) {
    return buckets[current_bucket][current_pos];
}
