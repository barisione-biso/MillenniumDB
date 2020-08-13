#ifndef BASE__OP_SELECT_H_
#define BASE__OP_SELECT_H_

#include <string>
#include <tuple>
#include <vector>
#include <memory>

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/grammar/query/query_ast.h"

class OpSelect : public Op {
public:
    bool select_all;
    uint_fast32_t limit;
    std::unique_ptr<Op> op;

    // pair <var_name, key_name>
    std::vector< std::pair<std::string, std::string> > select_items;

    OpSelect(std::vector<query_ast::Element> select_list, std::unique_ptr<Op> op, uint_fast32_t limit);

    // constructor for select*
    OpSelect(std::unique_ptr<Op> op, uint_fast32_t limit);

    ~OpSelect() = default;
    void accept_visitor(OpVisitor&);
};

#endif // BASE__OP_SELECT_H_
