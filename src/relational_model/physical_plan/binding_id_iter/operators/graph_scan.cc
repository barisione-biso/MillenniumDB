#include "relational_model/physical_plan/binding_id_iter/operators/graph_scan.h"

#include "base/var/var_id.h"
#include "file/index/record.h"
#include "file/index/bplus_tree/bplus_tree.h"
#include "file/index/bplus_tree/bplus_tree_leaf.h"
#include "file/index/bplus_tree/bplus_tree_params.h"

#include <vector>

using namespace std;

GraphScan::GraphScan(BPlusTree& bpt, std::vector<std::pair<ObjectId, int>> terms,
    std::vector<std::pair<VarId, int>> vars)
    : record_size(bpt.params.total_size), bpt(bpt), terms(terms), vars(vars)
     // TODO: use move for vectors?
{ }


void GraphScan::init(shared_ptr<BindingId> input) {
    this->input = input;

    if (input == nullptr) {
        it = nullptr;
        return;
    }

    vector<uint64_t> min_ids(record_size);
    vector<uint64_t> max_ids(record_size);

    for (int i = 0; i < record_size; i++) {
        min_ids[i] = 0;
        max_ids[i] = UINT64_MAX;
    }

    for (auto& term : terms) {
        min_ids[term.second] = term.first;
        max_ids[term.second] = term.first;
    }

    for (auto& var : vars) {
        auto obj_id = (*input)[var.first];
        if (obj_id.is_null()) {
            break;
        }
        else {
            min_ids[var.second] = obj_id;
            max_ids[var.second] = obj_id;
        }
    }

    it = bpt.get_range(
        Record(min_ids),
        Record(max_ids)
    );
}

unique_ptr<BindingId> GraphScan::next()
{
    if (it == nullptr)
        return nullptr;

    auto next = it->next();
    if (next != nullptr) {
        auto res = make_unique<BindingId>(input->var_count());
        res->add_all(*input);
        for (auto & var : vars) {
            ObjectId element_id = ObjectId(next->ids[var.second]);
            res->add(var.first, element_id);
        }
        return res;
    }
    else return nullptr;
}

void GraphScan::reset(shared_ptr<BindingId> input) {
    init(input);
}
