#include "connection_plan.h"

#include "relational_model/execution/binding_id_iter/index_scan.h"

using namespace std;

ConnectionPlan::ConnectionPlan(QuadModel& model, Id from, Id to, Id type, Id edge) :
    model         (model),
    from          (from),
    to            (to),
    type          (type),
    edge          (edge),
    from_assigned (std::holds_alternative<ObjectId>(from)),
    to_assigned   (std::holds_alternative<ObjectId>(to)),
    type_assigned (std::holds_alternative<ObjectId>(type)),
    edge_assigned (std::holds_alternative<ObjectId>(edge)) { }


ConnectionPlan::ConnectionPlan(const ConnectionPlan& other) :
    model         (other.model),
    from          (other.from),
    to            (other.to),
    type          (other.type),
    edge          (other.edge),
    from_assigned (other.from_assigned),
    to_assigned   (other.to_assigned),
    type_assigned (other.type_assigned),
    edge_assigned (other.edge_assigned) { }


std::unique_ptr<JoinPlan> ConnectionPlan::duplicate() {
    return make_unique<ConnectionPlan>(*this);
}


void ConnectionPlan::print(int indent, bool estimated_cost, std::vector<std::string>& var_names) {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "Connection(";
    if (std::holds_alternative<ObjectId>(from)) {
        cout << "from: " << model.get_graph_object(std::get<ObjectId>(from))->to_string() << "";
    } else {
        cout << "from: " <<  var_names[std::get<VarId>(from).id] << "";
    }
    if (std::holds_alternative<ObjectId>(to)) {
        cout << ", to: " << model.get_graph_object(std::get<ObjectId>(to))->to_string() << "";
    } else {
        cout << ", to: " <<  var_names[std::get<VarId>(to).id] << "";
    }
    if (std::holds_alternative<ObjectId>(type)) {
        cout << ", type: " << model.get_graph_object(std::get<ObjectId>(type))->to_string() << "";
    } else {
        cout << ", type: " <<  var_names[std::get<VarId>(type).id] << "";
    }
    // cout << "?" << var_names[edge_var_id.id]
    //  << ", ?" << var_names[node_from_var_id.id]
    //  << ", ?" << var_names[node_to_var_id.id]
    cout << ")";

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
    const auto total_connections = static_cast<double>(model.catalog().connections_count);

    const auto total_objects     = static_cast<double>(model.catalog().identifiable_nodes_count
                                                     + model.catalog().anonymous_nodes_count
                                                     + model.catalog().connections_count);

    if (total_connections == 0 || total_objects == 0) { // to avoid division by 0
        return 0;
    }
    // TODO: think again
    if (from_assigned) {
        if (to_assigned) {
            if (type_assigned) {
                return 1.0 / (total_connections * total_objects * total_objects);
            } else {
                return total_connections / (total_objects * total_objects);
            }
        } else {
            if (type_assigned) {
                return 1.0 / (total_connections * total_objects);
            } else {
                return total_connections / total_objects;
            }
        }
    } else {
        if (to_assigned) {
            if (type_assigned) {
                return 1.0 / (total_connections * total_objects);
            } else {
                return total_connections / total_objects;
            }
        } else {
            if (type_assigned) {
                return 1;
            } else {
                return total_connections;
            }
        }
    }
}


void ConnectionPlan::set_input_vars(const uint64_t input_vars) {
    if (std::holds_alternative<VarId>(from)) {
        auto from_var_id = std::get<VarId>(from);
        assert(from_var_id.id >= 0 && "Inconsistent VarId");
        if ((input_vars & (1UL << from_var_id.id)) != 0) {
            from_assigned = true;
        }
    }
    if (std::holds_alternative<VarId>(to)) {
        auto to_var_id = std::get<VarId>(to);
        assert(to_var_id.id >= 0 && "Inconsistent VarId");
        if ((input_vars & (1UL << to_var_id.id)) != 0) {
            to_assigned = true;
        }
    }
    if (std::holds_alternative<VarId>(type)) {
        auto type_var_id = std::get<VarId>(type);
        assert(type_var_id.id >= 0 && "Inconsistent VarId");
        if ((input_vars & (1UL << type_var_id.id)) != 0) {
            type_assigned = true;
        }
    }
    if (std::holds_alternative<VarId>(edge)) {
        auto edge_var_id = std::get<VarId>(edge);
        assert(edge_var_id.id >= 0 && "Inconsistent VarId");
        if ((input_vars & (1UL << edge_var_id.id)) != 0) {
            edge_assigned = true;
        }
    }
}

