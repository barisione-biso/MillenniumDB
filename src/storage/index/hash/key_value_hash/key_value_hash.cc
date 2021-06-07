#include "key_value_hash.h"

#include <cassert>
#include <iostream>

#include "base/ids/object_id.h"
#include "storage/file_manager.h"
#include "storage/buffer_manager.h"
#include "storage/index/hash/hash_functions/hash_function_wrapper.h"
#include "storage/index/hash/key_value_hash/key_value_hash_bucket.h"

using namespace std;

template class KeyValueHash<ObjectId, ObjectId>;

template <class K, class V>
KeyValueHash<K, V>::KeyValueHash(std::size_t key_size, std::size_t value_size) :
    key_size        (key_size),
    value_size      (value_size),
    max_tuples      ( (Page::PAGE_SIZE - sizeof(uint64_t)) / (key_size*sizeof(K) + value_size*sizeof(V)) ),
    buckets_file    ( file_manager.get_tmp_file_id())
    { }


template <class K, class V>
KeyValueHash<K, V>::~KeyValueHash() {
    current_buckets_pages.clear();
    file_manager.remove_tmp(buckets_file);
}


template <class K, class V>
void KeyValueHash<K, V>::begin(uint_fast32_t initial_depth) {
    depth = initial_depth;
    const uint_fast32_t number_of_buckets = 1 << depth; // 2^depth

    buckets_sizes.reserve(number_of_buckets);
    current_buckets_pages.reserve(number_of_buckets);
    buckets_page_numbers.reserve(number_of_buckets);

    last_page_number = -1;  // overflow to start with 0 in loop
    for (uint_fast32_t i = 0; i < number_of_buckets; ++i) {
        current_buckets_pages.push_back(
            std::make_unique<KeyValueHashBucket<K, V>>(buckets_file, ++last_page_number, key_size, value_size)
        );
        // pages could be dirty
        *(current_buckets_pages[i]->tuple_count) = 0;
        current_buckets_pages[i]->page.make_dirty();
        buckets_sizes.push_back(0);

        std::vector<uint_fast32_t> vec;
        vec.push_back(last_page_number);
        buckets_page_numbers.push_back(vec);
    }
    tuples_count = 0;
}


template <class K, class V>
void KeyValueHash<K, V>::reset() {
    const uint_fast32_t number_of_buckets = 1 << depth; // 2^depth
    last_page_number = -1;
    for (uint_fast32_t i = 0; i < number_of_buckets; ++i) {
        current_buckets_pages[i] = std::make_unique<KeyValueHashBucket<K, V>>(
                buckets_file, ++last_page_number, key_size, value_size
            );

        *(current_buckets_pages[i]->tuple_count) = 0;
        current_buckets_pages[i]->page.make_dirty();
        buckets_sizes[i] = 0;

        buckets_page_numbers[i].clear();
        buckets_page_numbers[i].push_back(last_page_number);
    }
    while (!available_pages.empty()) {
        available_pages.pop();
    }
    tuples_count = 0;
}


template <class K, class V>
void KeyValueHash<K, V>::insert(const std::vector<K>& key, const std::vector<V>& value) {
    assert(key.size() == key_size);

    uint64_t hash_ = hash_function_wrapper(key.data(), key_size);
    // assuming enough memory in each bucket
    uint64_t mask = (1 << depth) - 1;  // last depth bits
    uint64_t bucket_number = hash_ & mask;  // suffix = bucket_number in this case

    while (current_buckets_pages[bucket_number]->get_tuple_count() >= max_tuples) {
        if (tuples_count >= get_split_treshold()) {  // split treshold should depend on depth or could be infinite
            split();
            mask = (1 << depth) - 1;
            bucket_number = hash_ & mask;
            continue;
        }
        // get page number for new bucket page
        uint_fast32_t new_page_number;
        if (available_pages.size() > 0) {
            new_page_number = available_pages.front();
            available_pages.pop();
        } else {
            new_page_number = ++last_page_number;
        }
        current_buckets_pages[bucket_number] = std::make_unique<KeyValueHashBucket<K, V>>(
            buckets_file, new_page_number, key_size, value_size // +1 before assign
        );
        *(current_buckets_pages[bucket_number]->tuple_count) = 0;
        current_buckets_pages[bucket_number]->page.make_dirty();
        buckets_page_numbers[bucket_number].push_back(new_page_number);
    }
    current_buckets_pages[bucket_number]->insert(key, value);
    buckets_sizes[bucket_number]++;
    tuples_count++;
}


template <class K, class V>
pair<vector<K>, vector<V>> KeyValueHash<K, V>::get_pair(uint_fast32_t bucket_number,uint_fast32_t current_pos) {
    assert(current_pos <= buckets_sizes[bucket_number]);
    uint32_t bucket_page_number = current_pos / max_tuples;
    uint32_t real_page_number = buckets_page_numbers[bucket_number][bucket_page_number];
    if (current_buckets_pages[bucket_number]->page.get_page_number() != real_page_number) {
        current_buckets_pages[bucket_number] = std::make_unique<KeyValueHashBucket<K, V>>(
            buckets_file, real_page_number, key_size, value_size
        );
    }
    uint32_t page_pos = current_pos % max_tuples;
    return current_buckets_pages[bucket_number]->get_pair(page_pos);
}


