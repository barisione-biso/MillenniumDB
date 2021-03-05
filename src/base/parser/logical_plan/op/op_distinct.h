#ifndef BASE__OP_DISTINCT_H_
#define BASE__OP_DISTINCT_H_

#include <string>
#include <vector>

//#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/logical_plan/op/op.h"

class OpDistinct : public Op {
public:
    const std::unique_ptr<Op> op;
    //std::vector<query::ast::SelectItem> items;

    OpDistinct(std::unique_ptr<Op> op)://, const std::vector<query::ast::OrderedSelectItem>& ordered_items) :
        op (std::move(op))
    {
        // TODO:
        //for (auto& order_item : ordered_items) {
        //    items.push_back(order_item.item);
        //    ascending_order.push_back(order_item.order == query::ast::Order::Ascending);
       // }
    }

    ~OpDistinct() = default;

    void accept_visitor(OpVisitor& visitor) override {
        // TODO: complete
        visitor.visit(*this);
    }

    std::set<std::string> get_var_names() const override {
        // TODO: complete if needed
        return op->get_var_names();
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override {
        // TODO: complete
        os << std::string(indent, ' ');
        os << "OpDistinct(";
        //bool first = true;
        /*for (auto & item : items) {
            if (!first) os << ", ";
            first = false;
            if (item.key) {
                os << item.var << "." << item.key.get();
            } else {
                os << item.var;
            }
        }*/
        os << ")\n";
        return op->print_to_ostream(os, indent + 2);
    }
};

#endif // BASE__OP_DISTINCT_H_
