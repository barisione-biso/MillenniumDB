#include "label_plan.h"

#include <cassert>

#include "relational_model/execution/binding_id_iter/index_scan.h"

using namespace std;

LabelPlan::LabelPlan(QuadModel& model, Id node, Id label) :
    model          (model),
    node           (node),
    label          (label),
    node_assigned  (std::holds_alternative<ObjectId>(node)),
    label_assigned (std::holds_alternative<ObjectId>(label)) { }


LabelPlan::LabelPlan(const LabelPlan& other) :
    model          (other.model),
    node           (other.node),
    label          (other.label),
    node_assigned  (other.node_assigned),
    label_assigned (other.label_assigned) { }


std::unique_ptr<JoinPlan> LabelPlan::duplicate() {
    return make_unique<LabelPlan>(*this);
}


void LabelPlan::print(int indent, bool estimated_cost, std::vector<std::string>& var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "Label(";
    if (std::holds_alternative<ObjectId>(node)) {
        cout << "node: " << model.get_graph_object(std::get<ObjectId>(node)) << "";
    } else {
        cout << "node: " <<  var_names[std::get<VarId>(node).id] << "";
    }

    if (std::holds_alternative<ObjectId>(label)) {
        cout << ", label: " << model.get_graph_object(std::get<ObjectId>(label)) << "";
    } else {
        cout << ", label: " << var_names[std::get<VarId>(label).id] << "";
    }
    cout << ")";

    if (estimated_cost) {
        cout << ",\n";
        for (int i = 0; i < indent; ++i) {
            cout << ' ';
        }
        cout << "  ↳ Estimated factor: " << estimate_output_size();
    }
}


double LabelPlan::estimate_cost() {
    return /*100.0 +*/ estimate_output_size();
}


double LabelPlan::estimate_output_size() {
    const auto total_nodes = static_cast<double>(model.catalog().identifiable_nodes_count
                                               + model.catalog().anonymous_nodes_count);

    auto total_labels      = static_cast<double>(model.catalog().label_count);


    if (total_nodes == 0) { // to avoid division by 0
        return 0;
    }

    if (label_assigned) {
        // nodes with label `label_id`
        double label_count;
        if (std::holds_alternative<ObjectId>(label)) {
            label_count = static_cast<double>(model.catalog().label2total_count[std::get<ObjectId>(label).id]);
        } else {
            // TODO: this case is not possible yet, but we need to cover it for the future
            return 0;
        }

        if (node_assigned) {
            return label_count / total_nodes;
        } else {
            return label_count;
        }
    } else {
        if (node_assigned) {
            return total_labels / total_nodes;
        } else {
            return total_nodes;
        }
    }
}


void LabelPlan::set_input_vars(const uint64_t input_vars) {
    if (std::holds_alternative<VarId>(node)) {
        auto node_var_id = std::get<VarId>(node);
        if ((input_vars & (1UL << node_var_id.id)) != 0) {
            node_assigned = true;
        }
    }
    if (std::holds_alternative<VarId>(label)) {
        auto label_var_id = std::get<VarId>(label);
        if ((input_vars & (1UL << label_var_id.id)) != 0) {
            label_assigned = true;
        }
    }
}

// Must be consistent with the index scan returned in get_binding_id_iter()
uint64_t LabelPlan::get_vars() {
    uint64_t result = 0;

    if ( std::holds_alternative<VarId>(node) ) {
        result |= 1UL << std::get<VarId>(node).id;
    }
    if ( std::holds_alternative<VarId>(label) ) {
        result |= 1UL << std::get<VarId>(label).id;
    }
    return result;
}


/**
 * ╔═╦═══════════════╦═════════════════╦═════════╗
 * ║ ║ Node Assigned ║  Label Assigned ║  Index  ║
 * ╠═╬═══════════════╬═════════════════╬═════════╣
 * ║1║       yes     ║       yes       ║    NL   ║
 * ║2║       yes     ║       no        ║    NL   ║
 * ║3║       no      ║       yes       ║    LN   ║
 * ║4║       no      ║       no        ║    LN   ║
 * ╚═╩═══════════════╩═════════════════╩═════════╝
 */
unique_ptr<BindingIdIter> LabelPlan::get_binding_id_iter(std::size_t binding_size) {
    array<unique_ptr<ScanRange>, 2> ranges;
    if (node_assigned) {
        ranges[0] = ScanRange::get(node, node_assigned);
        ranges[1] = ScanRange::get(label, label_assigned);
        return make_unique<IndexScan<2>>(binding_size, *model.node_label, move(ranges));
    } else {
        ranges[0] = ScanRange::get(label, label_assigned);
        ranges[1] = ScanRange::get(node, node_assigned);
        return make_unique<IndexScan<2>>(binding_size, *model.label_node, move(ranges));
    }
}


unique_ptr<LeapfrogIter> LabelPlan::get_leapfrog_iter(const vector<VarId>& global_intersection_vars) {
    vector<ObjectId> terms;
    vector<VarId> intersection_vars;
    vector<VarId> enumeration_vars;

    // index = INT32_MAX means enumeration, index = -1 means term
    int_fast32_t node_index  = std::holds_alternative<ObjectId>(node)  ? -1 : INT32_MAX;
    int_fast32_t label_index = std::holds_alternative<ObjectId>(label) ? -1 : INT32_MAX;

    // set index if they are in global_intersection_vars
    for (size_t i = 0; i < global_intersection_vars.size(); i++) {
        if (node_index == INT32_MAX && std::get<VarId>(node) == global_intersection_vars[i]) {
            node_index = i;
        }
        if (label_index == INT32_MAX && std::get<VarId>(label) == global_intersection_vars[i]) {
            label_index = i;
        }
    }

    auto assign = [&terms, &enumeration_vars, &intersection_vars](int_fast32_t& index, Id id) -> void {
        if (index == -1) {
            terms.push_back(std::get<ObjectId>(id));
        } else if (index == INT32_MAX) {
            enumeration_vars.push_back(std::get<VarId>(id));
        } else {
            intersection_vars.push_back(std::get<VarId>(id));
        }
    };

    // node_label
    if (node_index <= label_index) {
        assign(node_index, node);
        assign(label_index, label);

        return make_unique<LeapfrogIterImpl<2>>(
            *model.node_label,
            move(terms),
            move(intersection_vars),
            move(enumeration_vars)
        );
    }
    // to_type_from_edge
    else {
        assign(label_index, label);
        assign(node_index, node);

        return make_unique<LeapfrogIterImpl<2>>(
            *model.label_node,
            move(terms),
            move(intersection_vars),
            move(enumeration_vars)
        );
    }
}