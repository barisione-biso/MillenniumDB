#ifndef RELATIONAL_MODEL__QUERY_OPTIMIZER_H_
#define RELATIONAL_MODEL__QUERY_OPTIMIZER_H_

#include <set>
#include <map>
#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "base/ids/object_id.h"
#include "base/ids/graph_id.h"
#include "base/graph/graph_object.h"
#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/grammar/manual_plan/manual_plan_ast.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"
#include "relational_model/models/quad_model/quad_model.h"

class BindingIter;
class BindingIdIter;
class OpMatch;
class OpFilter;
class OpSelect;
class JoinPlan;

class QueryOptimizer : OpVisitor {
public:
    QueryOptimizer(QuadModel& model);
    ~QueryOptimizer() = default;

    std::unique_ptr<BindingIter> exec(OpSelect&);
    std::unique_ptr<BindingIter> exec(manual_plan_ast::Root&);

    void visit(OpSelect&);
    void visit(OpMatch&);
    void visit(OpFilter&);
    void visit(OpNodeLabel&);
    void visit(OpEdgeLabel&);
    void visit(OpNodeProperty&);
    void visit(OpEdgeProperty&);
    void visit(OpConnection&);
    void visit(OpLonelyNode&);
    void visit(OpNodeLoop&);

private:
    QuadModel& model;
    std::unique_ptr<BindingIter> tmp;
    std::map<std::string, VarId> id_map;
    std::map<std::string, GraphId> graph_ids; // graph_name to graph_id
    std::map<std::string, GraphId> var2graph_id;
    std::set<std::string> node_names;
    std::set<std::string> edge_names;
    std::vector<std::pair<std::string, std::string>> select_items;
    int_fast32_t id_count = 0;

    VarId get_var_id(const std::string& var_name);
    ObjectId get_value_id(const common::ast::Value& value);
    GraphId search_graph_id(const std::string& graph_name);

    std::unique_ptr<BindingIdIter> get_greedy_join_plan(std::vector<std::unique_ptr<JoinPlan>> base_plans);

};

#endif // RELATIONAL_MODEL__QUERY_OPTIMIZER_H_
