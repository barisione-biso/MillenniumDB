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

    // TODO: este for solo dejaría vectores vacíos en buckets_page_numbers
    // luego hay que tener cuidado al ver buckets_page_numbers[i] porque podría estar vacío
    for (uint_fast32_t i = 0; i < number_of_buckets; ++i) {
        current_buckets_pages.push_back(
            std::make_unique<KeyValueHashBucket<K, V>>(buckets_file, ++last_page_number, key_size, value_size, max_tuples)
        );
        // pages could be dirty
        *(current_buckets_pages[i]->tuple_count) = 0;
        current_buckets_pages[i]->page.make_dirty();
        buckets_sizes.push_back(0);

        std::vector<uint_fast32_t> vec;
        vec.push_back(last_page_number);
        buckets_page_numbers.push_back(vec); // TODO: initial size cero
    }
    tuples_count = 0;
}


template <class K, class V>
void KeyValueHash<K, V>::reset(uint_fast32_t new_depth) {
    depth = new_depth;
    const uint_fast32_t number_of_buckets = 1 << depth; // 2^depth
    last_page_number = -1;

    buckets_sizes.resize(number_of_buckets);
    current_buckets_pages.resize(number_of_buckets);
    buckets_page_numbers.resize(number_of_buckets);

    // TODO: actualizar para que sea consistente con cambios en el begin
    for (uint_fast32_t i = 0; i < number_of_buckets; ++i) {
        current_buckets_pages[i] = std::make_unique<KeyValueHashBucket<K, V>>(
                buckets_file, ++last_page_number, key_size, value_size, max_tuples
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

    // TODO: cambiar current_buckets_pages[bucket_number] por variable local current_bucket_page = make_unique ...
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
            buckets_file, new_page_number, key_size, value_size, max_tuples // +1 before assign
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
pair<vector<K>, vector<V>> KeyValueHash<K, V>::get_pair(uint_fast32_t bucket_number, uint_fast32_t current_pos) {
    assert(current_pos <= buckets_sizes[bucket_number]);
    uint32_t bucket_page_number = current_pos / max_tuples;
    uint32_t real_page_number = buckets_page_numbers[bucket_number][bucket_page_number];

    // TODO: borrar if, siempre se hara el make_unique, pero asignando una variable local
    if (current_buckets_pages[bucket_number]->page.get_page_number() != real_page_number) {
        current_buckets_pages[bucket_number] = std::make_unique<KeyValueHashBucket<K, V>>(
            buckets_file, real_page_number, key_size, value_size, max_tuples
        );
    }
    uint32_t page_pos = current_pos % max_tuples;
    return current_buckets_pages[bucket_number]->get_pair(page_pos); // TODO: reemplazar por variable local
}


template <class K, class V>
K* KeyValueHash<K, V>::get_key(uint_fast32_t bucket_number,uint_fast32_t current_pos) {
    assert(current_pos <= buckets_sizes[bucket_number]);
    uint32_t bucket_page_number = current_pos / max_tuples;
    uint32_t real_page_number = buckets_page_numbers[bucket_number][bucket_page_number];
    // TODO: borrar if, siempre se hara el make_unique, pero asignando una variable local
    if (current_buckets_pages[bucket_number]->page.get_page_number() != real_page_number) {
        current_buckets_pages[bucket_number] = std::make_unique<KeyValueHashBucket<K, V>>(
            buckets_file, real_page_number, key_size, value_size, max_tuples
        );
    }
    uint32_t page_pos = current_pos % max_tuples;
    return current_buckets_pages[bucket_number]->get_key(page_pos); // TODO: reemplazar por variable local
}


template <class K, class V>
V* KeyValueHash<K, V>::get_value(uint_fast32_t bucket_number,uint_fast32_t current_pos) {
    assert(current_pos <= buckets_sizes[bucket_number]);
    uint32_t bucket_page_number = current_pos / max_tuples;
    uint32_t real_page_number = buckets_page_numbers[bucket_number][bucket_page_number];
    // TODO: borrar if, siempre se hara el make_unique, pero asignando una variable local
    if (current_buckets_pages[bucket_number]->page.get_page_number() != real_page_number) {
        current_buckets_pages[bucket_number] = std::make_unique<KeyValueHashBucket<K, V>>(
            buckets_file, real_page_number, key_size, value_size, max_tuples
        );
    }
    uint32_t page_pos = current_pos % max_tuples;
    return current_buckets_pages[bucket_number]->get_value(page_pos); // TODO: reemplazar por variable local
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
            std::make_unique<KeyValueHashBucket<K, V>>(buckets_file, new_page_number, key_size, value_size, max_tuples)
        );
        *(current_buckets_pages[new_bucket_number]->tuple_count) = 0;
        current_buckets_pages[new_bucket_number]->page.make_dirty();
        buckets_sizes.push_back(0);

        // we need an auxiliar bucket for writing tuples, because we are reading with current_buckets_pages
        std::unique_ptr<KeyValueHashBucket<K, V>> aux_bucket; //save tuples in this bucket if not full
        aux_bucket = std::make_unique<KeyValueHashBucket<K, V>>(
            buckets_file, buckets_page_numbers[bucket_number][0], key_size, value_size, max_tuples
        );
        uint_fast32_t aux_bucket_pos = 0;  // pos in current_buckets_pages
        uint_fast32_t aux_tuple_count = 0;  // total insertions using aux bucket
        for (uint_fast32_t i = 0; i < buckets_page_numbers[bucket_number].size(); i++) {
            auto real_page_number = buckets_page_numbers[bucket_number][i];
            if (current_buckets_pages[bucket_number]->page.get_page_number() != real_page_number) {
                current_buckets_pages[bucket_number] = std::make_unique<KeyValueHashBucket<K, V>>(
                    buckets_file, real_page_number, key_size, value_size, max_tuples
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
                            key_size, value_size, max_tuples
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
                            buckets_file, new_page_number, key_size, value_size, max_tuples
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
            key_size, value_size, max_tuples
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


template <class K, class V>
void KeyValueHash<K, V>::sort_buckets(){
    const uint_fast32_t number_of_buckets = 1 << depth; // 2^depth
    for (uint_fast32_t bucket_number=0; bucket_number<number_of_buckets; bucket_number++) {
        sort_bucket(bucket_number);
    }
}


template <class K, class V>
void KeyValueHash<K, V>::check_order() {
    const uint_fast32_t number_of_buckets = 1 << depth; // 2^depth
    std::unique_ptr<KeyValueHashBucket<K, V>> current_bucket_page;
    K* last_key;
    K* current_key;
    for (uint_fast32_t bucket_number=0; bucket_number<number_of_buckets; bucket_number++) {
        uint32_t current_pos = 0;
        if (buckets_sizes[bucket_number] > 0) {
            last_key = get_key(bucket_number, current_pos);
        }
        current_pos++;
        while (current_pos < buckets_sizes[bucket_number]) {
            current_key = get_key(bucket_number, current_pos);
            bool smaller = false;
            for (uint_fast16_t i = 0; i < key_size; i++) {
                if (current_key[i] < last_key[i]) {
                    smaller = true;
                    break;
                }
                else if (current_key[i] > last_key[i]) {
                    break;
                }
            }
            if (smaller) {
                cout << "bucket: " << bucket_number << ", pos: " << current_pos << "not ordered\n";
            }
            last_key = current_key;
            current_pos++;
        }
    }
}


template <class K, class V>
bool KeyValueHash<K, V>::find_first(const std::vector<K>& current_key, uint_fast32_t bucket_number,
                                    uint_fast32_t* current_bucket_pos) {
    if (buckets_sizes[bucket_number] == 0) {
        return false;
    }
    // initial
    uint_fast32_t curr_max = buckets_sizes[bucket_number] - 1;
    uint_fast32_t curr_min = 0;
    uint_fast32_t curr_pos = (curr_min + curr_max)/2;

    // set key
    K* key_ptr = get_key(bucket_number, curr_pos);

    while (curr_min <= curr_max) {
        auto key_ptr_smaller = false;
        for (uint_fast32_t i=0; i<key_size; i++) {
            if (key_ptr[i] < current_key[i]) {
                key_ptr_smaller = true;
                break;
            }
            else if (key_ptr[i] > current_key[i]) {
                break;
            }
        }
        if (key_ptr_smaller) {
            curr_min = curr_pos + 1;
        }
        else { //equal or bigger
            curr_max = curr_pos;
        }
        // set new curr_pos
        curr_pos = (curr_min + curr_max)/2;
        key_ptr = get_key(bucket_number, curr_pos);
        if (curr_pos == curr_min && curr_pos == curr_max) {
            for (uint_fast32_t i=0; i<key_size; i++) {
                if (key_ptr[i] != current_key[i]) {
                    return false;
                }
            }
            *current_bucket_pos = curr_pos;
            return true;
        }
    }
    return false;
}


template <class K, class V>
void KeyValueHash<K, V>::sort_bucket(uint_fast32_t bucket_number){
    std::unique_ptr<KeyValueHashBucket<K, V>> current_bucket_page; // aux bucket
    std::unique_ptr<KeyValueHashBucket<K, V>> current_left_page;
    std::unique_ptr<KeyValueHashBucket<K, V>> current_right_page;
    K* left_key;
    K* right_key;
    V* left_value;
    V* right_value;
    // sort each page
    for (auto real_page_number: buckets_page_numbers[bucket_number]) {
        current_bucket_page = std::make_unique<KeyValueHashBucket<K, V>>(
            buckets_file, real_page_number, key_size, value_size, max_tuples
        );
        current_bucket_page->sort();
    }
    // merge
    uint_fast32_t merge_size = 1;
    while (buckets_page_numbers[bucket_number].size() > merge_size) {
        // set aux buckets
        vector<uint_fast32_t> new_page_numbers;
        new_page_numbers.reserve(buckets_page_numbers[bucket_number].size());
        uint_fast32_t new_page_number;
        if (available_pages.size() > 0) {
            new_page_number = available_pages.front();
            available_pages.pop();
        } else {
            new_page_number = ++last_page_number;
        }
        new_page_numbers.push_back(new_page_number);
        current_bucket_page = std::make_unique<KeyValueHashBucket<K, V>>(
            buckets_file, new_page_number,
            key_size, value_size, max_tuples
        );
        *(current_bucket_page->tuple_count) = 0;
        current_bucket_page->page.make_dirty();
        for (uint_fast32_t step = 0; step < buckets_page_numbers[bucket_number].size(); step+=(2* merge_size)) {
            uint_fast32_t left_page_number = step;
            uint_fast32_t right_page_number = step + merge_size;
            if (right_page_number >= buckets_page_numbers[bucket_number].size()) {
                while (left_page_number < buckets_page_numbers[bucket_number].size()) {
                    new_page_numbers.push_back(buckets_page_numbers[bucket_number][left_page_number]);
                    left_page_number++;
                }
                break;
            }
            current_left_page = std::make_unique<KeyValueHashBucket<K, V>>(
                buckets_file, buckets_page_numbers[bucket_number][left_page_number],
                key_size, value_size, max_tuples
            );
            current_right_page = std::make_unique<KeyValueHashBucket<K, V>>(
                buckets_file, buckets_page_numbers[bucket_number][right_page_number],
                key_size, value_size, max_tuples
            );
            uint_fast32_t left_pos = 0;
            uint_fast32_t right_pos = 0;
            left_key = current_left_page->get_key(left_pos);
            left_value = current_left_page->get_value(left_pos);
            right_key = current_right_page->get_key(right_pos);
            right_value = current_right_page->get_value(right_pos);
            while (left_page_number < step + merge_size &&
                    right_page_number < step + (2*merge_size) &&
                    right_page_number < buckets_page_numbers[bucket_number].size()) {
                bool smaller = false;
                for (uint_fast16_t i = 0; i < key_size; i++) {
                    if (left_key[i] < right_key[i]) {
                        smaller = true;
                        break;
                    }
                    else if (left_key[i] > right_key[i]) {
                        break;
                    }
                }
                // check if aux is full before insert
                if (current_bucket_page->get_tuple_count() >= max_tuples) {
                    if (available_pages.size() > 0) {
                        new_page_number = available_pages.front();
                        available_pages.pop();
                    } else {
                        new_page_number = ++last_page_number;
                    }
                    new_page_numbers.push_back(new_page_number);
                    current_bucket_page = std::make_unique<KeyValueHashBucket<K, V>>(
                        buckets_file, new_page_number,
                        key_size, value_size, max_tuples
                    );
                    *(current_bucket_page->tuple_count) = 0;
                    current_bucket_page->page.make_dirty();
                }
                // add to aux
                if (smaller) {
                    current_bucket_page->insert_with_pointers(left_key, left_value);
                    left_pos += 1;
                    if (left_pos >= current_left_page->get_tuple_count()) {
                        available_pages.push(buckets_page_numbers[bucket_number][left_page_number]);
                        left_page_number += 1;
                        if (left_page_number >= step + merge_size) {
                            break;
                        }
                        current_left_page = std::make_unique<KeyValueHashBucket<K, V>>(
                            buckets_file, buckets_page_numbers[bucket_number][left_page_number],
                            key_size, value_size, max_tuples
                        );
                        left_pos = 0;
                    }
                    left_key = current_left_page->get_key(left_pos);
                    left_value = current_left_page->get_value(left_pos);
                }
                else {
                    current_bucket_page->insert_with_pointers(right_key, right_value);
                    right_pos += 1;
                    if (right_pos >= current_right_page->get_tuple_count()) {
                        available_pages.push(buckets_page_numbers[bucket_number][right_page_number]);
                        right_page_number += 1;
                        if (right_page_number >= step + (2*merge_size) ||
                            right_page_number >= buckets_page_numbers[bucket_number].size()) {
                            break;
                        }
                        current_right_page = std::make_unique<KeyValueHashBucket<K, V>>(
                            buckets_file, buckets_page_numbers[bucket_number][right_page_number],
                            key_size, value_size, max_tuples
                        );
                        right_pos = 0;
                    }
                    right_key = current_right_page->get_key(right_pos);
                    right_value = current_right_page->get_value(right_pos);
                }
            }
            // insert the rest
            if (left_page_number < step + merge_size) {  // left have more tuples to insert
                while (left_page_number < step + merge_size) {
                    // check if aux is full before insert
                    if (current_bucket_page->get_tuple_count() >= max_tuples) {
                        if (available_pages.size() > 0) {
                            new_page_number = available_pages.front();
                            available_pages.pop();
                        } else {
                            new_page_number = ++last_page_number;
                        }
                        new_page_numbers.push_back(new_page_number);
                        current_bucket_page = std::make_unique<KeyValueHashBucket<K, V>>(
                            buckets_file, new_page_number,
                            key_size, value_size, max_tuples
                        );
                        *(current_bucket_page->tuple_count) = 0;
                        current_bucket_page->page.make_dirty();
                    }
                    current_bucket_page->insert_with_pointers(left_key, left_value);
                    left_pos += 1;
                    if (left_pos >= current_left_page->get_tuple_count()) {
                        available_pages.push(buckets_page_numbers[bucket_number][left_page_number]);
                        left_page_number += 1;
                        if (left_page_number >= step + merge_size) {
                            break;
                        }
                        current_left_page = std::make_unique<KeyValueHashBucket<K, V>>(
                            buckets_file, buckets_page_numbers[bucket_number][left_page_number],
                            key_size, value_size, max_tuples
                        );
                        left_pos = 0;
                    }
                    left_key = current_left_page->get_key(left_pos);
                    left_value = current_left_page->get_value(left_pos);
                }
            }
            else {               // right have more tuples to insert
                while (right_page_number < step + (2*merge_size) &&
                        right_page_number < buckets_page_numbers[bucket_number].size()) {
                    // check if aux is full before insert
                    if (current_bucket_page->get_tuple_count() >= max_tuples) {
                        if (available_pages.size() > 0) {
                            new_page_number = available_pages.front();
                            available_pages.pop();
                        } else {
                            new_page_number = ++last_page_number;
                        }
                        new_page_numbers.push_back(new_page_number);
                        current_bucket_page = std::make_unique<KeyValueHashBucket<K, V>>(
                            buckets_file, new_page_number,
                            key_size, value_size, max_tuples
                        );
                        *(current_bucket_page->tuple_count) = 0;
                        current_bucket_page->page.make_dirty();
                    }
                    current_bucket_page->insert_with_pointers(right_key, right_value);
                    right_pos += 1;
                    if (right_pos >= current_right_page->get_tuple_count()) {
                        available_pages.push(buckets_page_numbers[bucket_number][right_page_number]);
                        right_page_number += 1;
                        if (right_page_number >= step + (2*merge_size) ||
                            right_page_number >= buckets_page_numbers[bucket_number].size()) {
                            break;
                        }
                        current_right_page = std::make_unique<KeyValueHashBucket<K, V>>(
                            buckets_file, buckets_page_numbers[bucket_number][right_page_number],
                            key_size, value_size, max_tuples
                        );
                        right_pos = 0;
                    }
                    right_key = current_right_page->get_key(right_pos);
                    right_value = current_right_page->get_value(right_pos);
                }
            }
        }
        buckets_page_numbers[bucket_number] = move(new_page_numbers);
        merge_size *= 2;
    }
}
