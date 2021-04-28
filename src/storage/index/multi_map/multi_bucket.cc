#include "multi_bucket.h"

#include <cstring>

#include "storage/buffer_manager.h"
#include "base/ids/object_id.h"

using namespace std;

MultiBucket::MultiBucket(Page& page, std::size_t key_size, std::size_t value_size) :
    page        (page),
    key_size    (key_size),
    value_size  (value_size),
    tuple_count ( reinterpret_cast<uint32_t*>(page.get_bytes() ) ),
    tuples      ( reinterpret_cast<ObjectId*>(page.get_bytes() + sizeof(uint32_t)) )
    {}


MultiBucket::~MultiBucket() {
    //page.make_dirty();
    buffer_manager.unpin(page);
}


void MultiBucket::insert(const MultiPair& pair) {
    for (uint_fast16_t i = 0; i < key_size; i++) {
        tuples[((key_size + value_size) * (*tuple_count)) + i] = pair.first[i];
    }
    for (uint_fast16_t i = 0; i < value_size; i++) {
        tuples[((key_size + value_size) * (*tuple_count)) + key_size + i] = pair.second[i];
    }
    ++(*tuple_count);
    page.make_dirty();
}


MultiPair MultiBucket::get_pair(std::uint_fast32_t current_pos) const {
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
    return MultiPair(key, value);
}
