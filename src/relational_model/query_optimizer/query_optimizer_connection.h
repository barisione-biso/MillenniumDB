#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_CONNECTION_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_CONNECTION_H_

#include <string>
#include "relational_model/query_optimizer/query_optimizer_element.h"

class RelationalGraph;

class QueryOptimizerConnection : public QueryOptimizerElement {
private:
    RelationalGraph& graph;

    VarId from_var_id;
    VarId to_var_id;
    VarId edge_var_id;

    bool assigned = false;

    bool from_assigned;
    bool to_assigned;
    bool edge_assigned;

public:
    QueryOptimizerConnection(RelationalGraph& graph, VarId from_var_id, VarId to_var_id, VarId edge_var_id);
    ~QueryOptimizerConnection() = default;
    int get_heuristic();
    void try_assign_var(VarId var_id);
    std::vector<VarId> assign();
    std::unique_ptr<GraphScan> get_scan();
};

#endif //RELATIONAL_MODEL__QUERY_OPTIMIZER_CONNECTION_H_
