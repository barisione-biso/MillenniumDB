#include "connection_plan.h"

#include "relational_model/execution/binding_id_iter/index_scan.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/assigned_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/default_graph_var.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/term.h"

using namespace std;

ConnectionPlan::ConnectionPlan(GraphId graph_id, VarId node_from_var_id, VarId node_to_var_id,
                               VarId edge_var_id) :
    graph_id(graph_id),
    node_from_var_id(node_from_var_id),
    node_to_var_id(node_to_var_id),
    edge_var_id(edge_var_id),
    node_from_assigned(false),
    node_to_assigned(false),
    edge_assigned(false) { }


ConnectionPlan::ConnectionPlan(const ConnectionPlan& other) :
    graph_id(other.graph_id),
    node_from_var_id(other.node_from_var_id),
    node_to_var_id(other.node_to_var_id),
    edge_var_id(other.edge_var_id),
    node_from_assigned(other.node_from_assigned),
    node_to_assigned(other.node_to_assigned),
    edge_assigned(other.edge_assigned) { }


std::unique_ptr<JoinPlan> ConnectionPlan::duplicate() {
    return make_unique<ConnectionPlan>(*this);
}


void ConnectionPlan::print(int indent, bool estimated_cost, std::vector<std::string>& var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "Connection(?" << var_names[edge_var_id.id]
         << ", ?" << var_names[node_from_var_id.id]
         << ", ?" << var_names[node_to_var_id.id]
         << ")";

    if (estimated_cost) {
        cout << ",\n";
        for (int i = 0; i < indent; ++i) {
            cout << ' ';
        }
        cout << "  ↳ Estimated factor: " << estimate_output_size();
    }
}


double ConnectionPlan::estimate_cost() {
    // return 1 + estimate_output_size();
    // d: cost of traveling down the B+Tree;
    // t: cost per tuple;
    // n: estimated output size
    // return d + t*n;
    return /*100.0 +*/ estimate_output_size();
}


double ConnectionPlan::estimate_output_size() {
    // TODO: remake
    return 1;
    // auto total_connections = static_cast<double>(catalog.get_edge_count(graph_id));
    // auto total_nodes       = static_cast<double>(catalog.get_node_count(graph_id));

    // if (total_connections == 0 || total_nodes == 0) { // to avoid division by 0
    //     return 0;
    // }
    // if (node_from_assigned) {
    //     if (node_to_assigned) {
    //         if (edge_assigned) {
    //             return 1.0 / (total_connections * total_nodes * total_nodes);
    //         } else {
    //             return total_connections / (total_nodes * total_nodes);
    //         }
    //     } else {
    //         if (edge_assigned) {
    //             return 1.0 / (total_connections * total_nodes);
    //         } else {
    //             return total_connections / total_nodes;
    //         }
    //     }
    // } else {
    //     if (node_to_assigned) {
    //         if (edge_assigned) {
    //             return 1.0 / (total_connections * total_nodes);
    //         } else {
    //             return total_connections / total_nodes;
    //         }
    //     } else {
    //         if (edge_assigned) {
    //             return 1;
    //         } else {
    //             return total_connections;
    //         }
    //     }
    // }
}


void ConnectionPlan::set_input_vars(std::vector<VarId>& input_var_order) {
    for (auto& input_var : input_var_order) {
        if (node_to_var_id == input_var) {
            node_to_assigned = true;
        } else if (node_from_var_id == input_var) {
            node_from_assigned = true;
        } else if (edge_var_id == input_var) {
            edge_assigned = true;
        }
    }
}

