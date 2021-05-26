#include "key_value_hash_bucket.h"

#include <cstring>

#include "storage/buffer_manager.h"
#include "base/ids/object_id.h"

using namespace std;

template class KeyValueHashBucket<ObjectId, ObjectId>;

template <class K, class V>
KeyValueHashBucket<K, V>::KeyValueHashBucket(const TmpFileId file_id, const uint_fast32_t bucket_number,
                                        std::size_t key_size, std::size_t value_size) :
    page        (buffer_manager.get_tmp_page(file_id, bucket_number)),
    key_size    (key_size),
    value_size  (value_size),
    tuple_count ( reinterpret_cast<uint32_t*>(page.get_bytes() ) ),
    tuples      ( reinterpret_cast<char*>(page.get_bytes() + sizeof(uint32_t)) )
    {}


template <class K, class V>
KeyValueHashBucket<K, V>::~KeyValueHashBucket() {
    //page.make_dirty();
    buffer_manager.unpin(page);
}


template <class K, class V>
void KeyValueHashBucket<K, V>::insert(const vector<K>& key, const vector<V>& value) {
    K* p_k = reinterpret_cast<K*>(&tuples[(key_size*sizeof(K) + value_size*sizeof(V)) * (*tuple_count)]);
    for (uint_fast16_t i = 0; i < key_size; i++) {
        *p_k = key[i];
        p_k++;
    }
    V* p_v = reinterpret_cast<V*>(p_k);
    for (uint_fast16_t i = 0; i < value_size; i++) {
        *p_v = value[i];
        p_v++;
    }
    ++(*tuple_count);
    page.make_dirty();
}


template <class K, class V>
void KeyValueHashBucket<K, V>::insert_in_pos(const vector<K>& key, const vector<V>& value, uint_fast32_t pos) {
    K* p_k = reinterpret_cast<K*>(&tuples[(key_size*sizeof(K) + value_size*sizeof(V)) * pos]);
    for (uint_fast16_t i = 0; i < key_size; i++) {
        *p_k = key[i];
        p_k++;
    }
    V* p_v = reinterpret_cast<V*>(p_k);
    for (uint_fast16_t i = 0; i < value_size; i++) {
        *p_v = value[i];
        p_v++;
    }
    page.make_dirty();
}


template <class K, class V>
pair<vector<K>, vector<V>> KeyValueHashBucket<K, V>::get_pair(std::uint_fast32_t pos) const {
    std::vector<K> key;
    std::vector<V> value;
    key.reserve(key_size);
    value.reserve(value_size);
    K* p_k = reinterpret_cast<K*>(&tuples[(key_size*sizeof(K) + value_size*sizeof(V)) * pos]);
    for (uint_fast16_t i = 0; i < key_size; i++) {
        key.push_back(*p_k);
        p_k++;
    }
    V* p_v = reinterpret_cast<V*>(p_k);
    for (uint_fast16_t i = 0; i < value_size; i++) {
        value.push_back(*p_v);
        p_v++;
    }
    return make_pair(key, value);
}
