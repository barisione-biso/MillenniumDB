/*#ifndef BASE__OP_ORDER_BY_H_
#define BASE__OP_ORDER_BY_H_

#include <string>
#include <vector>

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/grammar/query/query_ast.h"

class OpOrderBy : public Op {
public:
    std::vector<query::ast::SelectItem> order_items;
    const std::unique_ptr<Op> child_op;

    OpOrderBy(std::vector<query::ast::SelectItem> order_items, std::unique_ptr<Op> child_op) :
      order_items(std::move(order_items)), child_op(std::move(child_op))
        { }

    ~OpOrderBy() = default;

    void accept_visitor(OpVisitor& visitor) {
        visitor.visit(*this);
    }
};

#endif // BASE__OP_ORDER_BY_H_
*/