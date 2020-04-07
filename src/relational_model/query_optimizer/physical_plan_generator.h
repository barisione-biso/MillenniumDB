#ifndef BASE__PHYSICAL_PLAN_GENERATOR_H_
#define BASE__PHYSICAL_PLAN_GENERATOR_H_

#include "base/ids/var_id.h"
#include "base/ids/object_id.h"
#include "base/ids/graph_id.h"
#include "base/graph/graph_object.h"
#include "base/parser/grammar/ast.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"

#include <map>
#include <memory>
#include <vector>

class BindingIter;
class OpMatch;
class OpFilter;
class OpSelect;
class RelationalGraph;

class PhysicalPlanGenerator : OpVisitor {
private:
    std::unique_ptr<BindingIter> tmp;
    std::map<std::string, VarId> id_map;
    std::map<std::string, GraphId> graph_ids; // graph_name to graph_id
    std::map<std::string, GraphId> var2graph_id;
    std::map<std::string, ObjectType> element_types;
    std::vector<std::pair<std::string, std::string>> select_items;
    int_fast32_t id_count = 0;

    VarId get_var_id(const std::string& var_name);
    ObjectId get_value_id(const ast::Value& value);

public:
    PhysicalPlanGenerator();
    ~PhysicalPlanGenerator() = default;

    std::unique_ptr<BindingIter> exec(OpSelect&);

    void visit(OpSelect&);
    void visit(OpMatch&);
    void visit(OpFilter&);
    void visit(OpLabel&);
    void visit(OpProperty&);
    void visit(OpConnection&);
    void visit(OpLonelyNode&);
};


#endif // BASE__PHYSICAL_PLAN_GENERATOR_H_