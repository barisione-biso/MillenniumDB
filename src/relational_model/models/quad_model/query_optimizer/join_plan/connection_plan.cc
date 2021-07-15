#include "connection_plan.h"

#include <algorithm>

#include "relational_model/execution/binding_id_iter/edge_table_lookup.h"
#include "relational_model/execution/binding_id_iter/index_scan.h"

using namespace std;

ConnectionPlan::ConnectionPlan(const QuadModel& model, Id from, Id to, Id type, VarId edge) :
    model         (model),
    from          (from),
    to            (to),
    type          (type),
    edge          (edge),
    from_assigned (std::holds_alternative<ObjectId>(from)),
    to_assigned   (std::holds_alternative<ObjectId>(to)),
    type_assigned (std::holds_alternative<ObjectId>(type)),
    edge_assigned (false) { }


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
        cout << "from: " << model.get_graph_object(std::get<ObjectId>(from));
    } else {
        cout << "from: " << var_names[std::get<VarId>(from).id];
    }
    if (std::holds_alternative<ObjectId>(to)) {
        cout << ", to: " << model.get_graph_object(std::get<ObjectId>(to));
    } else {
        cout << ", to: " << var_names[std::get<VarId>(to).id];
    }
    if (std::holds_alternative<ObjectId>(type)) {
        cout << ", type: " << model.get_graph_object(std::get<ObjectId>(type));
    } else {
        cout << ", type: " << var_names[std::get<VarId>(type).id];
    }
    cout << ", edge: " << var_names[edge.id];
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
    const auto distinct_from     = static_cast<double>(model.catalog().distinct_from);
    const auto distinct_to       = static_cast<double>(model.catalog().distinct_to);
    const auto distinct_type     = static_cast<double>(model.catalog().distinct_type);

    if (distinct_from == 0 || distinct_to == 0) { // to avoid division by 0
        return 0;
    } else if (edge_assigned) {
        return 1 / (distinct_from * distinct_to * distinct_type);
    }
    // check for special cases
    if (from == to) {
        if (from == type) {
            // from == to == type
            if (std::holds_alternative<ObjectId>(type)) {
                const auto special_connections_with_type = static_cast<double>(
                    model.catalog().equal_from_to_type_with_type(std::get<ObjectId>(type).id)
                );
                return special_connections_with_type;
            } else {
                const auto special_connections = static_cast<double>(model.catalog().equal_from_to_type_count);
                if (from_assigned) {
                    return special_connections /
                           std::max<double>(distinct_from, std::max<double>(distinct_to, distinct_type));
                } else {
                    return special_connections;
                }
            }
        } else {
            // from == to
            if (std::holds_alternative<ObjectId>(type)) {
                const auto special_connections_with_type = static_cast<double>(
                    model.catalog().equal_from_to_with_type(std::get<ObjectId>(type).id)
                );
                if (from_assigned) {
                    return special_connections_with_type / distinct_from;
                } else if (to_assigned) {
                    return special_connections_with_type / distinct_to;
                } else {
                    return special_connections_with_type;
                }
            } else {
                const auto special_connections = static_cast<double>(model.catalog().equal_from_to_count);
                if (from_assigned) {
                    return special_connections / std::max<double>(distinct_from, distinct_to);
                } else {
                    return special_connections;
                }
            }
        }
    } else if (from == type) {
        if (std::holds_alternative<ObjectId>(type)) {
            const auto special_connections_with_type = static_cast<double>(
                model.catalog().equal_from_type_with_type(std::get<ObjectId>(type).id)
            );
            if (to_assigned) {
                return special_connections_with_type / distinct_to;
            } else {
                return special_connections_with_type;
            }
        } else {
            const auto special_connections = static_cast<double>(model.catalog().equal_from_type_count);
            if (from_assigned) {
                return special_connections /  std::max<double>(distinct_from, distinct_type);
            } else {
                return special_connections;
            }
        }
    }
    else if (to == type) {
        if (std::holds_alternative<ObjectId>(type)) {
            const auto special_connections_with_type = static_cast<double>(
                model.catalog().equal_to_type_with_type(std::get<ObjectId>(type).id)
            );
            if (from_assigned) {
                return special_connections_with_type / distinct_from;
            } else {
                return special_connections_with_type;
            }

        } else {
            const auto special_connections = static_cast<double>(model.catalog().equal_to_type_count);
            if (to_assigned) {
                return special_connections /  std::max<double>(distinct_to, distinct_type);
            } else {
                return special_connections;
            }
        }
    } // end special cases
    else if (type_assigned) {
        if (std::holds_alternative<ObjectId>(type)) {
            const auto connections_with_type = static_cast<double>(
                model.catalog().connections_with_type(std::get<ObjectId>(type).id)
            );
            if (from_assigned) {
                if (to_assigned) {
                    return connections_with_type / (distinct_from * distinct_to);
                } else {
                    return connections_with_type / (distinct_from);
                }
            } else {
                if (to_assigned) {
                    return connections_with_type / (distinct_to);
                } else {
                    return connections_with_type;
                }
            }
        } else {
            if (from_assigned) {
                if (to_assigned) {
                    return total_connections / (distinct_from * distinct_to * distinct_type);
                } else {
                    return total_connections / (distinct_from * distinct_type);
                }
            } else {
                if (to_assigned) {
                    return total_connections / (distinct_to * distinct_type);
                } else {
                    return total_connections / distinct_type;
                }
            }
        }
    } else {
        if (from_assigned) {
            if (to_assigned) {
                return total_connections / (distinct_from * distinct_to);
            } else {
                return total_connections / distinct_from;
            }
        } else {
            if (to_assigned) {
                return total_connections / distinct_to;
            } else {
                return total_connections;
            }
        }
    }
}


