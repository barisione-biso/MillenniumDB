#ifndef GRAMMAR__VISITORS__FORMULA_TREE_H
#define GRAMMAR__VISITORS__FORMULA_TREE_H

#include "grammar/ast.h"

#include <boost/variant.hpp>

namespace formtree {
    
    struct statement {
        ast::element lhs;
        ast::comparator comp;
        boost::variant<ast::element, ast::value> rhs;

        statement() = default;
        statement(ast::statement const& st)
            : lhs(st.lhs_), comp(st.comparator_), rhs(st.rhs_) {}

    }; // struct statement

    struct and_op;
    struct or_op;
    struct not_op;

    typedef boost::variant<boost::recursive_wrapper<and_op>, 
                           boost::recursive_wrapper<or_op>, 
                           boost::recursive_wrapper<not_op>, 
                           statement> formula;

    struct and_op {
        std::vector<formula> content;
    
        and_op(std::vector<formula> & cnt)
            : content(cnt) {}
    };

    struct or_op {
        std::vector<formula> content;
        or_op(std::vector<formula> & cnt)
            : content(cnt) {}
    };

    struct not_op {
        formula content;

        not_op(formula &cnt)
            : content(cnt) {}
        not_op(formula &&cnt)
            : content(cnt) {}
    };

    

} // namespace formtree

namespace visitors {

    class formulaTree: 
        public boost::static_visitor<formtree::formula> {

        typedef formtree::formula    formula;

    public:

        formula operator()(ast::root const& r) const {
            return (*this)(r.where_);
        }

        formula operator()(boost::optional<ast::formula> const& formula) const {
            if(formula) {
                ast::formula realFormula = static_cast<ast::formula>(formula.get());
                return (*this)(realFormula);
            } 

            return formtree::statement();
        }

        formula operator()(ast::formula const& formula) const {
            std::vector<formtree::formula> tmp1, tmp2;

            tmp1.push_back((*this)(formula.root_));

            for (auto const& sFormula: formula.path_) {
                if (sFormula.op_.type() == typeid(ast::or_)) {
                    if (tmp1.size() > 1) {
                        tmp2.push_back(formtree::and_op(tmp1));
                    }
                    else {
                        tmp2.push_back(tmp1.back());
                    }
                    tmp1.clear();
                }
                tmp1.push_back((*this)(sFormula.cond_));
            }
            if(tmp2.empty()) {
                if (tmp1.size() > 1) {
                    return formtree::and_op(tmp1);
                }
                return tmp1.back();
            }
            return formtree::or_op(tmp2);
            
        }

        formula operator()(ast::condition const& cond) const {
            if(cond.negation_) {
                formula cont = boost::apply_visitor(*this, cond.content_);
                return formtree::not_op(cont);
            }
            else {
                return boost::apply_visitor(*this, cond.content_); 
            }
        }
    
        formula operator()(ast::statement const& stat) const {
            return formtree::statement(stat);
        }
       
    }; // class formulaTree

} // namespace visitors

#endif // GRAMMAR__VISITORS__FORMULA_TREE_H