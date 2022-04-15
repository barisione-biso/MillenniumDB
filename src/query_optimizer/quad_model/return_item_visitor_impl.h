#pragma once

#include <functional>
#include <set>
#include <string>
#include <vector>

#include "base/ids/var_id.h"
#include "base/query/var.h"
#include "execution/binding_iter/aggregation/agg.h"
#include "parser/query/return_item/return_item_visitor.h"
#include "query_optimizer/quad_model/binding_iter_visitor.h"

class ReturnItemVisitorImpl : public ReturnItemVisitor {
public:
    ReturnItemVisitorImpl(std::set<std::pair<Var, std::string>>& var_properties,
                          std::vector<std::pair<Var, VarId>>& projection_vars,
                          std::map<VarId, std::unique_ptr<Agg>>& aggregates,
                          const BindingIterVisitor& binding_iter_visitor) :
        var_properties       (var_properties),
        projection_vars      (projection_vars),
        aggregates           (aggregates),
        binding_iter_visitor (binding_iter_visitor) { }

    void visit(ReturnItemAgg&)   override;
    void visit(ReturnItemCount&) override;
    void visit(ReturnItemVar&)   override;

private:
    std::set<std::pair<Var, std::string>>& var_properties;

    std::vector<std::pair<Var, VarId>>& projection_vars;

    std::map<VarId, std::unique_ptr<Agg>>& aggregates;

    const BindingIterVisitor& binding_iter_visitor;
};
