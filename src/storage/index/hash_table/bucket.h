#ifndef STORAGE__BUCKET_H_
#define STORAGE__BUCKET_H_

#include <cstdint>
#include <map>
#include <string>

#include "storage/file_id.h"
#include "storage/index/object_file/object_file.h"
#include "storage/page.h"

class Bucket {
friend class ExtendibleHash;

// 2 bytes needed for key_count and local_depth, 16 bytes from hash and 6 bytes from id
// TODO: maybe 5 bytes is enough => ~1TB of objects
static constexpr auto BYTES_FOR_ID = 6U;
static constexpr auto MAX_KEYS = (Page::PAGE_SIZE - 2) / (16+BYTES_FOR_ID);
static_assert(MAX_KEYS <= UINT8_MAX, "BUCKET KEY_COUNT(UINT8) CAN'T REACH MAX_KEYS");

public:
    Bucket(const FileId file_id, const uint_fast32_t bucket_number, ObjectFile& objecy_file);
    ~Bucket();

    uint64_t get_id(const std::string& str, const uint64_t hash1, const uint64_t hash2) const;

    uint64_t get_or_create_id(const std::string& str, const uint64_t hash1, const uint64_t hash2,
                              bool* const need_split, bool* const created);

private:
    Page& page;
    ObjectFile& object_file;

    uint8_t*  const key_count;
    uint8_t*  const local_depth;
    uint64_t* const hashes; // each tuple is (hash1, hash2)
    uint8_t*  const ids;

    void write_id(const uint64_t id, const uint_fast32_t index);
    uint64_t read_id(const uint_fast32_t index) const;

    void redistribute(Bucket& other, const uint64_t mask, const uint64_t other_suffix);
};

#endif // STORAGE__BUCKET_H_
