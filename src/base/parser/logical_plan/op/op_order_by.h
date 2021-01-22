#ifndef BASE__OP_ORDER_BY_H_
#define BASE__OP_ORDER_BY_H_

#include <string>
#include <vector>

#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/logical_plan/op/op.h"

class OpOrderBy : public Op {
public:
    const std::unique_ptr<Op> op;
    const bool ascending_order;
    const std::vector<query::ast::SelectItem> items;

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpOrderBy()";
        if(items.size() == 0){
            os << "*";
        }else{
            bool first = true;
            for (auto & item : items) {
                if (!first) os << ", ";
                first = false;
                if(item.key)
                {
                    os << item.var << "." << item.key.get();
                }else{
                    os << item.var;
                }
            }
        }
        os << ")";

        if (ascending_order) {
            os << " ASCENDING";
        } else {
            os << " DESCENDING";
        }
        os << "\n";
        return op->print_to_ostream(os, indent + 2);
    };

    OpOrderBy(std::vector<query::ast::SelectItem> items, std::unique_ptr<Op> op, bool ascending_order) :
        op              (std::move(op)),
        ascending_order (ascending_order),
        items           (items) { }

    ~OpOrderBy() = default;


    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::set<std::string> get_var_names() const override {
        // TODO: should add properties mentioned in the GROUP BY that are not present in the MATCH?
        return op->get_var_names();
    }
};

#endif // BASE__OP_ORDER_BY_H_
