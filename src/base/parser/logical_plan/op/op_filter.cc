#include "op_filter.h"

using namespace std;

OpFilter::OpFilter(unique_ptr<Condition> condition)
    : condition(move(condition)) { }


OpFilter::OpFilter(boost::optional<ast::Formula> const& optional_formula) {
    FormulaVisitor visitor = FormulaVisitor();
    condition = visitor(optional_formula);
}

void OpFilter::visit(OpVisitor& visitor) {
    visitor.visit(*this);
}
