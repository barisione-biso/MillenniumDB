#ifndef STORAGE__EXTENDABLE_BUCKET_H_
#define STORAGE__EXTENDABLE_BUCKET_H_

#include <cstdint>
#include <map>
#include <string>
#include <memory>
#include <vector>

#include "base/ids/object_id.h"
#include "base/ids/var_id.h"
#include "storage/file_id.h"
#include "storage/page.h"

template <class U> class ExtendableTable;

template <class T>
class ExtendableBucket {

friend class ExtendableTable<T>;

public:
    ExtendableBucket(const FileId file_id, const uint_fast32_t bucket_number, std::size_t tuple_size);
    ~ExtendableBucket();

    bool is_in(std::vector<T>& tuple, const uint64_t hash1, const uint64_t hash2);
    bool is_in_or_insert(std::vector<T>& tuple, const uint64_t hash1, const uint64_t hash2, bool* const need_split);

private:
    Page& page;

    const uint8_t MAX_TUPLES;

    uint16_t*       const tuple_size;
    uint8_t*        const tuple_count;
    uint8_t*        const local_depth;
    uint64_t*       const hashes; // each tuple is (hash1, hash2)
    T*              const tuples;

    void redistribute(ExtendableBucket<T>& other, const uint64_t mask, const uint64_t other_suffix);
};

#endif // STORAGE__EXTENDABLE_BUCKET_H_