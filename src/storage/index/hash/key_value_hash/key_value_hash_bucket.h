#ifndef STORAGE__KEY_VALUE_HASH_BUCKET_H_
#define STORAGE__KEY_VALUE_HASH_BUCKET_H_

#include <cstdint>
#include <map>
#include <string>
#include <memory>
#include <vector>

#include "base/ids/object_id.h"
#include "base/ids/var_id.h"
#include "storage/file_id.h"
#include "storage/page.h"

//using KeyValuePair = std::pair<std::vector<ObjectId>, std::vector<ObjectId>>;
template<class K, class V> class KeyValueHash;

template <class K, class V>
class KeyValueHashBucket {

friend class KeyValueHash<K, V>;

public:
    KeyValueHashBucket(const TmpFileId file_id, const uint_fast32_t bucket_number,
                       std::size_t key_size, std::size_t value_size);
    ~KeyValueHashBucket();

    void insert(const std::vector<K>& key, const std::vector<V>& value);
    void insert_in_pos(const std::vector<K>& key, const std::vector<V>& value, uint_fast32_t pos);  // for split
    std::pair<std::vector<K>, std::vector<V>> get_pair(uint_fast32_t current_pos) const; // TODO: can optimize without copying
    //std::pair<K*, V*> get_pair(uint_fast32_t current_pos) const;

    inline uint_fast32_t get_tuple_count() const noexcept { return *tuple_count; }
    inline void set_tuple_count(uint_fast32_t value) const noexcept { *tuple_count = value; }

private:
    Page& page;

    uint32_t   const key_size;
    uint32_t   const value_size;
    uint32_t*  const tuple_count;
    char*      const tuples;
};

#endif // STORAGE__KEY_VALUE_HASH_BUCKET_H_