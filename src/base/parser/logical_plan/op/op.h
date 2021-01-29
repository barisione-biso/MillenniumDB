#ifndef BASE__OP_H_
#define BASE__OP_H_

#include <memory>
#include <set>
#include <string>

#include "base/binding/binding.h"
#include "base/parser/logical_plan/op/visitors/op_visitor.h"

class Op {
public:
    virtual ~Op() = default;

    virtual void accept_visitor(OpVisitor&) = 0;
    virtual std::set<std::string> get_var_names() const = 0;  // TODO: rename to get_match_var_names()?
                                                              // OpFilter, OpGroupBy and OpOrderBy are not adding their new variables (properties)

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent=0) const = 0;
    friend std::ostream& operator<<(std::ostream& os, const Op& b) {
        return b.print_to_ostream(os);
    }
};

#endif // BASE__OP_H_
