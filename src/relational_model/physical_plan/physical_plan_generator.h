#ifndef BASE__PHYSICAL_PLAN_GENERATOR_H_
#define BASE__PHYSICAL_PLAN_GENERATOR_H_

#include "base/ids/var_id.h"
#include "base/graph/element_type.h"
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
    // std::map<std::string, VarId> var2graph; // TODO:
    int_fast32_t id_count = 0;
    std::vector<std::pair<std::string, std::string>> select_items;
    std::map<std::string, ElementType> var_types;

    VarId get_var_id(const std::string& var);

public:
    PhysicalPlanGenerator();
    ~PhysicalPlanGenerator() = default;

    std::unique_ptr<BindingIter> exec(OpSelect&);

    void visit (OpSelect&);
    void visit (OpMatch&);
    void visit (OpFilter&);
    void visit (OpLabel&);
    void visit (OpProperty&);
    void visit (OpConnection&);
};


#endif // BASE__PHYSICAL_PLAN_GENERATOR_H_
