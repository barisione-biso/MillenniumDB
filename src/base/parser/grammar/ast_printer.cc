#include "ast_printer.h"

ASTPrinter::ASTPrinter(std::ostream& out)
    : out(out), base_indent(0) {}

ASTPrinter::ASTPrinter(std::ostream& out, int_fast32_t base_indent)
    : out(out), base_indent(base_indent) {}


void ASTPrinter::indent(std::string str) const {
    int_fast32_t spaces = base_indent * tab_size;
    for (int_fast32_t i = 0; i < spaces; i++) {
        out << " ";
    }
    out << str;
}


void ASTPrinter::indent(std::string str, int_fast32_t extra_indent) const {
    int_fast32_t spaces = (base_indent + extra_indent) * tab_size;
    for (int_fast32_t i = 0; i < spaces; i++) {
        out << " ";
    }
    out << str;
}


void ASTPrinter::operator()(ast::Root const& r) const {
    indent("{\n");
    auto printer = ASTPrinter(out, base_indent+1);

    boost::apply_visitor(printer, r.selection);
    out << ",\n";
    printer(r.graph_pattern);
    out << ",\n";
    printer(r.where);
    indent("\n}\n");
}


void ASTPrinter::operator()(std::vector<ast::Element> const& select) const {
    indent("\"SELECT\": [\n");
    auto printer = ASTPrinter(out, base_indent+1);
    auto it = select.begin();
    while (it != select.end()) {
        printer(*it);
        ++it;
        if (it != select.end()) {
            out << ",\n";
        }
        else {
            out << "\n";
        }
    }
    indent("]");
}


void ASTPrinter::operator() (ast::All const&) const {
    indent("\"SELECT\": \"<ALL>\"");
}


void ASTPrinter::operator() (std::vector<ast::LinearPattern> const& graph_pattern) const {
    indent("\"MATCH\": [\n");
    auto printer = ASTPrinter(out, base_indent+1);
    auto it = graph_pattern.begin();
    while (it != graph_pattern.end()) {
        printer(*it);
        ++it;
        if (it != graph_pattern.end()) {
            out << ",\n";
        }
        else {
            out << "\n";
        }
    }
    indent("]");
}


void ASTPrinter::operator() (ast::LinearPattern const& linear_pattern) const {
    indent("{\n");
    auto printer = ASTPrinter(out, base_indent+1);
    printer(linear_pattern.root);
    for (auto const& step_path : linear_pattern.path) {
        out << ",\n";
        printer(step_path);
    }
    out << "\n";
    indent("}");
}


void ASTPrinter::operator() (ast::StepPath step_path) const {
    (*this)(step_path.edge);
    out << ",\n";
    (*this)(step_path.node);
}


void ASTPrinter::operator() (ast::Node node) const {
    indent("\"NODE\": {\n");
    indent("\"VAR\": ", 1);
    out << "\"" << node.var << "\",\n";
    indent("\"LABELS\": [", 1);
    auto label_iter = node.labels.begin();
    while (label_iter != node.labels.end()) {
        out << "\"" << *label_iter << "\"";
        ++label_iter;
        if (label_iter != node.labels.end()) {
            out << ", ";
        }
    }
    out << "],\n";
    indent("\"PROPERTIES\": [\n", 1);
    auto prop_iter = node.properties.begin();
    while (prop_iter != node.properties.end()) {
        indent("{ \"KEY\": ", 2);
        out << "\"" << (*prop_iter).key << "\"";
        out << ", \"VALUE\": ";
        boost::apply_visitor(*this, (*prop_iter).value);
        ++prop_iter;
        if (prop_iter != node.properties.end()) {
            out << " },\n";
        }
        else {
            out << " }\n";
        }
    }
    indent("]\n", 1);
    indent("}");
}


void ASTPrinter::operator() (ast::Edge edge) const {
    indent("\"EDGE\": {\n");
    indent("\"DIRECTION\": ", 1);
    if (edge.direction == ast::EdgeDirection::right) {
        out << "\"RIGHT\",\n";
    }
    else {
        out << "\"LEFT\",\n";
    }
    indent("\"VAR\": ", 1);
    out << "\"" << edge.var << "\",\n";
    indent("\"LABELS\": [", 1);
    auto label_iter = edge.labels.begin();
    while (label_iter != edge.labels.end()) {
        out << "\"" << *label_iter << "\"";
        ++label_iter;
        if (label_iter != edge.labels.end()) {
            out << ", ";
        }
    }
    out << "],\n";
    indent("\"PROPERTIES\": [\n", 1);
    auto prop_iter = edge.properties.begin();
    while (prop_iter != edge.properties.end()) {
        indent("{ \"KEY\": ", 2);
        out << "\"" << (*prop_iter).key << "\"";
        out << ", \"VALUE\": ";
        boost::apply_visitor(*this, (*prop_iter).value);
        ++prop_iter;
        if (prop_iter != edge.properties.end()) {
            out << " },\n";
        }
        else {
            out << " }\n";
        }
    }
    indent("]\n", 1);
    indent("}");
}


void ASTPrinter::operator()(ast::Element const& element) const {
    indent("{ ");
    if (!element.function.empty()) {
        out << "\"FUNCTION\": \"" << element.function << "\", ";
    }
    out << "\"KEY\": \"" << element.key << "\", ";
    out << "\"VAR\": \"" << element.variable << "\"";
    out << " }";
}


void ASTPrinter::operator()(boost::optional<ast::Formula> const& where) const {
    indent("\"WHERE\": {\n");
    if (where) {
        ast::Formula formula = static_cast<ast::Formula>(where.get());
        // auto printer = ASTPrinter(out, base_indent+1);
        // boost::apply_visitor(printer, formula.root);
        // out << ",\n";
        // boost::apply_visitor(printer, formula.path);
        // out << "\n";
    }
    indent("}");
}


void ASTPrinter::operator()(std::string const& text) const {
    out << "\"" << text << "\"";
}


void ASTPrinter::operator() (VarId const& var_id)   const {out << "VarId(" << var_id.id << ")"; }
void ASTPrinter::operator() (int const& n)          const {out << n; }
void ASTPrinter::operator() (float const& n)        const {out << n; }
void ASTPrinter::operator() (ast::And const& a)     const {out << a.str; }
void ASTPrinter::operator() (ast::Or const& a)      const {out << a.str; }
void ASTPrinter::operator() (ast::EQ const& a)      const {out << a.str; }
void ASTPrinter::operator() (ast::NE const& a)      const {out << a.str; }
void ASTPrinter::operator() (ast::GT const& a)      const {out << a.str; }
void ASTPrinter::operator() (ast::LT const& a)      const {out << a.str; }
void ASTPrinter::operator() (ast::GE const& a)      const {out << a.str; }
void ASTPrinter::operator() (ast::LE const& a)      const {out << a.str; }


void ASTPrinter::operator() (bool const& b) const {
    if (b)
        out << "TRUE";
    else
        out << "FALSE";
}