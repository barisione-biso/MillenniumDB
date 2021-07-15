#include "unjoint_object_plan.h"

#include "relational_model/execution/binding_id_iter/node_table_enum.h"
#include "relational_model/execution/binding_id_iter/object_enum.h"
#include "relational_model/execution/binding_id_iter/union.h"
#include "relational_model/execution/binding_id_iter/index_scan.h"

using namespace std;

UnjointObjectPlan::UnjointObjectPlan(const QuadModel& model, const VarId object_var_id) :
    model         (model),
    object_var_id (object_var_id) { }


UnjointObjectPlan::UnjointObjectPlan(const UnjointObjectPlan& other) :
    model         (other.model),
    object_var_id (other.object_var_id) { }


unique_ptr<JoinPlan> UnjointObjectPlan::duplicate() {
    return make_unique<UnjointObjectPlan>(*this);
}


double UnjointObjectPlan::estimate_cost() {
    return /*100.0 +*/ estimate_output_size();
}

void UnjointObjectPlan::print(int indent, bool estimated_cost, std::vector<std::string>& var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "UnjointObject("  << var_names[object_var_id.id] << ")";

    if (estimated_cost) {
        cout << ",\n";
        for (int i = 0; i < indent; ++i) {
            cout << ' ';
        }
        cout << "  ↳ Estimated factor: " << estimate_output_size();
    }
}

double UnjointObjectPlan::estimate_output_size() {
    return model.catalog().connections_count
           + model.catalog().identifiable_nodes_count
           + model.catalog().anonymous_nodes_count;
}


uint64_t UnjointObjectPlan::get_vars() {
    return 1UL << object_var_id.id;
}


void UnjointObjectPlan::set_input_vars(uint64_t) {
    // no need to do nothing
}


unique_ptr<BindingIdIter> UnjointObjectPlan::get_binding_id_iter() {
    vector<unique_ptr<BindingIdIter>> iters;
    array<unique_ptr<ScanRange>, 1> ranges;
    ranges[0] = make_unique<UnassignedVar>(object_var_id);
    iters.push_back(
        make_unique<IndexScan<1>>(*model.nodes, move(ranges))
    );
    iters.push_back(
        make_unique<ObjectEnum>(object_var_id, QuadModel::ANONYMOUS_NODE_MASK, model.catalog().anonymous_nodes_count)
    );
    iters.push_back(
        make_unique<ObjectEnum>(object_var_id, QuadModel::CONNECTION_MASK, model.catalog().connections_count)
    );
    return make_unique<Union>(move(iters));
}
