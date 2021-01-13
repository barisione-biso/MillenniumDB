#ifndef BASE__OP_SELECT_H_
#define BASE__OP_SELECT_H_

#include <string>
#include <tuple>
#include <vector>
#include <memory>

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/grammar/query/query_ast.h"

class OpSelect : public Op {
public:
    const uint_fast32_t limit;
    const std::unique_ptr<Op> op;

    // pair <var_name, key_name>
    // or   <var_name, ""> (empty string when selecting just the variable)
    // empty vector means SELECT *
    const std::vector<query::ast::SelectItem> select_items;

    OpSelect(std::unique_ptr<Op> op, std::vector<query::ast::SelectItem> select_items, uint_fast32_t limit) :
        limit        (limit),
        op           (std::move(op)),
        select_items (std::move(select_items)) { }

    ~OpSelect() = default;

    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }
};

#endif // BASE__OP_SELECT_H_
