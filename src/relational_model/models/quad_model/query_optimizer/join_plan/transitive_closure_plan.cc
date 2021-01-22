#include "transitive_closure_plan.h"

#include <limits>

#include "relational_model/execution/binding_id_iter/transitive_closure_check.h"
#include "relational_model/execution/binding_id_iter/transitive_closure_enum.h"

using namespace std;

TransitiveClosurePlan::TransitiveClosurePlan(QuadModel& model, Id from, Id to, ObjectId type) :
    model         (model),
    from          (from),
    to            (to),
    type          (type),
    from_assigned (std::holds_alternative<ObjectId>(from)),
    to_assigned   (std::holds_alternative<ObjectId>(to)) { }


TransitiveClosurePlan::TransitiveClosurePlan(const TransitiveClosurePlan& other) :
    model         (other.model),
    from          (other.from),
    to            (other.to),
    type          (other.type),
    from_assigned (other.from_assigned),
    to_assigned   (other.to_assigned) { }


unique_ptr<JoinPlan> TransitiveClosurePlan::duplicate() {
    return make_unique<TransitiveClosurePlan>(*this);
}


double TransitiveClosurePlan::estimate_cost() {
    if (!to_assigned && !from_assigned) {
        return std::numeric_limits<double>::max();
    }
    return /*100.0 +*/ estimate_output_size();
}

void TransitiveClosurePlan::print(int indent, bool estimated_cost, std::vector<std::string>& var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "TransitiveClosure(";
    if (std::holds_alternative<ObjectId>(from)) {
        cout << "from: " << model.get_graph_object(std::get<ObjectId>(from));
    } else {
        cout << "from: " << var_names[std::get<VarId>(from).id];
    }

    if (std::holds_alternative<ObjectId>(to)) {
        cout << ", to: " << model.get_graph_object(std::get<ObjectId>(to));
    } else {
        cout << ", to: " << var_names[std::get<VarId>(to).id];
    }

    cout << ", type: " << model.get_graph_object(type);
    cout << ")";

    if (estimated_cost) {
        cout << ",\n";
        for (int i = 0; i < indent; ++i) {
            cout << ' ';
        }
        cout << "  ↳ Estimated factor: " << estimate_output_size();
    }
}

double TransitiveClosurePlan::estimate_output_size() {
    // TODO: find a better estimation
    const auto connections_with_type = static_cast<double>(
        model.catalog().connections_with_type(type.id)
    );
    return connections_with_type * connections_with_type;
}


uint64_t TransitiveClosurePlan::get_vars() {
    uint64_t result = 0;
    if ( std::holds_alternative<VarId>(from) ) {
        result |= 1UL << std::get<VarId>(from).id;
    }
    if ( std::holds_alternative<VarId>(to) ) {
        result |= 1UL << std::get<VarId>(to).id;
    }
    return result;
}


void TransitiveClosurePlan::set_input_vars(uint64_t input_vars) {
    if (std::holds_alternative<VarId>(from)) {
        auto from_var_id = std::get<VarId>(from);
        if ((input_vars & (1UL << from_var_id.id)) != 0) {
            from_assigned = true;
        }
    }
    if (std::holds_alternative<VarId>(to)) {
        auto to_var_id = std::get<VarId>(to);
        if ((input_vars & (1UL << to_var_id.id)) != 0) {
            to_assigned = true;
        }
    }
}


unique_ptr<BindingIdIter> TransitiveClosurePlan::get_binding_id_iter(std::size_t binding_size) {
    if (from_assigned) {
        if (to_assigned) {
            // bool case
            return make_unique<TransitiveClosureCheck>(binding_size, *model.type_from_to_edge, from, to, type, 1, 0);
        } else {
            // enum starting on from
            return make_unique<TransitiveClosureEnum>(binding_size, *model.type_from_to_edge, from, std::get<VarId>(to), type, 1, 0);
        }
    } else {
        if (to_assigned) {
            // enum starting on to
            return make_unique<TransitiveClosureEnum>(binding_size, *model.to_type_from_edge, to, std::get<VarId>(from), type, 0, 1);
        } else {
            throw runtime_error("transitive closure must have at least 1 node fixed");
        }
    }
}
