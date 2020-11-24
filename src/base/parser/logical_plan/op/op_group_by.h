#ifndef BASE__OP_GROUP_BY_H_
#define BASE__OP_GROUP_BY_H_

#include <string>
#include <vector>

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/grammar/query/query_ast.h"

class OpGroupBy : public Op {
public:
    const std::unique_ptr<Op> op;
    const bool ascending_order;
    const std::vector<query::ast::SelectItem> items;

    OpGroupBy(std::vector<query::ast::SelectItem> items, std::unique_ptr<Op> op, bool ascending_order) :
        op              (std::move(op)),
        ascending_order (ascending_order),
        items           (items) { }

    ~OpGroupBy() = default;

    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }
};

#endif // BASE__OP_GROUP_BY_H_