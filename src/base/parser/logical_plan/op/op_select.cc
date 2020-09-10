#include "op_select.h"

#include "base/parser/logical_plan/exceptions.h"

using namespace std;

OpSelect::OpSelect(unique_ptr<Op> op, uint_fast32_t limit) :
    select_all(true),
    limit(limit),
    op(move(op)) { }

OpSelect::OpSelect(vector<query::ast::Element> select_list, unique_ptr<Op> op, uint_fast32_t limit) :
    select_all(false),
    limit(limit),
    op(move(op))
{
    for (auto& select_item : select_list) {
        select_items.push_back({ select_item.var.name, select_item.key });
    }
}

void OpSelect::accept_visitor(OpVisitor& visitor) {
    visitor.visit(*this);
}
