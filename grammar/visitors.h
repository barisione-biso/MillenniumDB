#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <map>
#include <set>
#include <exception>

unsigned const tabsize = 2;

// Typedefs for returning structs
typedef std::map<std::string, unsigned> StrIntMap;
typedef std::map<unsigned, std::string> IntStrMap;
typedef std::map<unsigned, std::map<std::string, ast::value>> IntStrValMap;
typedef std::vector<std::array<unsigned, 3>> connectVect;

enum Entity {NODE, EDGE};

namespace ast {
    struct EntityError
        : public std::exception
    {
        const char * what() const throw() {
            return "Wrong variable assignation in MATCH statement";
        }
    };

     struct TypeError
        : public std::exception
    {
        std::string var;

        TypeError(std::string var)
            : var(var) {}

        const char * what() const throw() {
            return "Inconsistent value type in WHERE statement with MATCH statement";
        }
    };

    struct SelectionError
        : public std::exception
    {
        std::string var;

        SelectionError(std::string var)
            : var(var) {}
        
        const char * what() const throw() {
            return "A referenced variable is not in any MATCH statement";
        }
    };
}

namespace visitors {

    class whichVisitor
        : public boost::static_visitor<int>
    {
        public:

        int operator()(ast::value const& val) const {
            return val.which();
        }

        int operator()(ast::element const& elem) const {return -1;}
    }; // class whichVisitor


