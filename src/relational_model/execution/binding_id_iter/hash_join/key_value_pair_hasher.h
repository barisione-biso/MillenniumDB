#ifndef RELATIONAL_MODEL__KEY_VALUE_PAIR_HASHER_H_
#define RELATIONAL_MODEL__KEY_VALUE_PAIR_HASHER_H_

#include <unordered_map>

#include "storage/index/hash/key_value_hash/key_value_hash.h"
#include "storage/index/hash/murmur3.h"

struct KeyValuePairHasher {
    // hash 1 simple xor
    uint64_t operator()(const std::vector<ObjectId>& key) const {
        auto val = key[0].id;
        for (std::size_t i = 1; i < key.size(); i++) {
            val = val ^ key[i].id;
        }
        return val;

        //uint64_t hash[2];
        //MurmurHash3_x64_128(key.data(), key.size() * sizeof(ObjectId), 0, hash);
        // transform next line with depth
        //uint64_t mask = KeyValueHash::MAX_BUCKETS - 1;  // (assuming MAX_BUCKETS is power of 2)

        //uint64_t bucket_number = hash[1] & mask;  // suffix = bucket_number in this case
        //return bucket_number;

        //return (hash[0] >> 9) & mask;
    }
    // xor vars, murmur[0]>> 9, murmur[1], postgress?
};


using SmallMultiMap = std::unordered_multimap<std::vector<ObjectId>, std::vector<ObjectId>, KeyValuePairHasher>;

#endif // RELATIONAL_MODEL__KEY_VALUE_PAIR_HASHER_H_
