#ifndef GRAMMAR__VISITORS__ASSIGN_IDS_H
#define GRAMMAR__VISITORS__ASSIGN_IDS_H

#include "grammar/ast.h"
#include "grammar/exceptions.h"

#include <set>
#include <map>

#include <boost/variant.hpp>

using namespace std;

namespace visitors {


    // Assigns VarIDs to variables, renames empty variables with _1, _2, etc
    // and checks for inconsistent calling of variables in SELECT and WHERE
    // statements. Outputs the variable name to VarID map.
    class assignVarIDs : private boost::static_visitor<void> {

    private:
        map<string, uint_fast32_t> var2id;
        set<string> assigned_vars;
        uint_fast32_t total_vars = 0;
        uint_fast32_t unassigned_vars = 0;

    public:
        assignVarIDs() = default;

        map<string, uint_fast32_t> visit(ast::root & root) {
            (*this)(root);
            return var2id;
        }

        void operator()(ast::root & root) {
            for (auto & linear_pattern: root.graphPattern_) {
                (*this)(linear_pattern);
            }
            boost::apply_visitor(*this, root.selection_);

            (*this)(root.where_);
        }

        void operator()(ast::edge & edge) {
            if (edge.variable_.empty()) {
                edge.variable_ = "_" + std::to_string(unassigned_vars++);
            }
            else {
                assigned_vars.insert(edge.variable_);
            }
            const auto ptr = var2id.insert({edge.variable_, total_vars});
            if (ptr.second) {
                total_vars++;
            }
        }

        void operator()(ast::node & node) {
            if(node.variable_.empty()) {
                node.variable_ = "_" + std::to_string(unassigned_vars++);
            }
            else {
                assigned_vars.insert(node.variable_);
            }
            const auto ptr = var2id.insert({node.variable_, total_vars});
            if (ptr.second) {
                total_vars++;
            }
        }

        void operator()(ast::linear_pattern & linear_pattern) {
            (*this)(linear_pattern.root_);
            for (auto &sPath: linear_pattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }
        }

        void operator()(std::vector<ast::element> & container) {
            for (auto & elem: container) {
                (*this)(elem);
            }
        }

        void operator()(ast::element & elem) {
            // Check variable is present in match
            const bool found = assigned_vars.find(elem.variable_) != assigned_vars.end();
            if (!found) {
                throw ast::SelectionError(elem.variable_);
            }

            const auto ptr = var2id.insert({elem.variable_ + "." + elem.key_ , total_vars});
            if (ptr.second) {
                total_vars++;
            }
        }

        void operator()(ast::all_ const&) {}

        void operator()(boost::optional<ast::formula> & formula) {
            if (formula) {
                ast::formula realFormula = static_cast<ast::formula>(formula.get());
                (*this)(realFormula);
            }
        }

        void operator()(ast::formula & formula) {
            boost::apply_visitor(*this, formula.root_.content_);
            for (auto & sFormula: formula.path_) {
                boost::apply_visitor(*this, sFormula.cond_.content_);
            }
        }

        void operator()(ast::statement & stat) {
            (*this)(stat.lhs_);
        }

    }; // class assignVarIDs

}; // namespace visitors

#endif // GRAMMAR__VISITORS__ASSIGN_IDS_H