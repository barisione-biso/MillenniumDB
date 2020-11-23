#ifndef RELATIONAL_MODEL__TRANSITIVE_CLOSURE_H_
#define RELATIONAL_MODEL__TRANSITIVE_CLOSURE_H_

#include <array>
#include <memory>

#include "base/binding/binding_id_iter.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"

template <std::size_t N>
class TransitiveClosure : public BindingIdIter {
private:
    BPlusTree<N>& bpt;
    std::unique_ptr<BptIter<N>> it;

    BindingId* my_input;
    std::array<std::unique_ptr<ScanRange>, N> ranges;

    // statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

public:
    TransitiveClosure(std::size_t binding_size, BPlusTree<N>& bpt, std::array<std::unique_ptr<ScanRange>, N> ranges);
    ~TransitiveClosure() = default;

    void analyze(int indent = 0) const override;
    BindingId& begin(BindingId& input) override;
    void reset() override;
    bool next() override;
};

#endif // RELATIONAL_MODEL__TRANSITIVE_CLOSURE_H_
