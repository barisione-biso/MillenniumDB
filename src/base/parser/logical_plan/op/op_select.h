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

    OpSelect(std::vector<query::ast::SelectItem> select_items, std::unique_ptr<Op> op, uint_fast32_t limit) :
        limit        (limit),
        op           (std::move(op)),
        select_items (std::move(select_items)) { }

    ~OpSelect() = default;


    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpSelect(";
        if(select_items.size() == 0){
            os << "*";
        }else{
            bool first = true;
            for (auto & select_item : select_items) {
                if (!first) os << ", ";
                first = false;
                if(select_item.key)
                {
                    os << select_item.var << "." << select_item.key.get();
                }else{
                    os << select_item.var;
                }
            }
        }
        os << ")";

        if (limit)
        {
            os << " LIMIT " << limit;
        }
        os << "\n";
        return op->print_to_ostream(os, indent + 2);
    };
};

#endif // BASE__OP_SELECT_H_
