#ifndef GRAMMAR__VISITORS__ASSIGN_IDS_H
#define GRAMMAR__VISITORS__ASSIGN_IDS_H

#include "grammar/ast.h"
#include "grammar/exceptions.h"

#include <set>
#include <map>

#include <boost/variant.hpp>

#ifndef GRAMMAR__VISITORS
    typedef std::map<std::string, unsigned> str_int_map;
#endif

namespace visitors {

    // Assigns VarIDs to variables, renames empty variables with _1, _2, etc 
    // and checks for inconsistent calling of variables in SELECT and WHERE 
    // statements. Outputs the variable name to VarID map.
    class assignVarIDs
        : public boost::static_visitor<void> {

    private:
        str_int_map VarIDMap;
        std::set<std::string> assignedVars;
        unsigned totObjects; 
        unsigned totUnassignedVars;

    public:
        assignVarIDs() 
            : totObjects(0), totUnassignedVars(0) {};
        
        // Getter for VarIDMap
        str_int_map getVarIDMap() {
            return VarIDMap;
        }
        
        void operator()(ast::root & r) {
            for(auto & lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            boost::apply_visitor(*this, r.selection_);

            (*this)(r.where_);
        }

        void operator()(ast::edge & edge) {
            if(edge.variable_.empty()) {
                edge.variable_ = "_" + std::to_string(totUnassignedVars+1);
                totUnassignedVars++;
            }
            else {
                assignedVars.insert(edge.variable_);
            }
            const auto ptr = VarIDMap.insert({edge.variable_, totObjects});
            if (ptr.second) {
                totObjects++;
            }
        }

        void operator()(ast::node & node) {
            if(node.variable_.empty()) {
                node.variable_ = "_" + std::to_string(totUnassignedVars+1);
                totUnassignedVars++;
            }
            else {
                assignedVars.insert(node.variable_);
            }
            const auto ptr = VarIDMap.insert({node.variable_, totObjects});
            if (ptr.second) {
                totObjects++;
            }
        }

        void operator()(ast::linear_pattern & lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }
        }

        void operator()(std::vector<ast::element> & container) {
            for(auto & elem: container) {
                (*this)(elem);
            }
        }

        void operator()(ast::element & elem) {
            // Check variable is present in match
            const bool found = assignedVars.find(elem.variable_) != assignedVars.end();
            if(!found) {
                throw ast::SelectionError(elem.variable_);
            }

            const auto ptr = VarIDMap.insert({elem.variable_ + "." + elem.key_ , totObjects});
            if (ptr.second) {
                totObjects++;
            }
        }

        void operator()(ast::all_ const& a) {}

        void operator()(boost::optional<ast::formula> & formula) {
            if (formula) {
                ast::formula realFormula = static_cast<ast::formula>(formula.get());
                (*this)(realFormula);
            }
        }

        void operator()(ast::formula & formula) {
            boost::apply_visitor(*this, formula.root_);
            for (auto & sFormula: formula.path_) {
                boost::apply_visitor(*this, sFormula.cond_);
            }
        }

        void operator()(ast::statement & stat) {
            (*this)(stat.lhs_);
        }

    }; // class assignVarIDs

}; // namespace visitors

#endif // GRAMMAR__VISITORS__ASSIGN_IDS_H