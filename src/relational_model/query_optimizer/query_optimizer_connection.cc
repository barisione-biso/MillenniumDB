#include "query_optimizer_connection.h"

#include "relational_model/graph/relational_graph.h"

QueryOptimizerConnection::QueryOptimizerConnection(RelationalGraph& graph, VarId from_var_id, VarId to_var_id,
        VarId edge_var_id) :
    graph(graph),
    from_var_id(from_var_id),
    to_var_id(to_var_id),
    edge_var_id(edge_var_id)
{
    from_assigned = false;
    to_assigned = false;
    edge_assigned = false;
}


int QueryOptimizerConnection::get_heuristic() {
    if (assigned) return -1;

    else if (from_assigned && to_assigned && edge_assigned) return 99; // Connection(_,_,_)
    else if (from_assigned && to_assigned)                  return  9; // Connection(_,?,_)
    else if (from_assigned)                                 return  7; // Connection(_,?,?)
    else if (to_assigned)                                   return  6; // Connection(?,?,_)
    else                                                    return  1; // Connection(?,?,?)
}


void QueryOptimizerConnection::assign() {
    assigned = true;
}


void QueryOptimizerConnection::try_assign_var(VarId var_id) {
    if (from_var_id == var_id) {
        from_assigned = true;
    }
    else if (to_var_id == var_id) {
        to_assigned = true;
    }
    else if (edge_var_id == var_id) { // TODO: can `from` be equal to `to`
        edge_assigned = true;
    }
}


std::vector<VarId> QueryOptimizerConnection::get_assigned() {
    vector<VarId> res;

    if (!from_assigned)
        res.push_back(from_var_id);

    if (!to_assigned)
        res.push_back(to_var_id);

    if (!edge_assigned)
        res.push_back(edge_var_id);

    return std::move(res);
}


unique_ptr<GraphScan> QueryOptimizerConnection::get_scan() {
    std::vector<ObjectId> terms;
    std::vector<VarId> vars;

    if (from_assigned) { // Connection(_,_,_), Connection(_,?,_), Connection(_,?,?)
        vars.push_back(from_var_id);
        vars.push_back(to_var_id);
        vars.push_back(edge_var_id);
        return make_unique<GraphScan>(*graph.prop2element, terms, vars);
    }
    else { // Connection(?,?,_), Connection(?,?,?)
        vars.push_back(to_var_id);
        vars.push_back(from_var_id);
        vars.push_back(edge_var_id);
        return make_unique<GraphScan>(*graph.element2prop, terms, vars);
    }
}