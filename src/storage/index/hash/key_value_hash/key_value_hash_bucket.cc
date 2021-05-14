#include "key_value_hash_bucket.h"

#include <cstring>

#include "storage/buffer_manager.h"
#include "base/ids/object_id.h"

using namespace std;

KeyValueHashBucket::KeyValueHashBucket(const TmpFileId file_id, const uint_fast32_t bucket_number,
                                        std::size_t key_size, std::size_t value_size) :
    page        (buffer_manager.get_tmp_page(file_id, bucket_number)),
    key_size    (key_size),
    value_size  (value_size),
    tuple_count ( reinterpret_cast<uint32_t*>(page.get_bytes() ) ),
    tuples      ( reinterpret_cast<ObjectId*>(page.get_bytes() + sizeof(uint32_t)) )
    {}


KeyValueHashBucket::~KeyValueHashBucket() {
    //page.make_dirty();
    buffer_manager.unpin(page);
}


void KeyValueHashBucket::insert(const KeyValuePair& pair) {
    for (uint_fast16_t i = 0; i < key_size; i++) {
        tuples[((key_size + value_size) * (*tuple_count)) + i] = pair.first[i];
    }
    for (uint_fast16_t i = 0; i < value_size; i++) {
        tuples[((key_size + value_size) * (*tuple_count)) + key_size + i] = pair.second[i];
    }
    ++(*tuple_count);
    page.make_dirty();
}


void KeyValueHashBucket::insert_in_pos(const KeyValuePair& pair, uint_fast32_t pos) {
    for (uint_fast16_t i = 0; i < key_size; i++) {
        tuples[((key_size + value_size) * (pos)) + i] = pair.first[i];
    }
    for (uint_fast16_t i = 0; i < value_size; i++) {
        tuples[((key_size + value_size) * (pos)) + key_size + i] = pair.second[i];
    }
    page.make_dirty();
}


KeyValuePair KeyValueHashBucket::get_pair(std::uint_fast32_t current_pos) const {
    std::vector<ObjectId> key;
    std::vector<ObjectId> value;
    key.reserve(key_size);
    value.reserve(value_size);
    for (uint_fast16_t i = 0; i < key_size; i++) {
        key.push_back(tuples[((key_size + value_size) * (current_pos)) + i]);
    }
    for (uint_fast16_t i = 0; i < value_size; i++) {
        value.push_back(tuples[((key_size + value_size) * (current_pos)) + key_size + i]);
    }
    return KeyValuePair(key, value);
}
