#include "bucket.h"

#include <bitset>
#include <cstring>
#include <memory>
#include <iostream>

#include "storage/buffer_manager.h"
#include "relational_model/relational_model.h"

using namespace std;

Bucket::Bucket(FileId file_id, uint_fast32_t bucket_number) :
    page( buffer_manager.get_page(file_id, bucket_number) )
{
    auto bytes = page.get_bytes();

    auto key_count_ptr   = reinterpret_cast<uint8_t*>(bytes);
    auto local_depth_ptr = reinterpret_cast<uint8_t*>(bytes + sizeof(uint8_t));

    key_count = *key_count_ptr;
    local_depth = *local_depth_ptr;

    hashes      = reinterpret_cast<uint64_t*>(bytes + 2*sizeof(uint8_t));
    ids         = reinterpret_cast<uint8_t*>(bytes + 2*sizeof(uint8_t) + 2*MAX_KEYS*sizeof(uint64_t));
}


Bucket::~Bucket() {
    auto bytes = page.get_bytes();
    auto key_count_ptr   = reinterpret_cast<uint8_t*>(bytes);
    auto local_depth_ptr = reinterpret_cast<uint8_t*>(bytes + sizeof(uint8_t));

    *key_count_ptr = key_count;
    *local_depth_ptr = local_depth;

    buffer_manager.unpin(page);
}


uint64_t Bucket::get_id(const string& str, uint64_t hash1, uint64_t hash2, bool insert_if_not_present, bool* need_split) {
    for (uint8_t i = 0; i < key_count; ++i) {
        if (hashes[2*i] == hash1 && hashes[2*i + 1] == hash2) {
            // check if object is
            auto id = read_id(i);
            auto bytes = relational_model.get_object_file().read(id);
            string value_string(bytes->begin(), bytes->end());
            // TODO: try using string cache
            if (value_string == str) {
                return id;
            }
        }
    }
    if (insert_if_not_present) {
        if (key_count == MAX_KEYS) {
            *need_split = true;
            return ObjectId::OBJECT_ID_NOT_FOUND;
        }

        auto bytes = make_unique<vector<unsigned char>>(str.length());
        copy(str.begin(), str.end(), bytes->begin());
        auto new_id = relational_model.get_object_file().write(*bytes);

        hashes[2 * key_count]     = hash1;
        hashes[2 * key_count + 1] = hash2;

        write_id(new_id, key_count);
        ++key_count;
        page.make_dirty();

        return new_id;
    } else {
        return ObjectId::OBJECT_ID_NOT_FOUND;
    }
}

void Bucket::write_id(uint64_t id, int i) {
    auto offset = 6*i;

    ids[offset]     = static_cast<uint8_t>(  id        & 0xFF );
    ids[offset + 1] = static_cast<uint8_t>( (id >>  8) & 0xFF );
    ids[offset + 2] = static_cast<uint8_t>( (id >> 16) & 0xFF );
    ids[offset + 3] = static_cast<uint8_t>( (id >> 24) & 0xFF );
    ids[offset + 4] = static_cast<uint8_t>( (id >> 32) & 0xFF );
    ids[offset + 5] = static_cast<uint8_t>( (id >> 40) & 0xFF );
}


uint64_t Bucket::read_id(int i) {
    auto offset = 6*i;

    return ids[offset]
           + (static_cast<uint64_t>(ids[offset + 1]) <<  8)
           + (static_cast<uint64_t>(ids[offset + 2]) << 16)
           + (static_cast<uint64_t>(ids[offset + 3]) << 24)
           + (static_cast<uint64_t>(ids[offset + 4]) << 32)
           + (static_cast<uint64_t>(ids[offset + 5]) << 40);
}


void Bucket::redistribute(Bucket& other, uint64_t mask, uint64_t other_suffix) {
    // std::cout << "mask:         " << std::bitset<8*sizeof(mask)>(mask) << "\n";
    // std::cout << "other_suffix: " << std::bitset<8*sizeof(other_suffix)>(other_suffix) << "\n";

    // for (int i = 0; i < key_count; ++i) {
    //     std::cout << "hash[i][0]:   " << std::bitset<8*sizeof(hashes[2*i])>(hashes[2*i]) << "\n";
    // }

    uint8_t current_pos = 0;
    uint8_t other_pos = 0;

    while (current_pos < key_count) {
        auto suffix = mask & hashes[2 * current_pos];
        // std::cout << "suffix:       " << std::bitset<8*sizeof(suffix)>(suffix) << "\n";

        if (suffix == other_suffix) {
            std::memcpy(
                &other.hashes[2*other_pos],
                &hashes[2*current_pos],
                2 * sizeof(uint64_t)
            );

            std::memcpy(
                &other.ids[6*other_pos],
                &ids[6*current_pos],
                6 * sizeof(uint8_t)
            );
            ++other_pos;
            --key_count;

            if ( current_pos != key_count ) {
                // put last record in current_pos
                std::memcpy(
                    &hashes[2*current_pos],
                    &hashes[2*key_count],
                    2 * sizeof(uint64_t)
                );

                std::memcpy(
                    &ids[6*current_pos],
                    &ids[6*key_count],
                    6 * sizeof(uint8_t)
                );
            } else {
                // put last record in current_pos
                break;
            }
        } else {
            current_pos++;
        }
    }
    other.key_count = other_pos;
    other.page.make_dirty();
}
