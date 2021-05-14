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

// TODO: use templates
using KeyValuePair = std::pair<std::vector<ObjectId>, std::vector<ObjectId>>;


class KeyValueHashBucket {

friend class KeyValueHash;

public:
    KeyValueHashBucket(const TmpFileId file_id, const uint_fast32_t bucket_number, std::size_t key_size, std::size_t value_size);
    ~KeyValueHashBucket();

    void insert(const KeyValuePair& pair);
    void insert_in_pos(const KeyValuePair& pair, uint_fast32_t pos);  // for split
    KeyValuePair get_pair(uint_fast32_t current_pos) const;

    inline uint_fast32_t get_tuple_count() const noexcept { return *tuple_count; }
    inline void set_tuple_count(uint_fast32_t value) const noexcept { *tuple_count = value; }

private:
    Page& page;

    uint32_t   const key_size;
    uint32_t   const value_size;
    uint32_t*  const tuple_count;
    ObjectId*  const tuples;
};

#endif // STORAGE__KEY_VALUE_HASH_BUCKET_H_