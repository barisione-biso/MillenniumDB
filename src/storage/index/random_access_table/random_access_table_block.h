#ifndef STORAGE__RANDOM_ACCESS_TABLE_BLOCK_H_
#define STORAGE__RANDOM_ACCESS_TABLE_BLOCK_H_

#include <fstream>
#include <memory>
#include <string>

#include "storage/index/record.h"
#include "storage/page.h"

// N is the columns of the table
template <std::size_t N> class RandomAccessTableBlock {
public:
    static constexpr auto max_records = (Page::PAGE_SIZE - sizeof(uint32_t)) / (sizeof(uint64_t) * N);

    RandomAccessTableBlock(Page& page);
    ~RandomAccessTableBlock();

    // returns false if record was not inserted
    bool try_append_record(Record<N>);

    // in case of out-of-bounds returns nullptr
    std::unique_ptr<Record<N>> operator[](uint_fast32_t pos);

// private:
    Page& page;
    uint32_t* record_count;
    uint64_t* records;
};

template class RandomAccessTableBlock<1>;
template class RandomAccessTableBlock<3>;


#endif // STORAGE__RANDOM_ACCESS_TABLE_BLOCK_H_