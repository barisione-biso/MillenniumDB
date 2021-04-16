#include "multi_map.h"

#include <cassert>
#include <iostream>

#include "base/ids/object_id.h"
#include "storage/file_manager.h"
#include "storage/buffer_manager.h"
#include "storage/index/hash_table/murmur3.h"
#include "storage/index/multi_map/multi_bucket.h"


MultiMap::MultiMap(std::size_t key_size, std::size_t value_size, char tmp_char) :
    key_size        (key_size),
    value_size      (value_size),
    MAX_TUPLES      ( (Page::PAGE_SIZE - sizeof(uint32_t*)) / ((key_size + value_size)*sizeof(ObjectId)) )
    {
        buckets_files.reserve(MAX_BUCKETS);
        buckets_sizes.reserve(MAX_BUCKETS);
        last_buckets_pages.reserve(MAX_BUCKETS);
        current_buckets_pages.reserve(MAX_BUCKETS);
        for (uint_fast32_t i = 0; i < MAX_BUCKETS; ++i) {
            // TODO: use tmp files
            buckets_files.push_back(file_manager.get_file_id("tmp_multibucket_" + std::to_string(tmp_char) + "_" + std::to_string(i)));
            last_buckets_pages.push_back(
                std::make_unique<MultiBucket>(buffer_manager.get_page(buckets_files[i], 0), key_size, value_size)
            );
            current_buckets_pages.push_back(
                std::make_unique<MultiBucket>(buffer_manager.get_page(buckets_files[i], 0), key_size, value_size)
            );
            *(last_buckets_pages[i]->tuple_count) = 0;
            last_buckets_pages[i]->page.make_dirty();
            buckets_sizes.push_back(0);
        }
    }


MultiMap::~MultiMap() {
    for (uint_fast32_t i = 0; i < MAX_BUCKETS; ++i) {
        // TODO: use tmp files
        file_manager.remove(buckets_files[i]);
    }
}


void MultiMap::insert(std::vector<ObjectId> key, std::vector<ObjectId> value) {
    assert(key.size() == key_size);

    uint64_t hash[2];
    MurmurHash3_x64_128(key.data(), key_size * sizeof(ObjectId), 0, hash);
    // assuming enough memory in each bucket
    uint64_t mask = MAX_BUCKETS - 1;  // (assuming MAX_BUCKETS is power of 2)
    uint64_t bucket_number = hash[0] & mask;  // suffix = bucket_number in this case

    if (last_buckets_pages[bucket_number]->get_tuple_count() >= MAX_TUPLES) {
        auto new_page_number = last_buckets_pages[bucket_number]->page.get_page_number() + 1;
        last_buckets_pages[bucket_number] = std::make_unique<MultiBucket>(
            buffer_manager.get_page(buckets_files[bucket_number], new_page_number), key_size, value_size
        );
    }
    last_buckets_pages[bucket_number]->insert(MultiPair(key, value));
    buckets_sizes[bucket_number]++;
}


uint_fast32_t MultiMap::get_bucket_size(uint_fast32_t bucket_number) {
    return buckets_sizes[bucket_number];
}


MultiPair MultiMap::get_pair(uint_fast32_t bucket_number, uint_fast32_t current_pos) {
    assert(current_pos <= buckets_sizes[bucket_number]);
    uint32_t page_number = current_pos / MAX_TUPLES;
    if (current_buckets_pages[bucket_number]->page.get_page_number() != page_number) {
        current_buckets_pages[bucket_number] = std::make_unique<MultiBucket>(
            buffer_manager.get_page(buckets_files[bucket_number], page_number), key_size, value_size
        );
    }
    uint32_t page_pos = current_pos % MAX_TUPLES;
    return current_buckets_pages[bucket_number]->get_pair(page_pos);
}
