#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_ELEMENT_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_ELEMENT_H_

#include <string>
#include <memory>
#include <vector>


#include "relational_model/physical_plan/binding_id_iter/operators/graph_scan.h"

enum class ElementType {
    UNKNOWN_NODE,
    UNKNOWN_EDGE,
    FIXED_NODE,
    FIXED_EDGE
};

class QueryOptimizerElement {
public:
    virtual int get_heuristic() = 0;
    virtual void assign() = 0;
    virtual void try_assign_var(VarId var_id) = 0;
    virtual std::vector<VarId> get_assigned() = 0;
    virtual std::unique_ptr<GraphScan> get_scan() = 0;
};

#endif //RELATIONAL_MODEL__QUERY_OPTIMIZER_ELEMENT_H_
