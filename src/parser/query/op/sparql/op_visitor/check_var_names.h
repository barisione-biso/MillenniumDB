#pragma once

#include <set>

#include "base/query/var.h"
#include "parser/query/op/op_visitor.h"

namespace SPARQL {
/*
Will throw an exception if an unused variable is mentioned in some of these statements:
- SELECT
- FILTER
- ORDER BY
Also check that every path variable is an unique variable
*/
class CheckVarNames : public OpVisitor {
public:
    void visit(OpBasicGraphPattern&) override;
    void visit(OpFilter&)            override;
    void visit(OpOptional&)          override;
    void visit(OpOrderBy&)           override;
    void visit(OpSelect&)            override;
    void visit(OpWhere&)             override;

private:
    std::set<Var> declared_vars;
    std::set<Var> declared_path_vars;
};
} // namespace SPARQL