template <class K, class V>
void KeyValueHash<K, V>::split() {
    const uint_fast32_t previous_number_of_buckets = 1 << depth++;
    const uint_fast32_t number_of_buckets = 1 << depth; // 2^new_depth
    buckets_sizes.reserve(number_of_buckets);
    current_buckets_pages.reserve(number_of_buckets);
    buckets_page_numbers.reserve(number_of_buckets);

    for (uint_fast32_t bucket_number = 0; bucket_number < previous_number_of_buckets; bucket_number++) {
        uint_fast32_t new_bucket_number = previous_number_of_buckets + bucket_number;
        uint_fast32_t new_page_number;
        if (available_pages.size() > 0) {
            new_page_number = available_pages.front();
            available_pages.pop();
        } else {
            new_page_number = ++last_page_number;
        }
        std::vector<uint_fast32_t> vec;
        vec.push_back(new_page_number);
        buckets_page_numbers.push_back(vec);

        // initialize new bucket
        current_buckets_pages.push_back(
            std::make_unique<KeyValueHashBucket<K, V>>(buckets_file, new_page_number, key_size, value_size)
        );
        *(current_buckets_pages[new_bucket_number]->tuple_count) = 0;
        current_buckets_pages[new_bucket_number]->page.make_dirty();
        buckets_sizes.push_back(0);

        // we need an auxiliar bucket for writing tuples, because we are reading with current_bucket_pages
        std::unique_ptr<KeyValueHashBucket<K, V>> aux_bucket; //save tuples in this bucket if not full
        aux_bucket = std::make_unique<KeyValueHashBucket<K, V>>(
            buckets_file, buckets_page_numbers[bucket_number][0], key_size, value_size
        );
        uint_fast32_t aux_bucket_pos = 0;  // pos in current_buckets_pages
        uint_fast32_t aux_tuple_count = 0;  // total insertions using aux bucket
        for (uint_fast32_t i = 0; i < buckets_page_numbers[bucket_number].size(); i++) {
            auto real_page_number = buckets_page_numbers[bucket_number][i];
            if (current_buckets_pages[bucket_number]->page.get_page_number() != real_page_number) {
                current_buckets_pages[bucket_number] = std::make_unique<KeyValueHashBucket<K, V>>(
                    buckets_file, real_page_number, key_size, value_size
                );
            }
            uint_fast32_t bucket_page_tuples = current_buckets_pages[bucket_number]->get_tuple_count();
            for (uint_fast32_t tuple_number = 0; tuple_number < bucket_page_tuples; tuple_number++) {
                auto pair = current_buckets_pages[bucket_number]->get_pair(tuple_number);
                uint64_t hash_ = hash_function_wrapper(pair.first.data(), key_size);
                uint64_t mask = number_of_buckets - 1;  // last (depth) bits
                uint64_t insert_bucket_number = hash_ & mask;  // suffix = bucket_number
                assert(insert_bucket_number == bucket_number || insert_bucket_number == new_bucket_number);

                if (insert_bucket_number == bucket_number) { // insert to aux (old bucket page)
                    if (aux_tuple_count >= max_tuples) {
                        // save values of aux
                        aux_bucket->set_tuple_count(aux_tuple_count);
                        aux_bucket->page.make_dirty();
                        // set next aux
                        aux_tuple_count = 0;
                        aux_bucket = std::make_unique<KeyValueHashBucket<K, V>>(
                            buckets_file, buckets_page_numbers[bucket_number][++aux_bucket_pos],
                            key_size, value_size
                        );
                        assert(aux_bucket_pos <= i);
                    }
                    aux_bucket->insert_in_pos(pair.first, pair.second, aux_tuple_count);
                    aux_tuple_count++;
                }
                else { // insert to new bucket
                    buckets_sizes[insert_bucket_number]++;
                    buckets_sizes[bucket_number]--;
                    if (current_buckets_pages[insert_bucket_number]->get_tuple_count() >= max_tuples) {
                        if (available_pages.size() > 0) {
                            new_page_number = available_pages.front();
                            available_pages.pop();
                        } else {
                            new_page_number = ++last_page_number;
                        }
                        buckets_page_numbers[insert_bucket_number].push_back(new_page_number);

                        current_buckets_pages[insert_bucket_number] = std::make_unique<KeyValueHashBucket<K, V>>(
                            buckets_file, new_page_number, key_size, value_size
                        );
                        *(current_buckets_pages[insert_bucket_number]->tuple_count) = 0;
                        current_buckets_pages[insert_bucket_number]->page.make_dirty();
                    }
                    current_buckets_pages[insert_bucket_number]->insert(pair.first, pair.second);
                }
            }
        }
        // save current aux
        aux_bucket->set_tuple_count(aux_tuple_count);
        aux_bucket->page.make_dirty();
        //set current bucket page to aux page
        current_buckets_pages[bucket_number] = std::make_unique<KeyValueHashBucket<K, V>>(
            buckets_file, buckets_page_numbers[bucket_number][aux_bucket_pos],
            key_size, value_size
        );
        // add rest pages to available pages
        aux_bucket_pos++;
        auto n = 0;
        while (aux_bucket_pos < buckets_page_numbers[bucket_number].size()) {
            auto real_page_number = buckets_page_numbers[bucket_number][aux_bucket_pos++];
            available_pages.push(real_page_number);
            n += 1;
        }
        // update buckets_page_numbers[bucket_number]
        buckets_page_numbers[bucket_number].resize(buckets_page_numbers[bucket_number].size()-n);
    }
}


template <class K, class V>
uint64_t KeyValueHash<K, V>::get_bucket(const std::vector<K>& key) const {
    assert(key.size() == key_size);

    uint64_t hash_ = hash_function_wrapper(key.data(), key_size);
    uint64_t mask = (1 << depth) - 1;
    uint64_t bucket_number = hash_ & mask;  // suffix = bucket_number in this case
    return bucket_number;
}
