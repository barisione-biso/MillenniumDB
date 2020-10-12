#include "index_scan.h"

#include <cassert>
#include <iostream>
#include <vector>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

template class IndexScan<2>;
template class IndexScan<3>;
template class IndexScan<4>;
template class std::unique_ptr<IndexScan<2>>;
template class std::unique_ptr<IndexScan<3>>;
template class std::unique_ptr<IndexScan<4>>;

template <std::size_t N>
IndexScan<N>::IndexScan(BPlusTree<N>& bpt, std::array<std::unique_ptr<ScanRange>, N> ranges) :
    bpt    (bpt),
    ranges (move(ranges)) { }


template <std::size_t N>
BindingId& IndexScan<N>::begin(BindingId& input) {
    assert(ranges.size() == N && "Inconsistent size of ranges and bpt");

    my_input = &input;
    my_binding.init(my_input->var_count());
    my_binding.add_all(*my_input);

    std::array<uint64_t, N> min_ids;
    std::array<uint64_t, N> max_ids;

    for (uint_fast32_t i = 0; i < N; ++i) {
        assert(ranges[i] != nullptr);

        min_ids[i] = ranges[i]->get_min(*my_input);
        max_ids[i] = ranges[i]->get_max(*my_input);
    }

    it = bpt.get_range(
        Record<N>(std::move(min_ids)),
        Record<N>(std::move(max_ids))
    );
    ++bpt_searches;
    return my_binding;
}


template <std::size_t N>
bool IndexScan<N>::next() {
    if (it == nullptr)
        return false;

    auto next = it->next();
    if (next != nullptr) {
        for (uint_fast32_t i = 0; i < N; ++i) {
            ranges[i]->try_assign(my_binding, ObjectId(next->ids[i]));
        }
        ++results_found;
        return true;
    } else {
        return false;
    }
}


template <std::size_t N>
void IndexScan<N>::reset() {
    // TODO: if nulls were supported a my_binding->clean should be performed to set NULL_OBJECT_ID
    my_binding.add_all(*my_input);

    std::array<uint64_t, N> min_ids;
    std::array<uint64_t, N> max_ids;

    for (uint_fast32_t i = 0; i < N; ++i) {
        min_ids[i] = ranges[i]->get_min(*my_input);
        max_ids[i] = ranges[i]->get_max(*my_input);
    }

    it = bpt.get_range(
        Record<N>(std::move(min_ids)),
        Record<N>(std::move(max_ids))
    );
    ++bpt_searches;
}


template <std::size_t N>
void IndexScan<N>::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    auto real_factor = static_cast<double>(results_found) / static_cast<double>(bpt_searches);
    cout << "IndexScan(bpt_searches: " << bpt_searches << ", found: " << results_found << ")\n";
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "  ↳ Real factor: " << real_factor;
}
