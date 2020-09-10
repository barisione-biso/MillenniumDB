#ifndef STORAGE__BUCKET_H_
#define STORAGE__BUCKET_H_

#include <cstdint>
#include <string>
#include <map>

#include "storage/file_id.h"
#include "storage/page.h"
#include "storage/index/object_file/object_file.h"

class Bucket {
friend class ExtendibleHash;

// 2 bytes needed for key_count and local_depth, 16 bytes from hash and 6 bytes from id
static constexpr auto MAX_KEYS = (PAGE_SIZE - 2) / (16+6);
static_assert(MAX_KEYS <= UINT8_MAX, "BUCKET KEY_COUNT(UINT8) CAN'T REACH MAX_KEYS");

public:
    Bucket(FileId file_id, uint_fast32_t bucket_number, ObjectFile& objecy_file);
    ~Bucket();

    uint64_t get_id(const std::string& str, uint64_t hash1, uint64_t hash2, bool insert_if_not_present, bool* need_split);

private:
    Page& page;
    ObjectFile& object_file;

    uint8_t key_count;
    uint8_t local_depth;
    uint64_t* hashes; // each tuple is (hash1, hash2)
    uint8_t* ids;

    void write_id(uint64_t id, int index);
    uint64_t read_id(int index);

    void redistribute(Bucket& other, uint64_t mask, uint64_t other_suffix);
};

#endif // STORAGE__BUCKET_H_