// Must be consistent with the index scan returned in get_binding_id_iter()
vector<VarId> ConnectionPlan::get_var_order() {
    vector<VarId> result;
    if (node_from_assigned) {
        if (edge_assigned) { // CASES 1 and 2 => EFT
            result.push_back(edge_var_id);
            result.push_back(node_from_var_id);
            result.push_back(node_to_var_id);
        } else { // CASES 3 and 4 => FTE
            result.push_back(node_from_var_id);
            result.push_back(node_to_var_id);
            result.push_back(edge_var_id);
        }
    } else {
        if (node_to_assigned) { // CASES 5 and 7 => TEF
            result.push_back(node_to_var_id);
            result.push_back(edge_var_id);
            result.push_back(node_from_var_id);
        } else { // CASES 6 and 8 => EFT
            result.push_back(edge_var_id);
            result.push_back(node_from_var_id);
            result.push_back(node_to_var_id);
        }
    }
    return result;
}


/** FTE | TEF | EFT
 * ╔═╦══════════╦════════╦══════════╦══════════╗
 * ║ ║ NodeFrom ║  Type  ║  NodeTo  ║  index   ║
 * ╠═╬══════════╬════════╬══════════╬══════════╣
 * ║1║     yes  ║   yes  ║    yes   ║    EFT   ║
 * ║2║     yes  ║   yes  ║    no    ║    EFT   ║
 * ║3║     yes  ║   no   ║    yes   ║    FTE   ║
 * ║4║     yes  ║   no   ║    no    ║    FTE   ║
 * ║5║     no   ║   yes  ║    yes   ║    TEF   ║
 * ║6║     no   ║   yes  ║    no    ║    EFT   ║
 * ║7║     no   ║   no   ║    yes   ║    TEF   ║
 * ║8║     no   ║   no   ║    no    ║    EFT   ║
 * ║8║     no   ║   no   ║    no    ║    EFT   ║
 * ╚═╩══════════╩════════╩══════════╩══════════╝
 */
unique_ptr<BindingIdIter> ConnectionPlan::get_binding_id_iter() {
    array<unique_ptr<ScanRange>, 3> ranges;

    // TODO: remake
    return nullptr;
    // if (node_from_assigned) {
    //     if (edge_assigned) { // CASES 1 and 2 => EFT
    //         ranges[0] = get_edge_range();
    //         ranges[1] = get_node_from_range();
    //         ranges[2] = get_node_to_range();

    //         return make_unique<IndexScan<3>>(model.get_edge_from_to(), move(ranges));
    //     } else { // CASES 3 and 4 => FTE
    //         ranges[0] = get_node_from_range();
    //         ranges[1] = get_node_to_range();
    //         ranges[2] = get_edge_range();

    //         return make_unique<IndexScan<3>>(model.get_from_to_edge(), move(ranges));
    //     }
    // } else {
    //     if (node_to_assigned) { // CASES 5 and 7 => TEF
    //         ranges[0] = get_node_to_range();
    //         ranges[1] = get_edge_range();
    //         ranges[2] = get_node_from_range();

    //         return make_unique<IndexScan<3>>(model.get_to_edge_from(), move(ranges));
    //     } else { // CASES 6 and 8 => EFT
    //         ranges[0] = get_edge_range();
    //         ranges[1] = get_node_from_range();
    //         ranges[2] = get_node_to_range();

    //         return make_unique<IndexScan<3>>(model.get_edge_from_to(), move(ranges));
    //     }
    // }
}


std::unique_ptr<ScanRange> ConnectionPlan::get_node_from_range() {
    if (node_from_assigned) {
        return make_unique<AssignedVar>(node_from_var_id);
    } else {
        return make_unique<DefaultGraphVar>(node_from_var_id);
    }
}


std::unique_ptr<ScanRange> ConnectionPlan::get_node_to_range() {
    if (node_to_assigned) {
        return make_unique<AssignedVar>(node_to_var_id);
    } else {
        return make_unique<DefaultGraphVar>(node_to_var_id);
    }
}


std::unique_ptr<ScanRange> ConnectionPlan::get_edge_range() {
    if (edge_assigned) {
        return make_unique<AssignedVar>(edge_var_id);
    } else {
        return make_unique<DefaultGraphVar>(edge_var_id);
    }
}
