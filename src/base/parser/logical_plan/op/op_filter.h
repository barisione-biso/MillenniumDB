#ifndef BASE__OP_FILTER_H_
#define BASE__OP_FILTER_H_

#include "base/parser/logical_plan/op/op.h"
#include "base/graph/condition/constant_condition.h"
#include "base/graph/condition/conjunction.h"
#include "base/graph/condition/comparisons/equals.h"
#include "base/graph/condition/disjunction.h"
#include "base/graph/condition/negation.h"

#include <memory>

class OpFilter : public Op {
public:
    std::unique_ptr<Condition> condition;
    std::unique_ptr<Op> op;

    OpFilter(boost::optional<ast::Formula> const& optional_formula, std::unique_ptr<Op> op);
    void accept_visitor(OpVisitor&);
    std::unique_ptr<Condition> move_condition();
};

class FormulaVisitor : public boost::static_visitor<std::unique_ptr<Condition>> {

public:
    std::unique_ptr<Condition> operator()(boost::optional<ast::Formula> const& optional_formula) const {
        if (optional_formula) {
            ast::Formula formula = static_cast<ast::Formula>(optional_formula.get());
            return (*this)(formula);
        }
        else {
            return nullptr;
        }
    }


    std::unique_ptr<Condition> operator()(ast::Formula const& formula) const {
        std::vector<std::unique_ptr<Condition>> tmp_disjunction;
        auto tmp = (*this)(formula.root);

        for (auto const& step_formula : formula.path) {
            auto step = (*this)(step_formula.condition);
            if (step_formula.op.type() == typeid(ast::Or)) { // OR
                tmp_disjunction.push_back(std::move(tmp));
                tmp = std::move(step);
            }
            else { // AND
                if (tmp->type() == ConditionType::conjunction) { // already AND
                    Conjunction* conjuction = dynamic_cast<Conjunction*>(tmp.get());
                    conjuction->add(std::move(step));
                }
                else {
                    tmp = std::make_unique<Conjunction>();
                    Conjunction* conjuction = dynamic_cast<Conjunction*>(tmp.get());
                    conjuction->add(std::move(tmp));
                    conjuction->add(std::move(step));
                }
            }
        }
        if (tmp_disjunction.size() == 0) {
            return tmp;
        }
        else {
            return std::make_unique<Disjunction>(std::move(tmp_disjunction));
        }
    }


    std::unique_ptr<Condition> operator()(ast::Condition const& condition) const {
        if (condition.negation) {
            std::unique_ptr<Condition> cont = boost::apply_visitor(*this, condition.content);
            return std::make_unique<Negation>(std::move(cont));
        }
        else {
            return boost::apply_visitor(*this, condition.content);
        }
    }


    std::unique_ptr<Condition> operator()(ast::Statement const& statement) const {
        return std::make_unique<Equals>(statement); // TODO: check comparison type
    }
};

#endif // BASE__OP_FILTER_H_
