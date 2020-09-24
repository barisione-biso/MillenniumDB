#include "op_select.h"

#include "base/parser/logical_plan/exceptions.h"

using namespace std;

// OpSelect::OpSelect(unique_ptr<Op> op, uint_fast32_t limit) :
//     select_all(true),
//     limit(limit),
//     op(move(op)) { }

OpSelect::OpSelect(vector<query::ast::SelectItem> select_items, unique_ptr<Op> op, uint_fast32_t limit) :
    limit        (limit),
    op           (move(op)),
    select_items (move(select_items)) { }

void OpSelect::accept_visitor(OpVisitor& visitor) {
    visitor.visit(*this);
}
