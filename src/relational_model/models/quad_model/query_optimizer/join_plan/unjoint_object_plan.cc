#include "unjoint_object_plan.h"

#include "relational_model/execution/binding_id_iter/node_enum.h"

using namespace std;

UnjointObjectPlan::UnjointObjectPlan(QuadModel& model, VarId object_var_id) :
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
    cout << "LonelyNode(";// << var_names[node_var_id.id]
    cout << ")";

    if (estimated_cost) {
        cout << ",\n";
        for (int i = 0; i < indent; ++i) {
            cout << ' ';
        }
        cout << "  ↳ Estimated factor: " << estimate_output_size();
    }
}

double UnjointObjectPlan::estimate_output_size() {
    // TODO: remake
    return 1;
    // return catalog.get_node_count(graph_id);
}


uint64_t UnjointObjectPlan::get_vars() {
    return 1UL >> object_var_id.id;
}


void UnjointObjectPlan::set_input_vars(uint64_t) {
    // no need to do nothing
}


unique_ptr<BindingIdIter> UnjointObjectPlan::get_binding_id_iter() {
    // return make_unique<NodeEnum>(graph_id, node_var_id);
    // TODO: make sequence using catalog
    return nullptr;
}
