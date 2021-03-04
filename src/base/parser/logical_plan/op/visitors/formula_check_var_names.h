#ifndef BASE__FORMULA_CHECK_VAR_NAMES_H_
#define BASE__FORMULA_CHECK_VAR_NAMES_H_

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <set>

#include "base/parser/grammar/query/query_ast.h"

class FormulaCheckVarNames : public boost::static_visitor<void> {
public:
    const std::set<std::string>& declared_var_names;

    FormulaCheckVarNames(const std::set<std::string>& declared_var_names);

    void operator()(boost::optional<query::ast::FormulaDisjunction> const&) const;
    void operator()(query::ast::AtomicFormula const&) const;
    void operator()(query::ast::FormulaConjunction const&) const;
    void operator()(query::ast::FormulaDisjunction const&) const;
    void operator()(query::ast::Statement const&) const;
    void operator()(query::ast::SelectItem const&) const;
    void operator()(common::ast::Value const&) const;
};

#endif // BASE__FORMULA_CHECK_VAR_NAMES_H_
