#pragma once

#include "parser/query/op/op_visitor.h"

namespace SPARQL {
/**
 * Will throw an exception if unused variable is mentioned in the SELECT statement
 */
class CheckVarNames : public OpVisitor {
public:
    void visit(OpSelect&)   override;
};
} // namespace SPARQL