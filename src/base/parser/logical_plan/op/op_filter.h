#ifndef BASE__OP_FILTER_H_
#define BASE__OP_FILTER_H_

#include <map>
#include <memory>
#include <vector>

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/visitors/formula_check_var_names.h"

class OpFilter : public Op {
public:
    std::unique_ptr<Op> op;
    const query::ast::Formula formula;

    OpFilter(std::unique_ptr<Op> op, const query::ast::Formula formula) :
        op      (std::move(op)),
        formula (formula) { }


    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }

    // checks filters only uses declared variables, throws QuerySemanticException if not
    void check_var_names(std::set<std::string>& declared_var_names) const {
        FormulaCheckVarNames visitor(declared_var_names);
        visitor(formula);
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override {
        os << std::string(indent, ' ');
        os << "OpFilter(";
        // TODO: print formulas
        // for (auto const& step_formula : formula.path) {
            // if (step_formula.condition.negation) {
                // os << "!" << step_formula.condition.content;
            // }
        // };
        os << ")\n";
        return op->print_to_ostream(os, indent + 2);
    };

    std::set<std::string> get_var_names() const override {
        return op->get_var_names(); // TODO: should add properties mentioned in the WHERE that are not present in the MATCH
    }
};

#endif // BASE__OP_FILTER_H_
