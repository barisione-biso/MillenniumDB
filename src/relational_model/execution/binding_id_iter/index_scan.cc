#include "index_scan.h"

#include <cassert>
#include <iostream>
#include <vector>

#include "base/ids/var_id.h"
#include "relational_model/relational_model.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

template <std::size_t N>
IndexScan<N>::IndexScan(BPlusTree<N>& bpt, std::array<std::unique_ptr<ScanRange>, N> ranges) :
    bpt(bpt),
    ranges(move(ranges)) { }


template <std::size_t N>
void IndexScan<N>::begin(BindingId& input) {
    assert(ranges.size() == N && "Inconsistent size of ranges and bpt");

    my_binding = make_unique<BindingId>(input.var_count());
    my_input = &input;
    my_binding->add_all(*my_input);

    std::array<uint64_t, N> min_ids;
    std::array<uint64_t, N> max_ids;

    for (uint_fast32_t i = 0; i < N; ++i) {
        min_ids[i] = ranges[i]->get_min(input);
        max_ids[i] = ranges[i]->get_max(input);
    }

    it = bpt.get_range(
        Record(min_ids),
        Record(max_ids)
    );
    ++bpt_searches;
}


template <std::size_t N>
BindingId* IndexScan<N>::next() {
    if (it == nullptr)
        return nullptr;

    auto next = it->next();
    if (next != nullptr) {
        for (uint_fast32_t i = 0; i < N; ++i) {
            ranges[i]->try_assign(*my_binding, ObjectId(next->ids[i]));
        }
        ++results_found;
        return my_binding.get();
    } else {
        return nullptr;
    }
}


template <std::size_t N>
void IndexScan<N>::reset(BindingId& input) {
    my_input = &input;
    // TODO: if nulls were supported a my_binding->clean should be performed to set NULL_OBJECT_ID
    my_binding->add_all(*my_input);

    std::array<uint64_t, N> min_ids;
    std::array<uint64_t, N> max_ids;

    for (uint_fast32_t i = 0; i < N; ++i) {
        min_ids[i] = ranges[i]->get_min(input);
        max_ids[i] = ranges[i]->get_max(input);
    }

    it = bpt.get_range(
        Record<N>(min_ids),
        Record<N>(max_ids)
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
