#ifndef STORAGE__MULTI_BUCKET_H_
#define STORAGE__MULTI_BUCKET_H_

#include <cstdint>
#include <map>
#include <string>
#include <memory>
#include <vector>

#include "base/ids/object_id.h"
#include "base/ids/var_id.h"
#include "storage/file_id.h"
#include "storage/page.h"

using MultiPair = std::pair<std::vector<ObjectId>, std::vector<ObjectId>>;


class MultiBucket {

friend class MultiMap;

public:
    MultiBucket(Page& page, std::size_t key_size, std::size_t value_size);
    ~MultiBucket();

    void insert(const MultiPair& pair);
    inline uint_fast32_t get_tuple_count() const noexcept { return *tuple_count; }
    MultiPair get_pair(uint_fast32_t current_pos);
    //inline bool is_full() const noexcept { return *tuple_count < MAX_TUPLES; }

private:
    Page& page;

    //const uint32_t MAX_TUPLES;

    uint32_t   const key_size;
    uint32_t   const value_size;
    uint32_t*  const tuple_count;
    ObjectId*  const tuples;
};

#endif // STORAGE__MULTI_BUCKET_H_