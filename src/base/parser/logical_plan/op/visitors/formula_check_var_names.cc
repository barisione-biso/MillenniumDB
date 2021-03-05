#include "formula_check_var_names.h"

#include "base/parser/logical_plan/exceptions.h"

FormulaCheckVarNames::FormulaCheckVarNames(const std::set<std::string>& declared_var_names) :
    declared_var_names(declared_var_names) { }


void FormulaCheckVarNames::operator()(query::ast::AtomicFormula const& atomic_formula) const {
    boost::apply_visitor(*this, atomic_formula.content);
}


void FormulaCheckVarNames::operator()(boost::optional<query::ast::FormulaDisjunction> const& optional_formula) const {
    if (optional_formula) {
        auto formula = static_cast<query::ast::FormulaDisjunction>(optional_formula.get());
        (*this)(formula);
    }
}


void FormulaCheckVarNames::operator()(query::ast::FormulaDisjunction const& disjunction) const {
    for (auto& f : disjunction.formula_conjunctions) {
        (*this)(f);
    }
}


void FormulaCheckVarNames::operator()(query::ast::FormulaConjunction const& conjunction) const {
    for (auto& f : conjunction.formulas) {
        (*this)(f);
    }
}


void FormulaCheckVarNames::operator()(query::ast::Statement const& statement) const {
    (*this)(statement.lhs);
    boost::apply_visitor(*this, statement.rhs);
}

void FormulaCheckVarNames::operator()(query::ast::SelectItem const& select_item) const {
    if (declared_var_names.find(select_item.var) == declared_var_names.cend()){
        throw QuerySemanticException("Variable \"" + select_item.var +
            "\" used in WHERE is not declared in MATCH");
    }
}

void FormulaCheckVarNames::operator()(common::ast::Value const&) const { }
