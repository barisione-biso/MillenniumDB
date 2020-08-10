#ifndef BASE__QUERY_AST_PRINTER_H_
#define BASE__QUERY_AST_PRINTER_H_

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "base/parser/grammar/query/query_ast.h"

// Prints the AST generated at parsing in a JSON-like
// format to a given stream.
class QueryAstPrinter : public boost::static_visitor<void> {
private:
    std::ostream& out;
    const int_fast32_t base_indent;
    static const int_fast32_t tab_size = 2;

public:
    QueryAstPrinter(std::ostream& out, int_fast32_t base_indent);
    QueryAstPrinter(std::ostream& out);
    void indent() const;
    void indent(std::string str) const;
    void indent(std::string str, int_fast32_t extra_indent) const;

    void operator() (query_ast::Root const&) const;
    void operator() (std::vector<query_ast::Element> const&) const;
    void operator() (query_ast::All const&) const;
    void operator() (std::vector<query_ast::LinearPattern> const&) const;
    void operator() (query_ast::LinearPattern const&) const;
    void operator() (query_ast::Element const&) const;
    void operator() (query_ast::Node) const;
    void operator() (query_ast::Edge) const;
    void operator() (query_ast::StepPath) const;
    void operator() (boost::optional<query_ast::Formula> const&) const;
    void operator() (query_ast::Condition const&) const;
    void operator() (query_ast::Statement const&) const;
    void operator() (query_ast::StepFormula const&) const;
    void operator() (ast::Value const&) const;

    void operator() (ast::Var const&) const;
    void operator() (std::string const&) const;
    void operator() (VarId    const&) const;
    void operator() (int64_t  const&) const;
    void operator() (float    const&) const;
    void operator() (bool     const&) const;
    void operator() (query_ast::And const&) const;
    void operator() (query_ast::Or  const&) const;
    void operator() (query_ast::EQ  const&) const;
    void operator() (query_ast::NE  const&) const;
    void operator() (query_ast::GT  const&) const;
    void operator() (query_ast::LT  const&) const;
    void operator() (query_ast::GE  const&) const;
    void operator() (query_ast::LE  const&) const;
};

#endif  // BASE__QUERY_AST_PRINTER_H_
