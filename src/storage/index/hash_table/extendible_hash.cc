#include "extendible_hash.h"

#include <bitset>
#include <cmath>
#include <cassert>
#include <cstring>
#include <iostream>

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

        uint_fast32_t dir_size = 1 << global_depth;
        dir = new uint_fast32_t[dir_size];
        for (uint_fast32_t i = 0; i < dir_size; ++i) {
            uint32_t tmp;
            dir_file.read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
            dir[i] = tmp;
        }
        // check eofbit/failbit/badbit to ensure  values were read correctly
        if (!dir_file.good()) {
            throw std::runtime_error("Error reading hash file.");
        }
    } else {
        global_depth = DEFAULT_GLOBAL_DEPTH;
        uint_fast32_t dir_size = 1 << global_depth;
        dir = new uint64_t[dir_size];
        for (uint_fast32_t i = 0; i < dir_size; ++i) {
            auto bucket = Bucket(buckets_file_id, i);
            bucket.key_count = 0;
            bucket.local_depth = DEFAULT_GLOBAL_DEPTH;
            dir[i] = i;
            bucket.page.make_dirty();
        }
    }
}


ExtendibleHash::~ExtendibleHash() {
    auto& dir_file = file_manager.get_file(dir_file_id);
    dir_file.seekg(0, dir_file.beg);

    dir_file.write(reinterpret_cast<const char*>(&global_depth), sizeof(uint8_t));
    uint_fast32_t dir_size = 1 << global_depth;
    for (uint64_t i = 0; i < dir_size; ++i) {
        uint32_t tmp = dir[i];
        dir_file.write(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
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

    // After a bucket split, need to try insert again.
    while (true) {
        // global_depth must be <= 64
        auto mask = 0xFFFF'FFFF'FFFF'FFFF >> (64 - global_depth);
        auto suffix = hash[0] & mask; // TODO: try XOR hash[0]^hash[1]?
        auto bucket_number = dir[suffix];
        auto bucket = Bucket(buckets_file_id, bucket_number);

        bool need_split = false;
        auto id = bucket.get_id(str, hash[0], hash[1], insert_if_not_present, &need_split);

        if (need_split) {
            // std::cout << "\nstring: " << str << "\n";
            // std::cout << "bucket        " << std::bitset<8*sizeof(bucket_number)>(bucket_number) << " split, global depth: " << (int)global_depth
            //     << ". local depth: " << (int)bucket.local_depth << ". key_count: " << (int)bucket.key_count << "\n";
            // std::cout << "hash:         " << std::bitset<8*sizeof(hash[0])>(hash[0]) << " | " << std::bitset<8*sizeof(hash[1])>(hash[1]) << "\n";

            if (bucket.local_depth < global_depth) {
                auto new_bucket_number = bucket_number | (1 << bucket.local_depth);
                ++bucket.local_depth;
                auto new_mask = 0xFFFF'FFFF'FFFF'FFFF >> (64 - bucket.local_depth);
                auto new_bucket = Bucket(buckets_file_id, new_bucket_number);
                new_bucket.key_count = 0;
                new_bucket.local_depth = bucket.local_depth;

                // redistribute keys between buckets `0|bucket_number` and `1|bucket_number`
                bucket.redistribute(new_bucket, new_mask, new_bucket_number);

                // update dirs having `new_bucket_number` suffix and point to the new_bucket
                auto update_dir_count = 1 << (global_depth - bucket.local_depth);
                for (auto i = 0; i < update_dir_count; ++i) {
                    dir[(i << bucket.local_depth) | new_bucket_number] = new_bucket_number;
                }
                // std::cout << "after redistribution: \n"
                //     << "  bucket.key_count: " << (int)bucket.key_count
                //     << "  bucket.local_depth: " << (int)bucket.local_depth
                //     << "  new_bucket.key_count: " << (int)new_bucket.key_count
                //     << "  new_bucket.local_depth: " << (int)new_bucket.local_depth << "\n";

                assert(bucket.key_count + new_bucket.key_count== Bucket::MAX_KEYS
                    && "EXTENDIBLE HASH INCONSISTENCY: sum of keys must be MAX_KEYS after a split");

            } else {
                assert(suffix == bucket_number && "EXTENDIBLE HASH INCONSISTENCY: suffix != bucket_number");
                assert(bucket.local_depth == global_depth && "EXTENDIBLE HASH INCONSISTENCY: bucket.local_depth != global_depth");
                ++bucket.local_depth;

                auto new_bucket_number = bucket_number | (1 << global_depth);
                auto new_bucket = Bucket(buckets_file_id, new_bucket_number);
                new_bucket.key_count = 0;
                new_bucket.local_depth = bucket.local_depth;

                duplicate_dirs();

                // redistribute keys between buckets `0|bucket_number` and `1|bucket_number`
                auto new_mask = 0xFFFF'FFFF'FFFF'FFFF >> (64 - global_depth);
                bucket.redistribute(new_bucket, new_mask, new_bucket_number);

                // update dir for `1|bucket_number`
                dir[new_bucket_number] = new_bucket_number;

                // std::cout << "after redistribution: \n"
                //     << "  bucket.key_count: " << (int)bucket.key_count
                //     << "  bucket.local_depth: " << (int)bucket.local_depth
                //     << "  new_bucket.key_count: " << (int)new_bucket.key_count
                //     << "  new_bucket.local_depth: " << (int)new_bucket.local_depth << "\n";

                assert(bucket.key_count + new_bucket.key_count== Bucket::MAX_KEYS
                    && "EXTENDIBLE HASH INCONSISTENCY: sum of keys must be MAX_KEYS after a split");
            }
        } else {
            return id;
        }
    }
}
