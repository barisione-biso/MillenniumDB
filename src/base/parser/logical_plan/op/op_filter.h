#ifndef BASE__OP_FILTER_H_
#define BASE__OP_FILTER_H_

#include <map>
#include <memory>
#include <vector>

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/visitors/formula_check_var_names.h"
#include "base/parser/grammar/query/printer/query_ast_printer.h"

class OpFilter : public Op {
public:
    std::unique_ptr<Op> op;
    const query::ast::FormulaDisjunction formula_disjunction;

    OpFilter(std::unique_ptr<Op> op, const query::ast::FormulaDisjunction formula_disjunction) :
        op                  (std::move(op)),
        formula_disjunction (formula_disjunction) { }


    void accept_visitor(OpVisitor& visitor) override {
        visitor.visit(*this);
    }


    // checks filters only uses declared variables, throws QuerySemanticException if not
    void check_var_names(std::set<std::string>& declared_var_names) const {
        FormulaCheckVarNames visitor(declared_var_names);
        visitor(formula_disjunction);
    }


    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override {
        os << std::string(indent, ' ');
        os << "OpFilter(";
        QueryAstPrinter printer(os, 0);
        printer(formula_disjunction);
        os << ")\n";
        return op->print_to_ostream(os, indent + 2);
    };


    void get_vars(std::set<Var>& set) const override {
        // TODO: should add properties mentioned in the WHERE that are not present in the MATCH?
        return op->get_vars(set);
    }
};

#endif // BASE__OP_FILTER_H_
