#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_ELEMENT_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_ELEMENT_H_

#include <string>
#include <memory>
#include <vector>


#include "relational_model/physical_plan/operators/graph_scan.h"

class QueryOptimizerElement {
public:
    virtual int get_heuristic() = 0;
    virtual void try_assign_var(VarId var_id) = 0;
    virtual std::vector<VarId> assign() = 0;
    virtual std::unique_ptr<GraphScan> get_scan() = 0;
};

#endif //RELATIONAL_MODEL__QUERY_OPTIMIZER_ELEMENT_H_
