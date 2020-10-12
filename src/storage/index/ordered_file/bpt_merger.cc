#include "bpt_merger.h"

#include <cstring>

using namespace std;

template <std::size_t N>
BptMerger<N>::BptMerger(OrderedFile<N>& ordered_file, BPlusTree<N>& bpt) :
    ordered_file(ordered_file),
    bpt(bpt) { }


template <std::size_t N>
BptMerger<N>::~BptMerger() = default;


template <std::size_t N>
void BptMerger<N>::begin() {
    ordered_file.begin();

    std::array<uint64_t, N> min_range;
    min_range.fill(0);

    std::array<uint64_t, N> max_range;
    max_range.fill(UINT64_MAX);

    bpt_iter = bpt.get_range(move(min_range), move(max_range));

    ordered_file_record = ordered_file.next_record();
    bpt_record = bpt_iter->next();
}


template <std::size_t N>
bool BptMerger<N>::has_more_tuples() {
    return ordered_file_record != nullptr || bpt_record != nullptr;
}


template <std::size_t N>
uint_fast32_t BptMerger<N>::next_tuples(uint64_t* output, uint_fast32_t max_tuples) {
    auto current_output = output;
    uint_fast32_t copied_tuples = 0;
    while (copied_tuples < max_tuples && (ordered_file_record != nullptr || bpt_record != nullptr)) {
        copied_tuples++;
        if (ordered_file_record == nullptr) {
            memcpy(
                current_output,
                bpt_record->ids.data(),
                N * sizeof(uint64_t)
            );
            bpt_record = bpt_iter->next();
        }
        else if (bpt_record == nullptr) {
            memcpy(
                current_output,
                ordered_file_record->ids.data(),
                N * sizeof(uint64_t)
            );
            ordered_file_record = ordered_file.next_record();
        }
        else if (*ordered_file_record < *bpt_record) {
            memcpy(
                current_output,
                ordered_file_record->ids.data(),
                N * sizeof(uint64_t)
            );
            ordered_file_record = ordered_file.next_record();
        }
        else if (*bpt_record < *ordered_file_record) {
            memcpy(
                current_output,
                bpt_record->ids.data(),
                N * sizeof(uint64_t)
            );
            bpt_record = bpt_iter->next();
        }
        else {
            throw logic_error("duplicated record at merging.");
        }
        current_output += N;
    }
    return copied_tuples;
}
