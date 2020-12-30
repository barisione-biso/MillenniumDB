#ifndef BASE__OP_OPTIONAL_H_
#define BASE__OP_OPTIONAL_H_

#include <string>
#include <vector>

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/grammar/query/query_ast.h"


// SELECT *
// MATCH (?y) OPTIONAL{ (?y)->(?x {age:65}) } OPTIONAL { (?y)->(?x:age:42) } => no es well designed
// MATCH (?y) OPTIONAL{ (?y)->(?x {age:65}) } OPTIONAL { (?y)->(?z:age:42) }
// MATCH (?x :Person) OPTIONAL { (?x {age:65}) } // OJO
// MATCH (?x {age:65}) OPTIONAL { (?x :Person) } // OJO
// Tal vez opcionales deberían prohibir añadir restricciones de properties o labels al padre
class OpOptional : public Op {
public:
    std::unique_ptr<Op> op;
    std::vector<std::unique_ptr<Op>> optionals;

    OpOptional(OpOptional&& op_optional) :
        op        (std::move(op_optional.op)),
        optionals (std::move(op_optional.optionals)) { }

    OpOptional(const query::ast::GraphPattern& graph_pattern) :
        op ( std::make_unique<OpMatch>(graph_pattern.pattern) )
    {
        for (auto& optional : graph_pattern.optionals) {
            if (optional.get().optionals.size() == 0) {
                optionals.emplace_back( std::make_unique<OpMatch>(optional.get().pattern) );
            } else {
                optionals.emplace_back( std::make_unique<OpOptional>(optional.get()) );
            }
        }
    }

    ~OpOptional() = default;


    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpOptional()\n";
        op->print_to_ostream(os, indent + 2);
        for (auto& optional : optionals) {
            optional->print_to_ostream(os, indent + 2);
        }
        return os;
    };

    std::set<std::string> get_var_names() const override {
        auto res = op->get_var_names();
        for (auto& child : optionals) {
            auto child_var_names = child->get_var_names();
            for (auto& var_name : child_var_names) {
                res.insert(var_name);
            }
        }
        return res;
    }
};
#endif // BASE__OP_OPTIONAL_H_
