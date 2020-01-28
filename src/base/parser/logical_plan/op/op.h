#ifndef BASE__OP_H_
#define BASE__OP_H_

#include <string>
#include <tuple>
#include <vector>
#include <memory>
#include <iostream>

#include "base/parser/grammar/ast_printer.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"

class OpSelect;

class Op {
private:
    static std::unique_ptr<OpSelect> get_select_plan(ast::Root& ast);

public:
    virtual void visit(OpVisitor&) = 0;
    static std::unique_ptr<OpSelect> get_select_plan(std::string query);
};

#endif //BASE__OP_H_