void ConnectionPlan::set_input_vars(const uint64_t input_vars) {
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
    if (std::holds_alternative<VarId>(type)) {
        auto type_var_id = std::get<VarId>(type);
        if ((input_vars & (1UL << type_var_id.id)) != 0) {
            type_assigned = true;
        }
    }

    if ((input_vars & (1UL << edge.id)) != 0) {
        edge_assigned = true;
    }
}

// Must be consistent with the index scan returned in get_binding_id_iter()
uint64_t ConnectionPlan::get_vars() {
    uint64_t result = 0;
    if ( std::holds_alternative<VarId>(from) ) {
        result |= 1UL << std::get<VarId>(from).id;
    }
    if ( std::holds_alternative<VarId>(to) ) {
        result |= 1UL << std::get<VarId>(to).id;
    }
    if ( std::holds_alternative<VarId>(type) ) {
        result |= 1UL << std::get<VarId>(type).id;
    }

    result |= 1UL << edge.id;

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

    if (edge_assigned) {
        return make_unique<EdgeTableLookup>(*model.edge_table, edge, from, to, type);
    }
    // check for special cases
    if (from == to) {
        if (from == type) {
            // equal_from_to_type
            array<unique_ptr<ScanRange>, 2> ranges;
            ranges[0] = ScanRange::get(from, from_assigned);
            ranges[1] = ScanRange::get(edge, edge_assigned);
            return make_unique<IndexScan<2>>(*model.equal_from_to_type, move(ranges));
        } else {
            // equal_from_to
            array<unique_ptr<ScanRange>, 3> ranges;
            ranges[2] = ScanRange::get(edge, edge_assigned);
            if (type_assigned) {
                ranges[0] = ScanRange::get(type, type_assigned);
                ranges[1] = ScanRange::get(from, from_assigned);
                return make_unique<IndexScan<3>>(*model.equal_from_to_inverted, move(ranges));
            } else {
                ranges[0] = ScanRange::get(from, from_assigned);
                ranges[1] = ScanRange::get(type, type_assigned);
                return make_unique<IndexScan<3>>(*model.equal_from_to, move(ranges));
            }
        }
    } else if (to == type) {
        // equal_to_type
        array<unique_ptr<ScanRange>, 3> ranges;
        ranges[2] = ScanRange::get(edge, edge_assigned);
        if (from_assigned) {
            ranges[0] = ScanRange::get(from, from_assigned);
            ranges[1] = ScanRange::get(to, to_assigned);
            return make_unique<IndexScan<3>>(*model.equal_to_type_inverted, move(ranges));
        } else {
            ranges[0] = ScanRange::get(to, to_assigned);
            ranges[1] = ScanRange::get(from, from_assigned);
            return make_unique<IndexScan<3>>(*model.equal_to_type, move(ranges));
        }
    } else if (from == type) {
        // equal_from_type
        array<unique_ptr<ScanRange>, 3> ranges;
        ranges[2] = ScanRange::get(edge, edge_assigned);
        if (to_assigned) {
            ranges[0] = ScanRange::get(to, to_assigned);
            ranges[1] = ScanRange::get(from, from_assigned);
            return make_unique<IndexScan<3>>(*model.equal_from_type_inverted, move(ranges));
        } else {
            ranges[0] = ScanRange::get(from, from_assigned);
            ranges[1] = ScanRange::get(to, to_assigned);
            return make_unique<IndexScan<3>>(*model.equal_from_type, move(ranges));
        }
    } else {
        // No special case
        array<unique_ptr<ScanRange>, 4> ranges;
        ranges[3] = ScanRange::get(edge, edge_assigned);

        if (from_assigned) {
            if (type_assigned) { // CASES 1 and 3 => YFTE
                // cout << "using type from to edge\n";
                ranges[0] = ScanRange::get(type, type_assigned);
                ranges[1] = ScanRange::get(from, from_assigned);
                ranges[2] = ScanRange::get(to,   to_assigned);

                return make_unique<IndexScan<4>>(*model.type_from_to_edge, move(ranges));
            } else { // CASES 2 and 4 => FTYE
                // cout << "using from to type edge\n";
                ranges[0] = ScanRange::get(from, from_assigned);
                ranges[1] = ScanRange::get(to,   to_assigned);
                ranges[2] = ScanRange::get(type, type_assigned);

                return make_unique<IndexScan<4>>(*model.from_to_type_edge, move(ranges));
            }
        } else {
            if (to_assigned) { // CASES 5 and 6 => TYFE
                // cout << "using to type from edge\n";
                ranges[0] = ScanRange::get(to,   to_assigned);
                ranges[1] = ScanRange::get(type, type_assigned);
                ranges[2] = ScanRange::get(from, from_assigned);

                return make_unique<IndexScan<4>>(*model.to_type_from_edge, move(ranges));
            } else { // CASES 7 and 8 => YFTE
                // cout << "using type from to edge\n";
                ranges[0] = ScanRange::get(type, type_assigned);
                ranges[1] = ScanRange::get(from, from_assigned);
                ranges[2] = ScanRange::get(to,   to_assigned);

                return make_unique<IndexScan<4>>(*model.type_from_to_edge, move(ranges));
            }
        }
    }
}


