#include "property_path_plan.h"

#include <limits>


#include "base/parser/logical_plan/op/op_path.h"
#include "relational_model/execution/binding_id_iter/property_paths/path_manager.h"
#include "relational_model/execution/binding_id_iter/property_paths/simple/property_path_bfs_check.h"
#include "relational_model/execution/binding_id_iter/property_paths/simple/property_path_bfs_simple_enum.h"

#include "relational_model/execution/binding_id_iter/property_paths/iter/property_path_bfs_iter_enum.h"
#include "relational_model/execution/binding_id_iter/property_paths/iter/property_path_dfs_iter_enum.h"
#include "relational_model/execution/binding_id_iter/property_paths/iter/property_path_a_star_iter_enum.h"


using namespace std;

using PropertyPathCheck = PropertyPathBFSCheck;
using PropertyPathEnum = PropertyPathBFSSimpleEnum;

PropertyPathPlan::PropertyPathPlan(QuadModel &model, VarId path_var, Id from, Id to, OpPath &path) :
    model         (model),
    path_var      (path_var),
    from          (from),
    to            (to),
    path          (path),
    from_assigned (std::holds_alternative<ObjectId>(from)),
    to_assigned   (std::holds_alternative<ObjectId>(to)) { }


PropertyPathPlan::PropertyPathPlan(const PropertyPathPlan &other) :
    model         (other.model),
    path_var      (other.path_var),
    from          (other.from),
    to            (other.to),
    path          (other.path),
    from_assigned (other.from_assigned),
    to_assigned   (other.to_assigned) { }


unique_ptr<JoinPlan> PropertyPathPlan::duplicate() {
    return make_unique<PropertyPathPlan>(*this);
}


double PropertyPathPlan::estimate_cost() {
    if (!to_assigned && !from_assigned) {
        return std::numeric_limits<double>::max();
    }
    return /*100.0 +*/ estimate_output_size();
}


void PropertyPathPlan::print(int indent, bool estimated_cost, std::vector<std::string> &var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathPlan(";
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
    // TODO: Print del to_string del property_paths
    cout << ", Path: " <<  var_names[path_var.id];
    cout << ")";

    if (estimated_cost) {
        cout << ",\n";
        for (int i = 0; i < indent; ++i) {
            cout << ' ';
        }
        cout << "  ↳ Estimated factor: " << estimate_output_size();
    }
}


double PropertyPathPlan::estimate_output_size() {
    // TODO: find a better estimation
    const auto total_connections = static_cast<double>(
        model.catalog().connections_count);
    return total_connections * total_connections;
}


uint64_t PropertyPathPlan::get_vars() {
    uint64_t result = 0;
    if (std::holds_alternative<VarId>(from)) {
        result |= 1UL << std::get<VarId>(from).id;
    }
    if (std::holds_alternative<VarId>(to)) {
        result |= 1UL << std::get<VarId>(to).id;
    }
    return result;
}


void PropertyPathPlan::set_input_vars(uint64_t input_vars) {
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


unique_ptr<BindingIdIter> PropertyPathPlan::get_binding_id_iter(std::size_t) {
    if (from_assigned) {
        auto automaton = path.get_transformed_automaton();
        transform_automaton(automaton);
        if (to_assigned) {
            // bool case
            return make_unique<PropertyPathCheck>(*model.type_from_to_edge,
                                                  *model.to_type_from_edge,
                                                  path_var,
                                                  from,
                                                  to,
                                                  automaton);
        } else {
            // enum starting on from
            return make_unique<PropertyPathEnum>(*model.type_from_to_edge,
                                                 *model.to_type_from_edge,
                                                 path_var,
                                                 from,
                                                 std::get<VarId>(to),
                                                 automaton);
        }
    } else {
        if (to_assigned) {
            // enum starting on to
            auto inverted_path = path.invert();
            auto automaton = inverted_path->get_transformed_automaton();
            transform_automaton(automaton);
            return make_unique<PropertyPathEnum>(*model.type_from_to_edge,
                                                 *model.to_type_from_edge,
                                                 path_var,
                                                 to,
                                                 std::get<VarId>(from),
                                                 automaton);
        } else {
            throw runtime_error("property path must have at least 1 node fixed");
        }
    }
    return nullptr;
}


// TODO: Change name and explain
void PropertyPathPlan::transform_automaton(PathAutomaton &automaton) {
    for (size_t i = 0; i < automaton.from_to_connections.size(); i++) {
        vector<TransitionId> transition_id_vector;
        for (const auto &t : automaton.from_to_connections[i]) {
            transition_id_vector.push_back(
                TransitionId(t.to, model.get_identifiable_object_id(t.label), t.inverse));
        }
        automaton.transitions.push_back(transition_id_vector);
    }
}
