#ifndef RELATIONAL_MODEL__BINDING_ID_ITER_VISITOR_H_
#define RELATIONAL_MODEL__BINDING_ID_ITER_VISITOR_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/graph/graph_object.h"
#include "base/ids/object_id.h"
#include "base/ids/var_id.h"
#include "base/parser/grammar/manual_plan/manual_plan_ast.h"
#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/logical_plan/var.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"

class BindingIdIter;

class BindingIdIterVisitor : public OpVisitor {
public:
    BindingIdIterVisitor(const QuadModel& model, const std::map<Var, VarId>& var2var_id);
    ~BindingIdIterVisitor() = default;

    std::unique_ptr<BindingIdIter> exec(OpSelect&);
    std::unique_ptr<BindingIdIter> exec(manual_plan::ast::ManualRoot&);

    const QuadModel& model;
    std::vector<query::ast::SelectItem> select_items;
    const std::map<Var, VarId>& var2var_id;
    std::set<VarId> assigned_vars;
    std::unique_ptr<BindingIdIter> tmp;

    VarId get_var_id(const Var& var);
    ObjectId get_value_id(const common::ast::Value& value);

    std::unique_ptr<JoinPlan> get_greedy_join_plan(
        std::vector<std::unique_ptr<JoinPlan>> base_plans,
        std::vector<std::string>& var_names,
        uint64_t input_vars);

    // May return nullptr if leapfrog is not possible
    std::unique_ptr<BindingIdIter> try_get_leapfrog_plan(
        const std::vector<std::unique_ptr<JoinPlan>>& base_plans,
        std::vector<std::string>& var_names,
        const std::size_t binding_size,
        uint64_t input_vars);

    void visit(OpBasicGraphPattern&) override;
    void visit(OpOptional&)          override;

    void visit(OpConnection&)       override { }
    void visit(OpDistinct&)         override { }
    void visit(OpGroupBy&)          override { }
    void visit(OpFilter&)           override { }
    void visit(OpGraphPatternRoot&) override { }
    void visit(OpIsolatedTerm&)     override { }
    void visit(OpIsolatedVar&)      override { }
    void visit(OpLabel&)            override { }
    void visit(OpOrderBy&)          override { }
    void visit(OpPath&)             override { }
    void visit(OpPathAlternatives&) override { }
    void visit(OpPathAtom&)         override { }
    void visit(OpPathSequence&)     override { }
    void visit(OpPathKleeneStar&)   override { }
    void visit(OpPathOptional&)     override { }
    void visit(OpProperty&)         override { }
    void visit(OpPropertyPath&)     override { }
    void visit(OpSelect&)           override { }
};

#endif // RELATIONAL_MODEL__BINDING_ID_ITER_VISITOR_H_