unique_ptr<LeapfrogIter> ConnectionPlan::get_leapfrog_iter(const std::set<VarId>& assigned_vars,
                                                           const vector<VarId>&   var_order,
                                                           uint_fast32_t          enumeration_level)
{
    // TODO: support special cases
    if (   (std::holds_alternative<VarId>(from) && from == to)
        || (std::holds_alternative<VarId>(from) && from == type)
        || (std::holds_alternative<VarId>(to)   && to == type)
        || (std::holds_alternative<VarId>(from) && std::get<VarId>(from) == edge)
        || (std::holds_alternative<VarId>(to)   && std::get<VarId>(from) == edge)
        || (std::holds_alternative<VarId>(type) && std::get<VarId>(from) == edge))
    {
        return nullptr;
    }

    vector<unique_ptr<ScanRange>> initial_ranges;
    vector<VarId> intersection_vars;
    vector<VarId> enumeration_vars;

    // index = INT32_MAX means enumeration, index = -1 means term or assigned_var
    int_fast32_t from_index, to_index, type_index, edge_index;

    auto assign_index = [&assigned_vars] (int_fast32_t& index, Id& id) -> void {
        if (std::holds_alternative<ObjectId>(id)) {
            index = -1;
        } else {
            if (assigned_vars.find(std::get<VarId>(id)) == assigned_vars.end()) {
                index = INT32_MAX;
            } else {
                index = -1;
            }
        }
    };

    assign_index(from_index, from);
    assign_index(to_index,   to);
    assign_index(type_index, type);

    // Assign edge_index
    if (assigned_vars.find(edge) == assigned_vars.end()) {
        edge_index = INT32_MAX;
    } else {
        edge_index = -1;
    }

    // search for vars marked as enumeraion (INT32_MAX) that are intersection
    // and assign them the correct index
    for (size_t i = 0; i < enumeration_level; i++) {
        if (from_index == INT32_MAX && std::get<VarId>(from) == var_order[i]) {
            from_index = i;
        }
        if (to_index   == INT32_MAX && std::get<VarId>(to)   == var_order[i]) {
            to_index = i;
        }
        if (type_index == INT32_MAX && std::get<VarId>(type) == var_order[i]) {
            type_index = i;
        }
        if (edge_index == INT32_MAX && edge == var_order[i]) {
            edge_index = i;
        }
    }

    auto assign = [&initial_ranges, &enumeration_vars, &intersection_vars]
                  (int_fast32_t& index, Id id)
                  -> void
    {
        if (index == -1) {
            initial_ranges.push_back(ScanRange::get(id, true));
        } else if (index == INT32_MAX) {
            enumeration_vars.push_back(std::get<VarId>(id));
        } else {
            intersection_vars.push_back(std::get<VarId>(id));
        }
    };

    auto assign_edge = [&initial_ranges, &enumeration_vars, &intersection_vars, &edge_index](VarId edge) -> void {
        if (edge_index == -1) {
            initial_ranges.push_back(ScanRange::get(edge, true));
        } else if (edge_index == INT32_MAX) {
            enumeration_vars.push_back(edge);
        } else {
            intersection_vars.push_back(edge);
        }
    };

    auto get_leapfrog_iter = [&initial_ranges, &enumeration_vars, &intersection_vars]
                             (BPlusTree<4>& bpt)
                             -> unique_ptr<LeapfrogIter>
    {
        return make_unique<LeapfrogBptIter<4>>(
            bpt,
            move(initial_ranges),
            move(intersection_vars),
            move(enumeration_vars)
        );
    };

    // cout << "assigned vars: ";
    // for (auto var : assigned_vars) {
    //     cout << var.id << " ";
    // }

    // cout << "\nedge index: " << edge_index << "\n";

    // if edge_index == -1 we can use the table
    if (edge_index == -1) {
        assign_edge(edge);
        std::array<uint_fast32_t, 3> permutation;

        // Using the corresponding indexes from the edge table
        const uint_fast32_t FROM = 0;
        const uint_fast32_t TO   = 1;
        const uint_fast32_t TYPE = 2;

        // from to type
        if (from_index <= to_index && to_index <= type_index) {
            assign(from_index, from);
            assign(to_index,   to);
            assign(type_index, type);
            permutation = {FROM,TO,TYPE};
        }
        // from type to
        else if (from_index <= type_index && type_index <= to_index) {
            assign(from_index, from);
            assign(type_index, type);
            assign(to_index,   to);
            permutation = {FROM,TYPE,TO};
        }

        // to from type
        else if (to_index <= from_index && from_index <= type_index) {
            assign(to_index,   to);
            assign(from_index, from);
            assign(type_index, type);
            permutation = {TO,FROM,TYPE};
        }

        // to type from
        else if (to_index <= type_index && type_index <= from_index) {
            assign(to_index,   to);
            assign(type_index, type);
            assign(from_index, from);
            permutation = {TO,TYPE,FROM};
        }

        // type from to
        else if (type_index <= from_index && from_index <= to_index) {
            assign(type_index, type);
            assign(from_index, from);
            assign(to_index,   to);
            permutation = {TYPE,FROM,TO};
        }

        // type to from
        else /*if (type_index <= to_index && to_index <= from_index)*/ {
            assign(type_index, type);
            assign(to_index,   to);
            assign(from_index, from);
            permutation = {TYPE,TO,FROM};
        }

        return make_unique<LeapfrogEdgeTableIter>(
            *model.edge_table,
            move(initial_ranges),
            move(intersection_vars),
            move(enumeration_vars),
            move(permutation)
        );
    }

    // from_to_type_edge
    if (from_index <= to_index && to_index <= type_index && type_index <= edge_index) {
        assign(from_index, from);
        assign(to_index,   to);
        assign(type_index, type);
        assign_edge(edge);
        return get_leapfrog_iter(*model.from_to_type_edge);
    }
    // to_type_from_edge
    else if (to_index <= type_index && type_index <= from_index && from_index <= edge_index) {
        assign(to_index,   to);
        assign(type_index, type);
        assign(from_index, from);
        assign_edge(edge);
        return get_leapfrog_iter(*model.to_type_from_edge);
    }
    // type_from_to_edge
    else if (type_index <= from_index && from_index <= to_index && to_index <= edge_index) {
        assign(type_index, type);
        assign(from_index, from);
        assign(to_index,   to);
        assign_edge(edge);
        return get_leapfrog_iter(*model.type_from_to_edge);
    }
    // type_to_from_edge
    else if (type_index <= to_index && to_index <= from_index && from_index <= edge_index) {
        assign(type_index, type);
        assign(to_index,   to);
        assign(from_index, from);
        assign_edge(edge);
        return get_leapfrog_iter(*model.type_to_from_edge);
    } else {
        cout << "no order for leapfrog\n"
             << "from: " << from_index
             << ", to: " << to_index
             << ", type: " << type_index
             << ", edge: " << edge_index
             << "\n";
        return nullptr;
    }
}
