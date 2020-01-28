#include "op_select.h"

#include "base/parser/logical_plan/exceptions.h"

using namespace std;

OpSelect::OpSelect(const boost::variant<ast::All, vector<ast::Element>>& selection, unique_ptr<Op> op)
    : op(move(op))
{
    if (selection.type() == typeid(ast::All)) {
        select_all = true;
        throw NotSupportedException();
    }
    else {
        select_all = false;
        auto& select_list = boost::get<vector<ast::Element>>(selection);

        for (auto& select_item : select_list) {
            select_items.push_back({ select_item.key, select_item.variable });
        }
    }
}

void OpSelect::visit(OpVisitor& visitor) {
    visitor.visit(*this);
}
