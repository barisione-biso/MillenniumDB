#ifndef STORAGE__BPT_MERGER_H_
#define STORAGE__BPT_MERGER_H_

#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/ordered_file/bpt_leaf_provider.h"
#include "storage/index/ordered_file/ordered_file.h"

template <std::size_t N>
class BptMerger : public BptLeafProvider {
public:
    BptMerger(OrderedFile<N>&, BPlusTree<N>&);
    ~BptMerger();

    void begin() override;
    bool has_more_tuples() override;
    uint_fast32_t next_tuples(uint64_t* output, uint_fast32_t max_tuples) override;

private:
    OrderedFile<N>& ordered_file;
    BPlusTree<N>& bpt;

    std::unique_ptr<BptIter<N>> bpt_iter;
    std::unique_ptr<Record<N>> bpt_record;
    std::unique_ptr<Record<N>> ordered_file_record;
};

template class BptMerger<2>;
template class BptMerger<3>;
template class BptMerger<4>;

#endif // STORAGE__BPT_MERGER_H_
