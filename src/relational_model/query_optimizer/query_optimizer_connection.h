#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_CONNECTION_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_CONNECTION_H_

#include <memory>
#include <string>
#include <vector>

#include "base/graph/graph_object.h"
#include "base/ids/graph_id.h"
#include "base/ids/var_id.h"
#include "relational_model/query_optimizer/query_optimizer_element.h"

class QueryOptimizerConnection : public QueryOptimizerElement {
public:
    QueryOptimizerConnection(GraphId graph_id, VarId node_from_var_id, VarId node_to_var_id, VarId edge_var_id);
    ~QueryOptimizerConnection() = default;
    int get_heuristic();
    void try_assign_var(VarId var_id);
    std::vector<VarId> assign();
    std::unique_ptr<BindingIdIter> get_scan();

private:
    GraphId graph_id;

    VarId node_from_var_id;
    VarId node_to_var_id;
    VarId edge_var_id;

    bool node_from_assigned;
    bool node_to_assigned;
    bool edge_assigned;

    bool assigned = false;

    std::unique_ptr<ScanRange> get_node_from_range();
    std::unique_ptr<ScanRange> get_node_to_range();
    std::unique_ptr<ScanRange> get_edge_range();
};

#endif // RELATIONAL_MODEL__QUERY_OPTIMIZER_CONNECTION_H_
