#pragma once

#include <set>

#include "base/query/var.h"
#include "parser/query/return_item/return_item_visitor.h"

class ReturnItemCheckVarName : public ReturnItemVisitor {
public:
    ReturnItemCheckVarName(std::set<Var>& declared_vars, std::set<Var>& declared_path_vars, bool op_return_distinct) :
        declared_vars      (declared_vars),
        declared_path_vars (declared_path_vars),
        op_return_distinct (op_return_distinct) {}

    void visit(ReturnItemAgg&)   override;
    void visit(ReturnItemCount&) override;
    void visit(ReturnItemVar&)   override;

private:
    std::set<Var>& declared_vars;

    std::set<Var>& declared_path_vars;

    bool op_return_distinct;
};
