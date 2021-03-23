#include "extendable_bucket.h"

#include <cstring>

#include "storage/buffer_manager.h"
#include "base/ids/object_id.h"
#include "base/graph/graph_object.h"

using namespace std;

template class ExtendableBucket<GraphObject>;
template class ExtendableBucket<ObjectId>;


template <class T>
ExtendableBucket<T>::ExtendableBucket(const FileId file_id, const uint_fast32_t bucket_number, std::size_t _tuple_size) :
    page        (buffer_manager.get_page(file_id, bucket_number)),
    MAX_TUPLES  ( (Page::PAGE_SIZE - sizeof(*tuple_size) - sizeof(*tuple_count) - sizeof(local_depth))
                  / (2*sizeof(*hashes) + _tuple_size*sizeof(T) ) ),


    tuple_size  ( reinterpret_cast<uint16_t*>(page.get_bytes()) ),
    tuple_count ( reinterpret_cast<uint8_t*> (page.get_bytes() + sizeof(*tuple_size)) ),
    local_depth ( reinterpret_cast<uint8_t*> (page.get_bytes() + sizeof(*tuple_size) + sizeof(*tuple_count)) ),
    hashes      ( reinterpret_cast<uint64_t*>(page.get_bytes() + sizeof(*tuple_size) + sizeof(*tuple_count) + sizeof(*local_depth)) ),
    tuples      ( reinterpret_cast<T*> (reinterpret_cast<uint8_t*>(hashes) + 2*MAX_TUPLES*sizeof(*hashes)) )
{
    *tuple_size = _tuple_size;
}


template <class T>
ExtendableBucket<T>::~ExtendableBucket() {
    buffer_manager.unpin(page);
}


template <class T>
bool ExtendableBucket<T>::is_in(std::vector<T>& tuple, const uint64_t hash1, const uint64_t hash2) {
    for (uint8_t i = 0; i < *tuple_count; ++i) {
        if (hashes[2*i] == hash1 && hashes[2*i + 1] == hash2) {
            bool tuple_found = true;
            for (uint_fast16_t j = 0; j < *tuple_size; j++) {
                if (tuple[j] != tuples[((*tuple_size) * i) + j]) {
                    tuple_found = false;
                    break;
                }
            }
            if (tuple_found) {
                return true;
            }
        }
    }
    return false;
}


template <class T>
bool ExtendableBucket<T>::is_in_or_insert(std::vector<T>& tuple, const uint64_t hash1, const uint64_t hash2, bool* const need_split) {
    for (uint8_t i = 0; i < *tuple_count; ++i) {
        if (hashes[2*i] == hash1 && hashes[2*i + 1] == hash2) {
            bool tuple_found = true;
            for (uint_fast16_t j = 0; j < *tuple_size; j++) {
                if (tuple[j] != tuples[((*tuple_size) * i) + j]) {
                    tuple_found = false;
                    break;
                }
            }
            if (tuple_found) {
                *need_split = false;
                return true;
            }
        }
    }
    if (*tuple_count == MAX_TUPLES) {
        *need_split = true;
        return false; // needs to try to insert again
    }

    hashes[2 * (*tuple_count)]     = hash1;
    hashes[2 * (*tuple_count) + 1] = hash2;

    for (uint_fast16_t i = 0; i < *tuple_size; i++) {
        tuples[((*tuple_size) * (*tuple_count)) + i] = tuple[i];
    }
    ++(*tuple_count);
    page.make_dirty();

    *need_split = false;
    return false;
}


template <class T>
void ExtendableBucket<T>::redistribute(ExtendableBucket<T>& other, const uint64_t mask, const uint64_t other_suffix) {
    uint8_t other_pos = 0;
    uint8_t this_pos = 0;

    for (uint8_t i = 0; i < *tuple_count; i++) {
        auto suffix = mask & hashes[2 * i];

        if (suffix == other_suffix) {
            // copy hash to other bucket
            std::memcpy(
                &other.hashes[2*other_pos],
                &hashes[2*i],
                2 * sizeof(uint64_t)
            );

            std::memcpy(
                &other.tuples[*tuple_size * other_pos],
                &tuples[*tuple_size*i],
                *tuple_size * sizeof(T)
            );
            ++other_pos;
        } else {
            if (i != this_pos) { // avoid redundant copy
                // copy hash in this bucket
                std::memcpy(
                    &hashes[2*this_pos],
                    &hashes[2*i],
                    2 * sizeof(uint64_t)
                );

                std::memcpy(
                    &tuples[*tuple_size * this_pos],
                    &tuples[*tuple_size *i],
                    *tuple_size * sizeof(T)
                );
            }
            ++this_pos;
        }
    }
    *this->tuple_count = this_pos;
    *other.tuple_count = other_pos;
    this->page.make_dirty();
    other.page.make_dirty();
}
