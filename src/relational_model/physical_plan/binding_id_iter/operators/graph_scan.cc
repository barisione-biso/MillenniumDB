#include "relational_model/physical_plan/binding_id_iter/operators/graph_scan.h"

#include "base/var/var_id.h"
#include "file/index/record.h"
#include "file/index/bplus_tree/bplus_tree.h"
#include "file/index/bplus_tree/bplus_tree_leaf.h"
#include "file/index/bplus_tree/bplus_tree_params.h"

using namespace std;

GraphScan::GraphScan(int graph_id, BPlusTree& bpt, vector<VarId> vars)
    : graph_id(graph_id), bpt(bpt), vars(vars)
{
    record_size = bpt.params.record_size;
}

void GraphScan::init(shared_ptr<BindingId const> input)
{
    this->input = input;
    auto min_ids = make_unique<uint64_t[]>(record_size);
    auto max_ids = make_unique<uint64_t[]>(record_size);

    if (input == nullptr) {
        int i = 0;
        for (; i < record_size; i++) {
            min_ids[i] = 0;
            max_ids[i] = UINT64_MAX;
        }
    }
    else {
        int i = 0;
        for (; i < record_size; i++) {
            const ObjectId* element_id = input->search_id(vars[i]);
            if (element_id != nullptr) {
                min_ids[i] = element_id->id;
                max_ids[i] = element_id->id;
            }
            else {
                for (; i < record_size; i++) {
                    min_ids[i] = 0;
                    max_ids[i] = UINT64_MAX;
                }
            }
        }
    }

    it = bpt.get_range(
        make_unique<Record>(min_ids.get(), record_size),
        make_unique<Record>(max_ids.get(), record_size)
    );
}

unique_ptr<BindingId const> GraphScan::next()
{
    auto next = it->next();
    if (next != nullptr) {
        auto res = make_unique<BindingId>();

        res->add(input->get_values());
        for (int i = 0; i < record_size; i++) {
            ObjectId element_id = ObjectId(graph_id, next->ids[i]);
            res->add(vars[i], element_id);
        }
        return res;
    }
    else return nullptr;
}

void GraphScan::reset(shared_ptr<BindingId const> input)
{
    init(input);
}
