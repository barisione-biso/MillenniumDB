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
#include "base/parser/logical_plan/op/visitors/op_visitor.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"

class BindingIdIter;

class BindingIdIterVisitor : public OpVisitor {
public:
    BindingIdIterVisitor(QuadModel& model, const std::map<std::string, VarId>& var_name2var_id);
    ~BindingIdIterVisitor() = default;

    std::unique_ptr<BindingIdIter> exec(OpSelect&);
    std::unique_ptr<BindingIdIter> exec(manual_plan::ast::ManualRoot&);

    void visit(OpMatch&) override;
    void visit(OpOptional&) override;
    void visit(OpSelect&) override;
    void visit(OpFilter&) override;
    void visit(OpConnection&) override;
    void visit(OpConnectionType&) override;
    void visit(OpLabel&) override;
    void visit(OpProperty&) override;
    void visit(OpGroupBy&) override;
    void visit(OpOrderBy&) override;
    void visit(OpUnjointObject&) override;
    void visit(OpGraphPatternRoot&) override;
    void visit(OpDistinct&) override;

    void visit(OpPropertyPath&) override;
    void visit(OpPath&) override;
    void visit(OpPathAtom&) override;
    void visit(OpPathAlternatives&) override;
    void visit(OpPathSequence&) override;
    void visit(OpPathKleeneStar&) override;
    void visit(OpPathOptional&) override;

    QuadModel& model;
    std::vector<query::ast::SelectItem> select_items;
    const std::map<std::string, VarId>& var_name2var_id;
    std::set<VarId> assigned_vars;
    std::unique_ptr<BindingIdIter> tmp;

    VarId get_var_id(const std::string& var_name);
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

};

#endif // RELATIONAL_MODEL__BINDING_ID_ITER_VISITOR_H_
