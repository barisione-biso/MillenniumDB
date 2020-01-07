#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_LABEL_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_LABEL_H_

#include <string>
#include <vector>

#include "base/graph/graph_element.h"
#include "base/var/var_id.h"
#include "relational_model/query_optimizer/query_optimizer_element.h"

class RelationalGraph;

class QueryOptimizerLabel : public QueryOptimizerElement {
private:
    RelationalGraph& graph;

    VarId element_var_id;
    VarId label_var_id;

    ElementType element_type;
    ObjectId label_object_id;

    bool assigned = false;

    bool element_assigned;
    bool label_assigned;

public:
    QueryOptimizerLabel(RelationalGraph& graph, VarId element_var_id, VarId label_var_id,
        ElementType element_type, ObjectId label_object_id);
    ~QueryOptimizerLabel() = default;
    int get_heuristic();
    void assign();
    void try_assign_var(VarId var_id);
    std::vector<VarId> get_assigned();
    unique_ptr<GraphScan> get_scan();
};

#endif //RELATIONAL_MODEL__QUERY_OPTIMIZER_LABEL_H_
