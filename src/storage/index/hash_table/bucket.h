#ifndef STORAGE__BUCKET_H_
#define STORAGE__BUCKET_H_

#include <cstdint>
#include <string>
#include <map>

#include "storage/file_id.h"
#include "storage/page.h"

class Bucket {
friend class ExtendibleHash;

// 2 bytes needed for key_count and local_depth, 16 bytes from hash and 8 bytes from id
static constexpr auto MAX_KEYS = (PAGE_SIZE - 2) / (16+8);

public:
    Bucket(FileId file_id, uint_fast32_t bucket_number);
    ~Bucket();

    uint64_t get_id(const std::string& str, uint64_t hash1, uint64_t hash2, bool insert_if_not_present);

private:
    // FileId file_id;
    // uint_fast32_t bucket_number;
    Page& page;
    uint8_t* key_count;
    uint8_t* local_depth;
    uint64_t* data; // each tuple is (hash1, hash2, id)
};

#endif // STORAGE__BUCKET_H_
