#include "bucket.h"

#include <memory>
#include <iostream>

#include "storage/buffer_manager.h"
#include "relational_model/relational_model.h"

using namespace std;

Bucket::Bucket(FileId file_id, uint_fast32_t bucket_number) :
    page( buffer_manager.get_page(file_id, bucket_number) )
{
    auto bytes = page.get_bytes();

    key_count   = reinterpret_cast<uint8_t*>(bytes);
    local_depth = reinterpret_cast<uint8_t*>(bytes + sizeof(uint8_t));
    data        = reinterpret_cast<uint64_t*>(bytes + 2*sizeof(uint8_t));
}


Bucket::~Bucket() {
    buffer_manager.unpin(page);
}


uint64_t Bucket::get_id(const string& str, uint64_t hash1, uint64_t hash2, bool insert_if_not_present, bool* need_split) {
    for (uint8_t i = 0; i < *key_count; ++i) {
        if (data[3*i] == hash1 && data[3*i + 1] == hash2) {
            // check if object is
            auto bytes = relational_model.get_object_file().read(data[3*i + 2]);
            string value_string(bytes->begin(), bytes->end());
            // TODO: use string cache
            // auto saved_str = relational_model.get_string(data[3*i + 2]);
            if (value_string == str) {
                return data[3*i + 2];
            }
        }
    }
    if (insert_if_not_present) {
        auto bytes = make_unique<vector<unsigned char>>(str.length());
        copy(str.begin(), str.end(), bytes->begin());
        auto new_id = relational_model.get_object_file().write(*bytes);

        if (*key_count >= MAX_KEYS) {
            *need_split = true;
            return ObjectId::OBJECT_ID_NOT_FOUND;
        }

        ++(*key_count);
        data[3 * (*key_count)] = hash1;
        data[3 * (*key_count) + 1] = hash2;
        data[3 * (*key_count) + 2] = new_id;
        page.make_dirty();

        return new_id;
    } else {
        return ObjectId::OBJECT_ID_NOT_FOUND;
    }
}