    class printer
        : public boost::static_visitor<>
    {
        unsigned indent;
        std::ostream& out; 

        public:

        // Constructor 
        printer(std::ostream& out, unsigned indent = 0)
            : out(out), indent(indent) {}
        
        void operator()(ast::root const& r) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Select> = ";
            boost::apply_visitor(printer(out, indent+tabsize), r.selection_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<GraphPattern> = [" << '\n';
            for (auto const& lPattern: r.graphPattern_) {
                tab(indent+2*tabsize);
                out << "<LinearPattern> = ";
                printer(out, indent+2*tabsize)(lPattern);
                out << ',' << '\n';
            }
            tab(indent+tabsize);
            out << ']' << '\n';
            tab(indent+tabsize);
            out << "<Where> = ";
            printer(out, indent+tabsize)(r.where_);
            out << '\n';
            tab(indent);
            out << '}' << '\n';
        }

        void operator()(boost::optional<ast::formula> const& formula) const {
            if(formula) {
                ast::formula realFormula = static_cast<ast::formula>(formula.get());
                out << '{' << '\n';
                tab(indent+tabsize);
                out << "<Formula> = ";
                boost::apply_visitor(printer(out, indent+tabsize), realFormula.root_);
                out << ',' << '\n';
                tab(indent+tabsize);
                out << "<Path> = [\n";
                for (auto const& sFormula: realFormula.path_) {
                    tab(indent+2*tabsize);
                    out << "<Op> = ";
                    boost::apply_visitor(printer(out, indent+2*tabsize), sFormula.op_);
                    out << ',' << '\n';
                    tab(indent+2*tabsize);
                    out << "<Formula> = ";
                    boost::apply_visitor(printer(out, indent+2*tabsize), sFormula.cond_);
                    out << ',' << '\n';
                }
                tab(indent+tabsize);
                out << ']' << '\n';
                tab(indent);
                out << '}';
            } else 
            {
                out << "[not present]" << '\n';
            }
        }

        void operator()(ast::formula const& formula) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Formula> = ";
            boost::apply_visitor(printer(out, indent+tabsize), formula.root_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Path> = [\n";
            for (auto const& sFormula: formula.path_) {
                tab(indent+2*tabsize);
                out << "<Op> = ";
                boost::apply_visitor(printer(out, indent+2*tabsize), sFormula.op_);
                out << ',' << '\n';
                tab(indent+2*tabsize);
                out << "<Formula> = ";
                boost::apply_visitor(printer(out, indent+2*tabsize), sFormula.cond_);
                out << ',' << '\n';
            }
            tab(indent+tabsize);
            out << ']' << '\n';
            tab(indent);
            out << '}';

        }

        void operator()(ast::element const& elem) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Function> = ";
            printer(out, indent+tabsize)(elem.function_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Variable> = ";
            printer(out, indent+tabsize)(elem.variable_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Key> = ";
            printer(out, indent+tabsize)(elem.key_);
            out << '\n';
            tab(indent);
            out << '}'; 
        }

        void operator()(ast::statement const& stat) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<LHS> = ";
            printer(out, indent+tabsize)(stat.lhs_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Comparator> = ";
            boost::apply_visitor(printer(out, indent+tabsize), stat.comparator_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<RHS> = ";
            boost::apply_visitor(printer(out, indent+tabsize), stat.rhs_);
            out << '\n';
            tab(indent);
            out << '}'; 
        }

        void operator()(ast::value const& val) const {
            boost::apply_visitor(printer(out, indent), val);
        }

        void operator()(ast::edge const& edge) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Variable> = \"" << edge.variable_ << "\",\n";
            tab(indent+tabsize);
            out << "<Labels> = [" << '\n';
            for (auto const& label: edge.labels_) {
                tab(indent+2*tabsize);
                out << label << ",\n";
            }
            tab(indent+tabsize);
            out << "]," << '\n';
            tab(indent+tabsize);
            out << "<Direction> = ";
            if(edge.isright_)
                out << "->," << '\n';
            else
                out << "<-," << '\n';
            tab(indent+tabsize);
            out << "<Properties> = {" << '\n';
            for (auto const& prop: edge.properties_) {
                tab(indent+2*tabsize);
                out << '"' <<  prop.key_ << "\" : ";
                boost::apply_visitor(printer(out, indent+2*tabsize), prop.value_);
                out << ',' << '\n';
            }
            tab(indent+tabsize);
            out << '}' << '\n';
            tab(indent);
            out << '}';
        }

        void operator()(ast::all_ const& a) const {
            out << "<All>";
        }

        void operator()(ast::node const& node) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Variable> = \"" << node.variable_ << "\",\n";
            tab(indent+tabsize);
            out << "<Labels> = [" << '\n';
            for (auto const& label: node.labels_) {
                tab(indent+2*tabsize);
                out << '"' << label << '"' << ",\n";
            }
            tab(indent+tabsize);
            out << "]," << '\n';
            tab(indent+tabsize);
            out << "<Properties> = {" << '\n';
            for (auto const& prop: node.properties_) {
                tab(indent+2*tabsize);
                out << '"' <<  prop.key_ << "\" : ";
                boost::apply_visitor(printer(out, indent+2*tabsize), prop.value_);
                out << ',' << '\n';
            }
            tab(indent+tabsize);
            out << '}' << '\n';
            tab(indent);
            out << '}';
        }

        void operator()(ast::linear_pattern const& lPattern) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Root> = ";
            printer(out, indent+tabsize)(lPattern.root_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Path> = [\n"; 
            for(auto const& stepPath: lPattern.path_) {
                tab(indent+2*tabsize);
                out << "<Edge> = ";
                printer(out, indent+2*tabsize)(stepPath.edge_);
                out << ',' << '\n';
                tab(indent+2*tabsize);
                out << "<Node> = ";
                printer(out, indent+2*tabsize)(stepPath.node_);
                out << ',' << '\n';
            }
            tab(indent+tabsize);
            out << ']' << '\n';
            tab(indent);
            out << '}';
        }

        void operator()(std::vector<ast::element> const& container) const {
            out << '[' << '\n';
            for(auto const& element: container) {
                tab(indent+tabsize);
                out << "<Element> = ";
                printer(out, indent+tabsize)(element);
                out << ',' << '\n';
            }
            tab(indent);
            out << ']';
        }

        void operator()(std::string const& text) const {
            out << '"' << text << '"';
        }

        void operator() (int const& n) const {out << n;}

        void operator() (double const& n) const {out << n;}

        void operator() (ast::and_ const& a) const {out << "AND";}

        void operator() (ast::or_ const& a) const {out << "OR";}

        void operator() (ast::eq_ const& a) const {out << "==";}

        void operator() (ast::neq_ const& a) const {out << "!=";}

        void operator() (ast::gt_ const& a) const {out << ">";}

        void operator() (ast::lt_ const& a) const {out << "<";}

        void operator() (ast::geq_ const& a) const {out << ">=";}

        void operator() (ast::leq_ const& a) const {out << "<=";}

        void operator() (bool const& b) const {
            if(b)
                out << "TRUE";
            else
                out << "FALSE";
        }

        void tab(unsigned spaces) const {
            for(int i = 0; i < spaces; i++) {
                out << ' ';
            }
        }
    }; // class printer


    class firstVisitor
        : public boost::static_visitor<void>
    {
        std::map<std::string, Entity> varToType;
        std::set<std::string> assignedVars;
        unsigned totUnasignedVars;

        public:

        firstVisitor() 
            : totUnasignedVars(0) {}
        
        void operator()(ast::root & r) {
            for(auto &lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            boost::apply_visitor(*this, r.selection_);

            (*this)(r.where_);
            
        }

        void operator()(ast::edge & edge) {
            if(edge.variable_.empty()) {
                edge.variable_ = "_" + std::to_string(totUnasignedVars+1);
                totUnasignedVars++;
            }
            else {
                edge.variable_ = "?" + edge.variable_;
                assignedVars.insert(edge.variable_);
            }
            auto search = varToType.find(edge.variable_);
            if (search != varToType.end()) {
                if (search->second != EDGE) 
                    throw ast::EntityError();
            }
            varToType[edge.variable_] = EDGE;
        }

        void operator()(ast::node & node) {
            if(node.variable_.empty()) {
                node.variable_ = "_" + std::to_string(totUnasignedVars+1);
                totUnasignedVars++;
            }
            else {
                node.variable_ = "?" + node.variable_;
                assignedVars.insert(node.variable_);
            }
            auto search = varToType.find(node.variable_);
            if (search != varToType.end()) { // Found
                if (search->second != NODE) 
                    throw ast::EntityError();
            }
            varToType[node.variable_] = NODE;
        }

        void operator()(ast::linear_pattern & lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }
        }

        void operator()(std::vector<ast::element> & container) {
            for(auto &elem: container) {
                (*this)(elem);
            }
        }

        void operator()(ast::element & elem) {
            if(elem.function_.empty()) {
                elem.function_ = "_none";
            } 
            elem.variable_ = "?" + elem.variable_;

            // Check variable is present in match
            const bool found = assignedVars.find(elem.variable_) != assignedVars.end();
            if(!found) {
                throw ast::SelectionError(elem.variable_);
            }
        }

        void operator()(boost::optional<ast::formula> & formula) {
            if(formula) {
                ast::formula realFormula = static_cast<ast::formula>(formula.get());
                boost::apply_visitor(*this, realFormula.root_);
                for (auto & sFormula: realFormula.path_) {
                    boost::apply_visitor(*this, sFormula.cond_);
                }
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
            boost::apply_visitor(*this, stat.rhs_);
        }

        // Dummy leaves 
         void operator()(ast::value & val)  {
            boost::apply_visitor(*this, val);
        }
        void operator()(ast::all_ & a) {}
        void operator()(std::string & text)  {}
        void operator() (int & n)  {}
        void operator() (double & n)  {}
        void operator() (bool const& b) const {}
    }; // class firstVisitor


    class secondVisitor
        : public boost::static_visitor<StrIntMap>
    {

        private:

        StrIntMap objectIdMap;
        unsigned totObjects; 

        public:

        // Constructor 
        secondVisitor() 
            : totObjects(0) {};
        
        StrIntMap operator()(ast::root const& r) {
            for(auto const& lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            return objectIdMap;
        }

        StrIntMap operator()(ast::edge const& edge) {
            const auto ptr = objectIdMap.insert({edge.variable_, totObjects});
            if (ptr.second) {
                totObjects++;
            }
            return objectIdMap;
        }

        StrIntMap operator()(ast::node const& node) {
            const auto ptr = objectIdMap.insert({node.variable_, totObjects});
            if (ptr.second) {
                totObjects++;
            }

            return objectIdMap;
        }

        StrIntMap operator()(ast::linear_pattern const& lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }

            return objectIdMap;
        }

    }; // class secondVisitor


    class thirdVisitor
        : public boost::static_visitor<IntStrMap>
    {
        StrIntMap idMap;
        IntStrMap labelMap;

        public:

        // Constructor 
        thirdVisitor(StrIntMap & idMap)
            : idMap(idMap) {}
        
        IntStrMap operator()(ast::root const& r) {
            for(auto const& lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            return labelMap;
        }

        IntStrMap operator()(ast::linear_pattern const& lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }

            return labelMap;
        }

        IntStrMap operator()(ast::edge const& edge) {
            for(auto const& label: edge.labels_) {
                labelMap.insert({idMap.at(edge.variable_), label});
            }
            return labelMap;
        }

        IntStrMap operator()(ast::node const& node) {
            for(auto const& label: node.labels_) {
                labelMap.insert({idMap.at(node.variable_), label});
            }
            return labelMap;
        }

    }; // class thirdVisitor


    class fourthVisitor
        : public boost::static_visitor<IntStrValMap>
    {
        StrIntMap idMap;
        IntStrValMap propertyMap;
        int storedWhich;

        public:

        // Constructor 
        fourthVisitor(StrIntMap & idMap, int which_ = -1, IntStrValMap pM = IntStrValMap())
            : idMap(idMap),  propertyMap(pM), storedWhich(which_) {}
        
        IntStrValMap operator()(ast::root const& r) {
            for(auto const& lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }

            (*this)(r.where_);
            return propertyMap;
        }

        IntStrValMap operator()(ast::linear_pattern const& lPattern) {
            (*this)(lPattern.root_);
            for(auto &sPath: lPattern.path_) {
                (*this)(sPath.edge_);
                (*this)(sPath.node_);
            }

            return propertyMap;
        }

        IntStrValMap operator()(ast::edge const& edge) {
            for(auto & prop: edge.properties_) {
                propertyMap[idMap.at(edge.variable_)][prop.key_] =  prop.value_;
            }
            return propertyMap;
        }

        IntStrValMap operator()(ast::node const& node) {
            for(auto & prop: node.properties_) {
                propertyMap[idMap.at(node.variable_)][prop.key_] = prop.value_;
            }
            return propertyMap;
        }

        IntStrValMap operator()(boost::optional<ast::formula> const& formula) {
            if(formula) {
                ast::formula realFormula = static_cast<ast::formula>(formula.get());
                boost::apply_visitor(*this, realFormula.root_);
                for (auto & sFormula: realFormula.path_) {
                    boost::apply_visitor(*this, sFormula.cond_);
                }
            }
            return propertyMap;
        }

        IntStrValMap operator()(ast::formula const& formula) {
            boost::apply_visitor(*this, formula.root_);
            for (auto & sFormula: formula.path_) {
                boost::apply_visitor(*this, sFormula.cond_);
            }

            return propertyMap;
        }

        IntStrValMap operator()(ast::statement const& stat) {
            int t = boost::apply_visitor(whichVisitor(), stat.rhs_);
            fourthVisitor(idMap, t, propertyMap)(stat.lhs_);

            return propertyMap;
        }

        IntStrValMap operator()(ast::element const& elem) {
            if(storedWhich > 0) {
                unsigned id_ = idMap.at(elem.variable_);
                std::map<std::string, ast::value> entMap = propertyMap.at(id_);
                auto foundIt = entMap.find(elem.key_);
                if(foundIt != entMap.end()) {
                    if (storedWhich != foundIt->second.which()) {
                        throw ast::TypeError(elem.variable_);
                    }
                }
            }
            return propertyMap;
        }
        
    }; // class fourthVisitor


    class fifthVisitor
        : public boost::static_visitor< connectVect >
    {
        StrIntMap idMap;
        connectVect connections;

        public:

        // Constructor 
        fifthVisitor(StrIntMap & idMap)
            : idMap(idMap) {}
        
        connectVect operator()(ast::root const& r) {
            for(auto const& lPattern: r.graphPattern_) {
                (*this)(lPattern);
            }
            return connections;
        }

        connectVect operator()(ast::linear_pattern const& lPattern) {
            unsigned id1, id2, id3;
            id1 = idMap.at(lPattern.root_.variable_);
            for(auto &sPath: lPattern.path_) {
                id2 = idMap.at(sPath.edge_.variable_);
                id3 = idMap.at(sPath.node_.variable_);
                if(sPath.edge_.isright_)
                    connections.push_back({id1, id2, id3});
                else
                    connections.push_back({id3, id2, id1});
                id1 = id3;
            }

            return connections;
        }
        
    }; // class fifthVisitor


} // namespace visitors


#endif