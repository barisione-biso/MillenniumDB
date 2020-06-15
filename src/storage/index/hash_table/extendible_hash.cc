#include "extendible_hash.h"

#include <cmath>
#include <cstring>

#include "storage/file_manager.h"
#include "storage/index/hash_table/murmur3.h"
#include "storage/index/hash_table/bucket.h"

ExtendibleHash::ExtendibleHash(const std::string& filename) :
    dir_file_id(file_manager.get_file_id(filename + ".dir")),
    buckets_file_id(file_manager.get_file_id(filename + ".dat"))
{
    auto& dir_file = file_manager.get_file(dir_file_id);
    dir_file.seekg(0, dir_file.end);
    // If the file is not empty, read the values
    if (dir_file.tellg() != 0) {
        dir_file.seekg(0, dir_file.beg);

        uint8_t f_global_depth;
        dir_file.read(reinterpret_cast<char*>(&f_global_depth), sizeof(f_global_depth));
        global_depth = f_global_depth;

        auto dir_size = 1 << global_depth;
        dir = new uint64_t[dir_size];
        for (uint64_t i = 0; i < dir_size; ++i) {
            dir_file.read(reinterpret_cast<char*>(&dir[i]), sizeof(uint64_t));
        }
        // TODO: check bits to ensure all values were read successfully
    } else {
        global_depth = 10;
        auto dir_size = 1 << global_depth;
        dir = new uint64_t[dir_size];
        for (uint64_t i = 0; i < dir_size; ++i) {
            auto bucket = Bucket(buckets_file_id, i);
            *bucket.key_count = 0;
            *bucket.local_depth = global_depth;
            dir[i] = i;
        }
    }
}


ExtendibleHash::~ExtendibleHash() {
    auto& dir_file = file_manager.get_file(dir_file_id);
    dir_file.seekg(0, dir_file.beg);

    dir_file.write(reinterpret_cast<const char*>(&global_depth), sizeof(uint8_t));
    auto dir_size = 1 << global_depth;
    for (uint64_t i = 0; i < dir_size; ++i) {
        dir_file.write(reinterpret_cast<const char*>(&dir[i]), sizeof(uint64_t));
    }
}


void ExtendibleHash::duplicate_dirs() {
    auto old_dir_size = 1 << global_depth;
    ++global_depth;
    auto new_dir_size = 1 << global_depth;
    auto new_dir = new uint64_t[new_dir_size];

    std::memcpy(
        new_dir,
        dir,
        old_dir_size * sizeof(uint64_t)
    );

    std::memcpy(
        &new_dir[old_dir_size],
        dir,
        old_dir_size * sizeof(uint64_t)
    );

    delete[](dir);
    dir = new_dir;
}

uint64_t ExtendibleHash::get_id(const std::string& str, bool insert_if_not_present) {
    uint64_t hash[2];
    MurmurHash3_x64_128(str.data(), str.length(), 0, hash);

    // se asume que la profundidad global sera <= 64
    auto mask = 0xFFFF'FFFF'FFFF'FFFF >> (64-global_depth);

    auto bucket_number = hash[0] & mask;

    auto bucket = Bucket(buckets_file_id, bucket_number);
    // TODO: no hace split por ahora
    return bucket.get_id(str, hash[0], hash[1], insert_if_not_present);
}
