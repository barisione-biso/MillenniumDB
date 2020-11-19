#ifndef BASE__FORMULA_CHECK_VAR_NAMES_H_
#define BASE__FORMULA_CHECK_VAR_NAMES_H_

#include <set>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "base/parser/grammar/query/query_ast.h"

class FormulaCheckVarNames : public boost::static_visitor<void> {
public:
    const std::set<std::string>& declared_var_names;

    FormulaCheckVarNames(const std::set<std::string>& declared_var_names);

    void operator()(query::ast::Formula const& formula) const;
    void operator()(boost::optional<query::ast::Formula> const& formula) const;
    void operator()(query::ast::Condition const& condition) const;
    void operator()(query::ast::Statement const& statement) const;
    void operator()(query::ast::SelectItem const& select_item) const;
    void operator()(common::ast::Value const&) const;
};

#endif // BASE__FORMULA_CHECK_VAR_NAMES_H_
