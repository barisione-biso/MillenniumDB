#include "multi_map.h"

#include <cassert>
#include <iostream>

#include "base/ids/object_id.h"
#include "storage/file_manager.h"
#include "storage/buffer_manager.h"
#include "storage/index/hash_table/murmur3.h"
#include "storage/index/multi_map/multi_bucket.h"


uint32_t MultiMap::instances_count = 0;


MultiMap::MultiMap(std::size_t key_size, std::size_t value_size) :
    key_size        (key_size),
    value_size      (value_size),
    MAX_TUPLES      ( (Page::PAGE_SIZE - sizeof(uint32_t)) / ((key_size + value_size)*sizeof(ObjectId)) ),
    buckets_file    ( file_manager.get_file_id("tmp_multibucket_" + std::to_string(++instances_count)) )
    { }


MultiMap::~MultiMap() {
    current_buckets_pages.clear();
    // TODO: use tmp files
    file_manager.remove(buckets_file);
}


void MultiMap::begin() {
    buckets_sizes.reserve(MAX_BUCKETS);
    current_buckets_pages.reserve(MAX_BUCKETS);
    buckets_page_numbers.reserve(MAX_BUCKETS);

    last_page_number = -1;  // overflow to start with 0 in loop
    for (uint_fast32_t i = 0; i < MAX_BUCKETS; ++i) {
        current_buckets_pages.push_back(
            std::make_unique<MultiBucket>(buffer_manager.get_page(buckets_file, ++last_page_number), key_size, value_size)
        );

        *(current_buckets_pages[i]->tuple_count) = 0;
        current_buckets_pages[i]->page.make_dirty();
        buckets_sizes.push_back(0);

        std::vector<uint_fast32_t> vec;
        vec.push_back(last_page_number);
        buckets_page_numbers.push_back(vec);
    }
}


void MultiMap::reset() {
    last_page_number = -1;
    for (uint_fast32_t i = 0; i < MAX_BUCKETS; ++i) {
        current_buckets_pages[i] = std::make_unique<MultiBucket>(
                buffer_manager.get_page(buckets_file, ++last_page_number), key_size, value_size
            );

        *(current_buckets_pages[i]->tuple_count) = 0;
        current_buckets_pages[i]->page.make_dirty();
        buckets_sizes[i] = 0;

        buckets_page_numbers[i].clear();
        buckets_page_numbers[i].push_back(last_page_number);
    }
}


void MultiMap::insert(std::vector<ObjectId> key, std::vector<ObjectId> value) {
    assert(key.size() == key_size);

    uint64_t hash[2];
    MurmurHash3_x64_128(key.data(), key_size * sizeof(ObjectId), 0, hash);
    // assuming enough memory in each bucket
    uint64_t mask = MAX_BUCKETS - 1;  // (assuming MAX_BUCKETS is power of 2)
    uint64_t bucket_number = hash[0] & mask;  // suffix = bucket_number in this case

    // if want to insert and read change current for last
    if (current_buckets_pages[bucket_number]->get_tuple_count() >= MAX_TUPLES) {
        current_buckets_pages[bucket_number] = std::make_unique<MultiBucket>(
            buffer_manager.get_page(buckets_file, ++last_page_number), key_size, value_size // +1 before assign
        );
        *(current_buckets_pages[bucket_number]->tuple_count) = 0;
        current_buckets_pages[bucket_number]->page.make_dirty();
        buckets_page_numbers[bucket_number].push_back(last_page_number);
    }
    current_buckets_pages[bucket_number]->insert(MultiPair(key, value));
    buckets_sizes[bucket_number]++;
}


uint_fast32_t MultiMap::get_bucket_size(uint_fast32_t bucket_number) {
    return buckets_sizes[bucket_number];
}


MultiPair MultiMap::get_pair(uint_fast32_t bucket_number, uint_fast32_t current_pos) {
    assert(current_pos <= buckets_sizes[bucket_number]);
    assert(bucket_number < MAX_BUCKETS);
    uint32_t page_number = current_pos / MAX_TUPLES;
    uint32_t real_page_number = buckets_page_numbers[bucket_number][page_number];
    if (current_buckets_pages[bucket_number]->page.get_page_number() != real_page_number) {
        current_buckets_pages[bucket_number] = std::make_unique<MultiBucket>(
            buffer_manager.get_page(buckets_file, real_page_number), key_size, value_size
        );
    }
    uint32_t page_pos = current_pos % MAX_TUPLES;
    return current_buckets_pages[bucket_number]->get_pair(page_pos);
}