// Must be consistent with the index scan returned in get_binding_id_iter()
uint64_t ConnectionPlan::get_vars() {
    uint64_t result = 0;
    if ( std::holds_alternative<VarId>(from) ) {
        assert(std::get<VarId>(from).id >= 0 && "Inconsistent VarId");
        result |= 1UL << std::get<VarId>(from).id;
    }
    if ( std::holds_alternative<VarId>(to) ) {
        assert(std::get<VarId>(to).id >= 0 && "Inconsistent VarId");
        result |= 1UL << std::get<VarId>(to).id;
    }
    if ( std::holds_alternative<VarId>(type) ) {
        assert(std::get<VarId>(type).id >= 0 && "Inconsistent VarId");
        result |= 1UL << std::get<VarId>(type).id;
    }
    if ( std::holds_alternative<VarId>(edge) ) {
        assert(std::get<VarId>(edge).id >= 0 && "Inconsistent VarId");
        result |= 1UL << std::get<VarId>(edge).id;
    }
    return result;
}


/** FTYE | TYFE | YFTE
 * ╔═╦══════════════╦════════════╦═══════════════╦═════════════════╦══════════╗
 * ║ ║ FromAssigned ║ ToAssigned ║ tYpeAssigned  ║  EdgeAssigned   ║  index   ║
 * ╠═╬══════════════╬════════════╬═══════════════╬═════════════════╬══════════╣
 * ║1║      yes     ║     yes    ║      yes      ║      no         ║  YFTE    ║
 * ║2║      yes     ║     yes    ║      no       ║      no         ║  FTYE    ║
 * ║3║      yes     ║     no     ║      yes      ║      no         ║  YFTE    ║
 * ║4║      yes     ║     no     ║      no       ║      no         ║  FTYE    ║
 * ║5║      no      ║     yes    ║      yes      ║      no         ║  TYFE    ║
 * ║6║      no      ║     yes    ║      no       ║      no         ║  TYFE    ║
 * ║7║      no      ║     no     ║      yes      ║      no         ║  YFTE    ║
 * ║8║      no      ║     no     ║      no       ║      no         ║  YFTE    ║
 * ║9║      *       ║     *      ║      *        ║      yes        ║  table   ║
 * ╚═╩══════════════╩════════════╩═══════════════╩═════════════════╩══════════╝
 */
unique_ptr<BindingIdIter> ConnectionPlan::get_binding_id_iter() {
    array<unique_ptr<ScanRange>, 4> ranges;

    if (edge_assigned) {
        // TODO:
    }

    ranges[3] = get_scan_range(edge, edge_assigned);
    // if (from_assigned)
    //     cout << "from assigned\n";
    // if (to_assigned)
    //     cout << "to assigned\n";
    // if (type_assigned)
    //     cout << "type assigned\n";
    // if (edge_assigned)
    //     cout << "edge assigned\n";

    if (from_assigned) {
        if (type_assigned) { // CASES 1 and 3 => YFTE
            // cout << "using type from to edge\n";
            ranges[0] = get_scan_range(type, type_assigned);
            ranges[1] = get_scan_range(from, from_assigned);
            ranges[2] = get_scan_range(to,   to_assigned);

            return make_unique<IndexScan<4>>(*model.type_from_to_edge, move(ranges));
        } else { // CASES 2 and 4 => FTYE
            // cout << "using from to type edge\n";
            ranges[0] = get_scan_range(from, from_assigned);
            ranges[1] = get_scan_range(to,   to_assigned);
            ranges[2] = get_scan_range(type, type_assigned);

            return make_unique<IndexScan<4>>(*model.from_to_type_edge, move(ranges));
        }
    } else {
        if (to_assigned) { // CASES 5 and 6 => TYFE
            // cout << "using to type from edge\n";
            ranges[0] = get_scan_range(to,   to_assigned);
            ranges[1] = get_scan_range(type, type_assigned);
            ranges[2] = get_scan_range(from, from_assigned);

            return make_unique<IndexScan<4>>(*model.to_type_from_edge, move(ranges));
        } else { // CASES 7 and 8 => YFTE
            // cout << "using type from to edge\n";
            ranges[0] = get_scan_range(type, type_assigned);
            ranges[1] = get_scan_range(from, from_assigned);
            ranges[2] = get_scan_range(to,   to_assigned);

            return make_unique<IndexScan<4>>(*model.type_from_to_edge, move(ranges));
        }
    }
}
