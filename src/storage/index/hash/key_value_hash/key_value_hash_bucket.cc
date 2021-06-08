#include "key_value_hash_bucket.h"

#include <cstring>

#include "storage/buffer_manager.h"
#include "base/ids/object_id.h"

using namespace std;

// alignment works well for <ObjectId, ObjectId> but need to analyze for other cases
template class KeyValueHashBucket<ObjectId, ObjectId>;

template <class K, class V>
KeyValueHashBucket<K, V>::KeyValueHashBucket(TmpFileId     file_id,
                                             uint_fast32_t bucket_number,
                                             std::size_t   _key_size,
                                             std::size_t   _value_size,
                                             uint32_t      _max_tuples) :

    page        (buffer_manager.get_tmp_page(file_id, bucket_number)),
    key_size    (_key_size),
    value_size  (_value_size),
    max_tuples  (_max_tuples),
    tuple_count ( reinterpret_cast<uint64_t*>(page.get_bytes() ) ),
    keys        ( reinterpret_cast<K*>(page.get_bytes() + sizeof(uint64_t)) ),
    values      ( reinterpret_cast<V*>(page.get_bytes() + sizeof(uint64_t) + (sizeof(K) * key_size * max_tuples) ))
    {}


template <class K, class V>
KeyValueHashBucket<K, V>::~KeyValueHashBucket() {
    //page.make_dirty();
    buffer_manager.unpin(page);
}


template <class K, class V>
void KeyValueHashBucket<K, V>::insert(const vector<K>& key, const vector<V>& value) {
    for (uint_fast16_t i = 0; i < key_size; i++) {
        keys[(key_size * (*tuple_count)) + i] = key[i];
    }
    for (uint_fast16_t i = 0; i < value_size; i++) {
        values[(value_size * (*tuple_count)) + i] = value[i];
    }
    ++(*tuple_count);
    page.make_dirty();
}


template <class K, class V>
void KeyValueHashBucket<K, V>::insert_in_pos(const vector<K>& key, const vector<V>& value, uint_fast32_t pos) {
    for (uint_fast16_t i = 0; i < key_size; i++) {
        keys[(key_size * pos) + i] = key[i];
    }
    for (uint_fast16_t i = 0; i < value_size; i++) {
        values[(value_size * pos) + i] = value[i];
    }
    page.make_dirty();
}


template <class K, class V>
pair<vector<K>, vector<V>> KeyValueHashBucket<K, V>::get_pair(std::uint_fast32_t pos) const {
    std::vector<K> key;
    std::vector<V> value;
    key.resize(key_size);
    value.resize(value_size);
    for (uint_fast16_t i = 0; i < key_size; i++) {
        key[i] = keys[(key_size * pos) + i];
    }
    for (uint_fast16_t i = 0; i < value_size; i++) {
        value[i] = values[(value_size * pos) + i];
    }
    return make_pair(move(key), move(value));
}
