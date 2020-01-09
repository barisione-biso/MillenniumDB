#ifndef GRAMMAR__VISITORS__PRINTER_H
#define GRAMMAR__VISITORS__PRINTER_H

#include "../ast.h"

#include <boost/variant.hpp>
#include <boost/optional.hpp>

unsigned const tabsize = 2;

namespace visitors {

    // Prints the AST generated at parsing in a JSON-like
    // format to a given stream.
    class printer
        : public boost::static_visitor<void>
    {
        
        unsigned indent;
        std::ostream& out; 

        public:

        // Constructor 
        printer(std::ostream& out, unsigned indent = 0)
            : out(out), indent(indent) {}
        
        inline void operator()(ast::root const& r) const {
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

        inline void operator()(boost::optional<ast::formula> const& formula) const {
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

        inline void operator()(ast::formula const& formula) const {
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

        inline void operator()(ast::element const& elem) const {
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

        inline void operator()(ast::statement const& stat) const {
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

        inline void operator()(ast::value const& val) const {
            boost::apply_visitor(printer(out, indent), val);
        }

        inline void operator()(ast::edge const& edge) const {
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

        inline void operator()(ast::all_ const& a) const {
            out << "<All>";
        }

        inline void operator()(ast::node const& node) const {
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

        inline void operator()(ast::linear_pattern const& lPattern) const {
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

        inline void operator()(std::vector<ast::element> const& container) const {
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

        inline void operator()(std::string const& text) const {
            out << '"' << text << '"';
        }

        inline void operator() (int const& n)          const {out << n;}
        inline void operator() (double const& n)       const {out << n;}
        inline void operator() (ast::and_ const& a)    const {out << "AND";}
        inline void operator() (ast::or_ const& a)     const {out << "OR";}
        inline void operator() (ast::eq_ const& a)     const {out << "==";}
        inline void operator() (ast::neq_ const& a)    const {out << "!=";}
        inline void operator() (ast::gt_ const& a)     const {out << ">";}
        inline void operator() (ast::lt_ const& a)     const {out << "<";}
        inline void operator() (ast::geq_ const& a)    const {out << ">=";}
        inline void operator() (ast::leq_ const& a)    const {out << "<=";}

        inline void operator() (bool const& b) const {
            if(b)
                out << "TRUE";
            else
                out << "FALSE";
        }

        inline void tab(unsigned spaces) const {
            for(int i = 0; i < spaces; i++) {
                out << ' ';
            }
        }
    }; // class printer
}

#endif  // GRAMMAR__VISITORS__PRINTER_H