#ifndef RELATIONAL_MODEL__GRAPH_SCAN_H_
#define RELATIONAL_MODEL__GRAPH_SCAN_H_

#include "storage/index/bplus_tree/bplus_tree.h"
#include "relational_model/execution/binding/binding_id_iter.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"

#include <functional>
#include <list>
#include <memory>
#include <tuple>

template <std::size_t N>
class IndexScan : public BindingIdIter {
private:
    BPlusTree<N>& bpt;
    std::unique_ptr<BptIter<N>> it;

    BindingId* my_input;
    std::unique_ptr<BindingId> my_binding;
    std::array<std::unique_ptr<ScanRange>, N> ranges;

    // statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

public:
    IndexScan(BPlusTree<N>& bpt, std::array<std::unique_ptr<ScanRange>, N> ranges);
    ~IndexScan() = default;

    void begin(BindingId& input);
    void reset(BindingId& input);
    BindingId* next();

    // prints execution statistics
    void analyze(int indent = 0) const override;
};

template class IndexScan<2>;
template class IndexScan<3>;
template class IndexScan<4>;

#endif // RELATIONAL_MODEL__GRAPH_SCAN_H_
