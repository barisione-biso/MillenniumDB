
#ifndef BASE__OP_OPTIONAL_H_
#define BASE__OP_OPTIONAL_H_

#include <string>
#include <vector>

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/grammar/query/query_ast.h"

class OpOptional : public Op {
public:
    const std::unique_ptr<Op> op;
    const std::vector<boost::recursive_wrapper<query::ast::GraphPattern>> optionals;

    OpOptional(std::unique_ptr<Op> op, std::vector<boost::recursive_wrapper<query::ast::GraphPattern>> optionals) :
        op              (std::move(op)),
        optionals(std::move(optionals)) { }

    ~OpOptional() = default;

    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }
};
#endif // BASE__OP_OPTIONAL_H_