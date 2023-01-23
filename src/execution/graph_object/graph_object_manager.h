#pragma once

#include <ostream>
#include <sstream>

#include "base/exceptions.h"
#include "base/graph_object/iri_tmp2.h"
#include "base/graph_object/string_tmp2.h"
#include "base/graph_object/literal_datatype_tmp2.h"
#include "base/graph_object/literal_language_tmp2.h"
#include "base/query/sparql/decimal.h"
#include "base/graph_object/anonymous_node.h"
#include "base/graph_object/edge.h"
#include "base/graph_object/graph_object.h"
#include "base/graph_object/not_found_object.h"
#include "base/graph_object/null_graph_object.h"
#include "base/graph_object/path.h"
#include "base/graph_object/string_external.h"
#include "base/graph_object/string_inlined.h"
#include "base/graph_object/string_tmp.h"
#include "base/graph_object/iri_external.h"
#include "base/graph_object/iri_inlined.h"
#include "base/graph_object/iri_tmp.h"
#include "base/graph_object/literal_datatype_inlined.h"
#include "base/graph_object/literal_datatype_external.h"
#include "base/graph_object/literal_datatype_tmp.h"
#include "base/graph_object/literal_language_inlined.h"
#include "base/graph_object/literal_language_external.h"
#include "base/graph_object/literal_language_tmp.h"
#include "base/graph_object/datetime.h"
#include "base/graph_object/decimal_inlined.h"
#include "base/graph_object/decimal_external.h"
#include "base/graph_object/decimal_tmp.h"
#include "execution/graph_object/char_iter.h"
#include "execution/graph_object/graph_object_factory.h"
#include "execution/graph_object/graph_object_types.h"
#include "storage/string_manager.h"
#include "storage/temporal_manager.h"
#include "query_optimizer/rdf_model/rdf_model.h"

// Types that can be saved both inline and external
enum SPARQL_COMPLEX_TYPES {
    NONE,
    DECIMAL,
    IRI,
    STRING,
    LITERAL_LANGUAGE,
    LITERAL_DATATYPE,
};

struct GraphObjectManager {
    static std::ostream& print(std::ostream& os, const GraphObject& graph_obj) {
        switch (graph_obj.type) {
        case GraphObjectType::NAMED_INLINED:
            return os << GraphObjectInterpreter::get<StringInlined>(graph_obj).id;
        case GraphObjectType::NAMED_EXTERNAL:
            string_manager.print(os,GraphObjectInterpreter::get<StringExternal>(graph_obj).external_id);
            return os;
        case GraphObjectType::NAMED_TMP:
            return os << *GraphObjectInterpreter::get<StringTmp>(graph_obj).str;
        case GraphObjectType::STR_INLINED:
            return os << '"' << GraphObjectInterpreter::get<StringInlined>(graph_obj).id << '"';
        case GraphObjectType::STR_EXTERNAL:
            os << '"';
            string_manager.print(os, GraphObjectInterpreter::get<StringExternal>(graph_obj).external_id);
            return os << '"';
        case GraphObjectType::STR_TMP:
            return os << '"' << *GraphObjectInterpreter::get<StringTmp>(graph_obj).str << '"';
        case GraphObjectType::EDGE:
            return os << "_e" << GraphObjectInterpreter::get<Edge>(graph_obj).id;
        case GraphObjectType::ANON:
            return os << "_a" << GraphObjectInterpreter::get<AnonymousNode>(graph_obj).id;
        case GraphObjectType::NULL_OBJ:
            return os << "null";
        case GraphObjectType::NOT_FOUND:
            return os << "NotFoundObj";
        case GraphObjectType::INT:
            return os << GraphObjectInterpreter::get<int64_t>(graph_obj);
        case GraphObjectType::FLOAT:
            return os << GraphObjectInterpreter::get<float>(graph_obj);
        case GraphObjectType::BOOL:
            return os << (GraphObjectInterpreter::get<bool>(graph_obj) ? "true" : "false");
        case GraphObjectType::PATH: {
            auto path = GraphObjectInterpreter::get<Path>(graph_obj);
            path.path_printer->print(os, path.path_id);
            return os;
        }
        default:
            throw LogicException("Unmanaged case print");
        }
    }

    static std::ostream& print_rdf(std::ostream& os, const GraphObject& graph_obj) {
        switch (graph_obj.type) {
        case GraphObjectType::STR_INLINED:
            return os << '"' << GraphObjectInterpreter::get<StringInlined>(graph_obj).id << '"';
        case GraphObjectType::STR_EXTERNAL:
            os << '"';
            string_manager.print(os, GraphObjectInterpreter::get<StringExternal>(graph_obj).external_id);
            return os << '"';
        case GraphObjectType::STR_TMP:
            return os << '"' << *GraphObjectInterpreter::get<StringTmp>(graph_obj).str << '"';
        case GraphObjectType::ANON:
            return os << "_:b" << GraphObjectInterpreter::get<AnonymousNode>(graph_obj).id;
        case GraphObjectType::NULL_OBJ:
            return os;
        case GraphObjectType::NOT_FOUND:
            return os << "NotFoundObj";
        case GraphObjectType::BOOL:
            return os << '"' << (GraphObjectInterpreter::get<bool>(graph_obj) ? "true" : "false") << "\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
        case GraphObjectType::IRI_INLINED: {
            auto iri_inl = GraphObjectInterpreter::get<IriInlined>(graph_obj);

            return os << '<'
                      << rdf_model.catalog().prefixes[iri_inl.prefix_id]
                      << iri_inl.id
                      << '>';
        }
        case GraphObjectType::IRI_EXTERNAL: {
            auto iri_ext = GraphObjectInterpreter::get<IriExternal>(graph_obj);

            uint8_t prefix_id = (iri_ext.external_id & 0x00FF'0000'0000'0000UL) >> 48;
            uint64_t iri_id = iri_ext.external_id & 0x0000'FFFF'FFFF'FFFFUL;

            os << '<'
               << rdf_model.catalog().prefixes[prefix_id];
            string_manager.print(os, iri_id);
            return os << '>';
        }
        case GraphObjectType::IRI_TMP:
            return os << '<'
                      << *GraphObjectInterpreter::get<IriTmp>(graph_obj).str
                      << '>';
        case GraphObjectType::LITERAL_DATATYPE_INLINED: {
            auto ld_inl = GraphObjectInterpreter::get<LiteralDatatypeInlined>(graph_obj);
            os << '"'
               << ld_inl.id;
            print_datatype_rdf(os, ld_inl.datatype_id);
            return os;
        }
        case GraphObjectType::LITERAL_DATATYPE_EXTERNAL: {
            auto ld_ext = GraphObjectInterpreter::get<LiteralDatatypeExternal>(graph_obj);
            uint64_t str_id = ld_ext.external_id & 0x0000'00FF'FFFF'FFFFUL;
            uint64_t datatype_id = (ld_ext.external_id & 0x00FF'FF00'0000'0000UL) >> 40;

            os << '"';
            string_manager.print(os, str_id);
            print_datatype_rdf(os, datatype_id);
            return os;
        }
        case GraphObjectType::LITERAL_DATATYPE_TMP: {
            auto ld_tmp = GraphObjectInterpreter::get<LiteralDatatypeTmp>(graph_obj);

            return os << '"'
                      << ld_tmp.ld->str
                      << "\"^^<"
                      << ld_tmp.ld->datatype
                      << '>';
        }
        case GraphObjectType::LITERAL_LANGUAGE_INLINED: {
            auto ll_inl = GraphObjectInterpreter::get<LiteralLanguageInlined>(graph_obj);
            os << '"'
               << ll_inl.id;
            print_language_rdf(os, ll_inl.language_id);
            return os;
        }
        case GraphObjectType::LITERAL_LANGUAGE_EXTERNAL: {
            auto ll_ext = GraphObjectInterpreter::get<LiteralLanguageExternal>(graph_obj);
            uint64_t str_id = ll_ext.external_id & 0x0000'00FF'FFFF'FFFFUL;
            uint64_t language_id = (ll_ext.external_id & 0x00FF'FF00'0000'0000UL) >> 40;
            os << '"';
            string_manager.print(os, str_id);
            print_language_rdf(os, language_id);
            return os;
        }
        case GraphObjectType::LITERAL_LANGUAGE_TMP: {
            auto ll_tmp = GraphObjectInterpreter::get<LiteralLanguageTmp>(graph_obj);

            return os << '"'
                      << ll_tmp.ll->str
                      << "\"@"
                      << ll_tmp.ll->language;
        }
        case GraphObjectType::DATETIME: {
            return os << '"'
                      << GraphObjectInterpreter::get<DateTime>(graph_obj).get_value_string()
                      << "\"^^<http://www.w3.org/2001/XMLSchema#dateTime>";
        }
        case GraphObjectType::DECIMAL_INLINED: {
            return os << '"'
                      << GraphObjectInterpreter::get<DecimalInlined>(graph_obj).get_value_string()
                      << "\"^^<http://www.w3.org/2001/XMLSchema#decimal>";
        }
        case GraphObjectType::DECIMAL_EXTERNAL: {
            os << '"';
            std::stringstream ss;
            string_manager.print(ss, GraphObjectInterpreter::get<DecimalExternal>(graph_obj).external_id);
            Decimal dec;
            dec.from_external(ss.str());
            os << dec.to_string();
            return os << "\"^^<http://www.w3.org/2001/XMLSchema#decimal>";
        }
        case GraphObjectType::DECIMAL_TMP: {
            return os << '"'
                      << *GraphObjectInterpreter::get<DecimalTmp>(graph_obj).str
                      << "\"^^<http://www.w3.org/2001/XMLSchema#decimal>";
        }
        case GraphObjectType::IRI_TMP2: {
            auto iri_tmp = GraphObjectInterpreter::get<IriTmp2>(graph_obj);

            uint8_t prefix_id = (iri_tmp.temporal_id & 0x00FF'0000'0000'0000UL) >> 48;
            uint64_t iri_id = iri_tmp.temporal_id & 0x0000'FFFF'FFFF'FFFFUL;

            os << '<'
               << rdf_model.catalog().prefixes[prefix_id];
            temporal_manager.print_str(os, iri_id); // gets string from id
            return os << '>';
        }
        case GraphObjectType::STR_TMP2: {
            auto string_tmp = GraphObjectInterpreter::get<StringTmp2>(graph_obj);
            os << '"';
            temporal_manager.print_str(os, string_tmp.temporal_id); // gets string from id
            return os << '"';
        }
        case GraphObjectType::LITERAL_DATATYPE_TMP2: {
            auto ld_tmp = GraphObjectInterpreter::get<LiteralDatatypeTmp2>(graph_obj);
            uint64_t str_id = ld_tmp.temporal_id & 0x0000'00FF'FFFF'FFFFUL;
            uint64_t datatype_id = (ld_tmp.temporal_id & 0x00FF'FF00'0000'0000UL) >> 40;

            os << '"';
            temporal_manager.print_str(os, str_id);
            print_datatype_rdf(os, datatype_id);
            return os;
        }
        case GraphObjectType::LITERAL_LANGUAGE_TMP2: {
            auto ll_temp = GraphObjectInterpreter::get<LiteralLanguageTmp2>(graph_obj);
            uint64_t str_id = ll_temp.temporal_id & 0x0000'00FF'FFFF'FFFFUL;
            uint64_t language_id = (ll_temp.temporal_id & 0x00FF'FF00'0000'0000UL) >> 40;
            os << '"';
            temporal_manager.print_str(os, str_id);
            print_language_rdf(os, language_id);
            return os;
        }
        case GraphObjectType::PATH: {
            auto path = GraphObjectInterpreter::get<Path>(graph_obj);
            path.path_printer->print(os, path.path_id);
            return os;
        }
        case GraphObjectType::INT: {
            return os << '"'
                      << GraphObjectInterpreter::get<int64_t>(graph_obj)
                      << "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
        }
        case GraphObjectType::FLOAT: {
            return os << '"'
                      << GraphObjectInterpreter::get<float>(graph_obj)
                      << "\"^^<http://www.w3.org/2001/XMLSchema#float>";
        }
        default:
            throw LogicException("Unmanaged case print_rdf");
        }
    }

    static std::ostream& print_rdf_json(std::ostream& os, const GraphObject& graph_obj) {
        switch (graph_obj.type) {
        case GraphObjectType::STR_INLINED:
            return os << "{\"type\":\"literal\",\"value\":\"" << GraphObjectInterpreter::get<StringInlined>(graph_obj).id << "\"}";
        case GraphObjectType::STR_EXTERNAL:
            os << "{\"type\":\"literal\",\"value\":\"";
            string_manager.print(os, GraphObjectInterpreter::get<StringExternal>(graph_obj).external_id);
            return os << "\"}";
        case GraphObjectType::STR_TMP:
            return os << "{\"type\":\"literal\",\"value\":\"" << *GraphObjectInterpreter::get<StringTmp>(graph_obj).str << "\"}";
        case GraphObjectType::ANON:
            return os << "{\"type\":\"bnode\",\"value\":\"" << "_:b" << GraphObjectInterpreter::get<AnonymousNode>(graph_obj).id << "\"}";
        case GraphObjectType::BOOL:
            return os << "{\"type\":\"literal\",\"value\":\"" << (GraphObjectInterpreter::get<bool>(graph_obj) ? "true" : "false")
                      << "\",\"datatype\":\"http://www.w3.org/2001/XMLSchema#boolean\"}";
        case GraphObjectType::IRI_INLINED: {
            auto iri_inl = GraphObjectInterpreter::get<IriInlined>(graph_obj);

            return os << "{\"type\":\"uri\",\"value\":\""
                      << rdf_model.catalog().prefixes[iri_inl.prefix_id]
                      << iri_inl.id
                      << "\"}";
        }
        case GraphObjectType::IRI_EXTERNAL: {
            auto iri_ext = GraphObjectInterpreter::get<IriExternal>(graph_obj);

            uint8_t prefix_id = (iri_ext.external_id & 0x00FF'0000'0000'0000UL) >> 48;
            uint64_t iri_id = iri_ext.external_id & 0x0000'FFFF'FFFF'FFFFUL;

            os << "{\"type\":\"uri\",\"value\":\""
               << rdf_model.catalog().prefixes[prefix_id];
            string_manager.print(os, iri_id);
            return os << "\"}";
        }
        case GraphObjectType::IRI_TMP: {
            return os << "{\"type\":\"uri\",\"value\":\""
                      << *GraphObjectInterpreter::get<IriTmp>(graph_obj).str
                      << "\"}";
        }
        case GraphObjectType::LITERAL_DATATYPE_INLINED: {
            auto ld_inl = GraphObjectInterpreter::get<LiteralDatatypeInlined>(graph_obj);

            return os << "{\"type\":\"literal\",\"value\":\""
                      << ld_inl.id
                      << "\",\"datatype\":\""
                      << rdf_model.catalog().datatypes[ld_inl.datatype_id]
                      << "\"}";
        }
        case GraphObjectType::LITERAL_DATATYPE_EXTERNAL: {
            auto ld_ext = GraphObjectInterpreter::get<LiteralDatatypeExternal>(graph_obj);

            uint16_t datatype_id = (ld_ext.external_id & 0x00FF'FF00'0000'0000UL) >> 40;
            uint64_t str_id      = ld_ext.external_id & 0x0000'00FF'FFFF'FFFFUL;

            os << "{\"type\":\"literal\",\"value\":\"";
            string_manager.print(os, str_id);
            return os << "\",\"datatype\":\""
                      << rdf_model.catalog().datatypes[datatype_id]
                      << "\"}";
        }
        case GraphObjectType::LITERAL_DATATYPE_TMP: {
            auto ld_tmp = GraphObjectInterpreter::get<LiteralDatatypeTmp>(graph_obj);

            return os << "{\"type\":\"literal\",\"value\":\""
                      << ld_tmp.ld->str
                      << "\",\"datatype\":\""
                      << ld_tmp.ld->datatype
                      << "\"}";
        }
        case GraphObjectType::LITERAL_LANGUAGE_INLINED: {
            auto ll_inl = GraphObjectInterpreter::get<LiteralLanguageInlined>(graph_obj);

            return os << "{\"type\":\"literal\",\"value\":\""
                      << ll_inl.id
                      << "\",\"xml:lang\":\""
                      << rdf_model.catalog().languages[ll_inl.language_id]
                      << "\"}";
        }
        case GraphObjectType::LITERAL_LANGUAGE_EXTERNAL: {
            auto ll_ext = GraphObjectInterpreter::get<LiteralLanguageExternal>(graph_obj);

            uint16_t language_id = (ll_ext.external_id & 0x00FF'FF00'0000'0000UL) >> 40;
            uint64_t str_id      = ll_ext.external_id & 0x0000'00FF'FFFF'FFFFUL;

            os << "{\"type\":\"literal\",\"value\":\"";
            string_manager.print(os, str_id);
            return os << "\",\"xml:lang\":\""
                      << rdf_model.catalog().languages[language_id]
                      << "\"}";
        }
        case GraphObjectType::LITERAL_LANGUAGE_TMP: {
            auto ll_tmp = GraphObjectInterpreter::get<LiteralLanguageTmp>(graph_obj);

            return os << "{\"type\":\"literal\",\"value\":\""
                      << ll_tmp.ll->str
                      << "\",\"xml:lang\":\""
                      << ll_tmp.ll->language
                      << "\"}";
        }
        case GraphObjectType::DATETIME: {
            return os << "{\"type\":\"literal\",\"value\":\""
                      << GraphObjectInterpreter::get<DateTime>(graph_obj).get_value_string()
                      << "\",\"datatype\":\"http://www.w3.org/2001/XMLSchema#dateTime\"}";
        }
        case GraphObjectType::INT: {
            return os << "{\"type\":\"literal\",\"value\":\""
                      << GraphObjectInterpreter::get<int64_t>(graph_obj)
                      << "\",\"datatype\":\"http://www.w3.org/2001/XMLSchema#integer\"}";
        }
        case GraphObjectType::FLOAT: {
            return os << "{\"type\":\"literal\",\"value\":\""
                      << GraphObjectInterpreter::get<float>(graph_obj)
                      << "\",\"datatype\":\"http://www.w3.org/2001/XMLSchema#float\"}";
        }
        case GraphObjectType::DECIMAL_INLINED: {
            return os << "{\"type\":\"literal\",\"value\":\""
                      << GraphObjectInterpreter::get<DecimalInlined>(graph_obj).get_value_string()
                      << "\",\"datatype\":\"http://www.w3.org/2001/XMLSchema#decimal\"}";
        }
        case GraphObjectType::DECIMAL_EXTERNAL: {
            std::stringstream ss;
            string_manager.print(ss, GraphObjectInterpreter::get<DecimalExternal>(graph_obj).external_id);
            Decimal dec;
            dec.from_external(ss.str());
            return os << "{\"type\":\"literal\",\"value\":\""
                      << dec
                      << "\",\"datatype\":\"http://www.w3.org/2001/XMLSchema#decimal\"}";
        }
        case GraphObjectType::DECIMAL_TMP: {
            return os << "{\"type\":\"literal\",\"value\":\""
                      << *GraphObjectInterpreter::get<DecimalTmp>(graph_obj).str
                      << "\",\"datatype\":\"http://www.w3.org/2001/XMLSchema#decimal\"}";
        }
        case GraphObjectType::PATH: {
            auto path = GraphObjectInterpreter::get<Path>(graph_obj);
            path.path_printer->print(os, path.path_id);
            return os;
        }
        case GraphObjectType::NULL_OBJ:
            // TODO: null print shouldn't be called?
            // return os;
        case GraphObjectType::NOT_FOUND:
            // return os << "NotFoundObj";
        default:
            throw LogicException("Unmanaged case");
        }
    }

    static std::ostream& print_rdf_compressed(std::ostream& os, const GraphObject& graph_obj) {
        switch (graph_obj.type) {
        case GraphObjectType::STR_INLINED:
            return os << '"' << GraphObjectInterpreter::get<StringInlined>(graph_obj).id << '"';
        case GraphObjectType::STR_EXTERNAL:
            os << '"';
            string_manager.print(os, GraphObjectInterpreter::get<StringExternal>(graph_obj).external_id);
            return os << '"';
        case GraphObjectType::STR_TMP:
            return os << '"' << *GraphObjectInterpreter::get<StringTmp>(graph_obj).str << '"';
        case GraphObjectType::ANON:
            return os << "_:b" << GraphObjectInterpreter::get<AnonymousNode>(graph_obj).id;
        case GraphObjectType::NULL_OBJ:
            return os;
        case GraphObjectType::NOT_FOUND:
            return os << "NotFoundObj";
        case GraphObjectType::BOOL:
            return os << '"' << (GraphObjectInterpreter::get<bool>(graph_obj) ? "true" : "false") << "\"^^xsd:boolean";
        case GraphObjectType::IRI_INLINED: {
            auto iri_inl = GraphObjectInterpreter::get<IriInlined>(graph_obj);

            if (iri_inl.prefix_id != 0) {
                // Print using alias from prefixes file
                return os << rdf_model.catalog().aliases[iri_inl.prefix_id]
                          << ':'
                          << iri_inl.id;
            }
            else {
                return os << '<'
                          << iri_inl.id
                          << '>';
            }
        }
        case GraphObjectType::IRI_EXTERNAL: {
            auto iri_ext = GraphObjectInterpreter::get<IriExternal>(graph_obj);

            uint8_t prefix_id = (iri_ext.external_id & 0x00FF'0000'0000'0000UL) >> 48;
            uint64_t iri_id = iri_ext.external_id & 0x0000'FFFF'FFFF'FFFFUL;

            if (prefix_id != 0) {
                // Print using alias from prefixes file
                os << rdf_model.catalog().aliases[prefix_id]
                   << ':';
                string_manager.print(os, iri_id);
                return os;
            }
            else {
                os << '<';
                string_manager.print(os, iri_id);
                return os << '>';
            }
        }
        case GraphObjectType::IRI_TMP:
            return os << '<'
                      << *GraphObjectInterpreter::get<IriTmp>(graph_obj).str
                      << '>';
        case GraphObjectType::LITERAL_DATATYPE_INLINED: {
            auto ld_inl = GraphObjectInterpreter::get<LiteralDatatypeInlined>(graph_obj);

            return os << '"'
                      << ld_inl.id
                      << "\"^^<"
                      << rdf_model.catalog().datatypes[ld_inl.datatype_id]
                      << '>';
        }
        case GraphObjectType::LITERAL_DATATYPE_EXTERNAL: {
            auto ld_ext = GraphObjectInterpreter::get<LiteralDatatypeExternal>(graph_obj);

            uint16_t datatype_id = (ld_ext.external_id & 0x00FF'FF00'0000'0000UL) >> 40;
            uint64_t str_id      = ld_ext.external_id & 0x0000'00FF'FFFF'FFFFUL;

            os << '"';
            string_manager.print(os, str_id);
            return os << "\"^^<"
                      << rdf_model.catalog().datatypes[datatype_id]
                      << '>';
        }
        case GraphObjectType::LITERAL_DATATYPE_TMP: {
            auto ld_tmp = GraphObjectInterpreter::get<LiteralDatatypeTmp>(graph_obj);

            return os << '"'
                      << ld_tmp.ld->str
                      << "\"^^<"
                      << ld_tmp.ld->datatype
                      << '>';
        }
        case GraphObjectType::LITERAL_LANGUAGE_INLINED: {
            auto ll_inl = GraphObjectInterpreter::get<LiteralLanguageInlined>(graph_obj);

            return os << '"'
                      << ll_inl.id
                      << "\"@"
                      << rdf_model.catalog().languages[ll_inl.language_id];
        }
        case GraphObjectType::LITERAL_LANGUAGE_EXTERNAL: {
            auto ll_ext = GraphObjectInterpreter::get<LiteralLanguageExternal>(graph_obj);

            uint16_t language_id = (ll_ext.external_id & 0x00FF'FF00'0000'0000UL) >> 40;
            uint64_t str_id      = ll_ext.external_id & 0x0000'00FF'FFFF'FFFFUL;

            os << '"';
            string_manager.print(os, str_id);
            return os << "\"@"
                      << rdf_model.catalog().languages[language_id];
        }
        case GraphObjectType::LITERAL_LANGUAGE_TMP: {
            auto ll_tmp = GraphObjectInterpreter::get<LiteralLanguageTmp>(graph_obj);

            return os << '"'
                      << ll_tmp.ll->str
                      << "\"@"
                      << ll_tmp.ll->language;
        }
        case GraphObjectType::DATETIME: {
            return os << '"'
                      << GraphObjectInterpreter::get<DateTime>(graph_obj).get_value_string()
                      << "\"^^xsd:dateTime";
        }
        case GraphObjectType::DECIMAL_INLINED: {
            return os << '"'
                      << GraphObjectInterpreter::get<DecimalInlined>(graph_obj).get_value_string()
                      << "\"^^xsd:decimal";
        }
        case GraphObjectType::DECIMAL_EXTERNAL: {
            os << '"';
            string_manager.print(os, GraphObjectInterpreter::get<DecimalExternal>(graph_obj).external_id);
            return os << "\"^^xsd:decimal";
        }
        case GraphObjectType::DECIMAL_TMP: {
            return os << '"'
                      << *GraphObjectInterpreter::get<DecimalTmp>(graph_obj).str
                      << "\"^^xsd:decimal";
        }
        case GraphObjectType::PATH: {
            auto path = GraphObjectInterpreter::get<Path>(graph_obj);
            path.path_printer->print(os, path.path_id);
            return os;
        }
        case GraphObjectType::INT: {
            return os << '"'
                      << GraphObjectInterpreter::get<int64_t>(graph_obj)
                      << "\"^^xsd:integer";
        }
        case GraphObjectType::FLOAT: {
            return os << '"'
                      << GraphObjectInterpreter::get<float>(graph_obj)
                      << "\"^^xsd:float";
        }
        default:
            throw LogicException("Unmanaged case print_rdf_compressed");
        }
    }


    static bool equal(const GraphObject& lhs, const GraphObject& rhs) {
        return lhs.type == rhs.type && lhs.encoded_value == rhs.encoded_value;
    }

    // returns negative number if lhs < rhs,
    // returns positive number if lhs > rhs
    // returns 0 if lhs == rhs
    static int compare(const GraphObject& lhs, const GraphObject& rhs) {
        if (lhs.type == rhs.type && lhs.encoded_value == rhs.encoded_value)
            return 0;

        bool lhs_is_string = false;
        bool rhs_is_string = false;

        bool lhs_is_numeric = false;
        bool rhs_is_numeric = false;

        std::unique_ptr<CharIter> lhs_string_iter;
        std::unique_ptr<CharIter> rhs_string_iter;

        double lhs_numeric;
        double rhs_numeric;

        switch (lhs.type) {
        case GraphObjectType::STR_INLINED:
        case GraphObjectType::NAMED_INLINED:
            lhs_string_iter = std::make_unique<StringInlineIter>(lhs.encoded_value);
            lhs_is_string   = true;
            break;
        case GraphObjectType::STR_EXTERNAL:
        case GraphObjectType::NAMED_EXTERNAL:
            lhs_string_iter = string_manager.get_char_iter(GraphObjectInterpreter::get<StringExternal>(lhs).external_id);
            lhs_is_string = true;
            break;
        case GraphObjectType::NAMED_TMP:
        case GraphObjectType::STR_TMP:
            lhs_string_iter = std::make_unique<StringTmpIter>(*GraphObjectInterpreter::get<StringTmp>(lhs).str);
            lhs_is_string   = true;
            break;
        case GraphObjectType::INT:
            lhs_is_numeric = true;
            lhs_numeric    = GraphObjectInterpreter::get<int64_t>(lhs);
            break;
        case GraphObjectType::FLOAT:
            lhs_is_numeric = true;
            lhs_numeric    = GraphObjectInterpreter::get<float>(lhs);
            break;
        // case GraphObjectType::EDGE:
        // case GraphObjectType::ANON:
        // case GraphObjectType::NULL_OBJ:
        // case GraphObjectType::NOT_FOUND:
        // case GraphObjectType::BOOL:
        // case GraphObjectType::PATH:
        default:
            break;
        }

        switch (rhs.type) {
        case GraphObjectType::STR_INLINED:
        case GraphObjectType::NAMED_INLINED:
            rhs_string_iter = std::make_unique<StringInlineIter>(rhs.encoded_value);
            rhs_is_string   = true;
            break;
        case GraphObjectType::STR_EXTERNAL:
        case GraphObjectType::NAMED_EXTERNAL:
            rhs_string_iter = string_manager.get_char_iter(GraphObjectInterpreter::get<StringExternal>(rhs).external_id);
            rhs_is_string = true;
            break;
        case GraphObjectType::NAMED_TMP:
        case GraphObjectType::STR_TMP:
            rhs_string_iter = std::make_unique<StringTmpIter>(*GraphObjectInterpreter::get<StringTmp>(rhs).str);
            rhs_is_string   = true;
            break;
        case GraphObjectType::INT:
            rhs_is_numeric = true;
            rhs_numeric    = GraphObjectInterpreter::get<int64_t>(rhs);
            break;
        case GraphObjectType::FLOAT:
            rhs_is_numeric = true;
            rhs_numeric    = GraphObjectInterpreter::get<float>(rhs);
            break;
        // case GraphObjectType::EDGE:
        // case GraphObjectType::ANON:
        // case GraphObjectType::NULL_OBJ:
        // case GraphObjectType::NOT_FOUND:
        // case GraphObjectType::BOOL:
        // case GraphObjectType::PATH:
        default:
            break;
        }

        if (lhs_is_string && rhs_is_string) {
            return StringManager::compare(*lhs_string_iter, *rhs_string_iter);
        } else if (lhs_is_numeric && rhs_is_numeric) {
            if (lhs_numeric < rhs_numeric)
                return -1;
            else if (lhs_numeric > rhs_numeric)
                return 1;
            else
                return 0;
        } else if (rhs.type == lhs.type) {
            switch (lhs.type) {
            // case GraphObjectType::NAMED_INLINED:
            // case GraphObjectType::NAMED_EXTERNAL:
            // case GraphObjectType::NAMED_TMP:
            // case GraphObjectType::STR_INLINED:
            // case GraphObjectType::STR_EXTERNAL:
            // case GraphObjectType::STR_TMP:
            // case GraphObjectType::INT:
            // case GraphObjectType::FLOAT:
            case GraphObjectType::NULL_OBJ:
                return 0;
            case GraphObjectType::NOT_FOUND:
                return 0;
            case GraphObjectType::EDGE:
                return GraphObjectInterpreter::get<Edge>(lhs).id - GraphObjectInterpreter::get<Edge>(lhs).id;
            case GraphObjectType::ANON:
                return GraphObjectInterpreter::get<AnonymousNode>(lhs).id
                     - GraphObjectInterpreter::get<AnonymousNode>(rhs).id;
            case GraphObjectType::BOOL:
                return GraphObjectInterpreter::get<bool>(lhs) - GraphObjectInterpreter::get<bool>(rhs);
            case GraphObjectType::PATH:
                return GraphObjectInterpreter::get<Path>(lhs).path_id - GraphObjectInterpreter::get<Path>(rhs).path_id;
            default:
                throw LogicException("Unmanaged case compare");
            }
        } else {
            return (lhs.type < rhs.type) ? -1 : 1;
        }
    }

    // returns negative number if lhs < rhs,
    // returns positive number if lhs > rhs
    // returns 0 if lhs == rhs
    static int compare_rdf(const GraphObject& lhs, const GraphObject& rhs) {
        if (lhs.type == rhs.type && lhs.encoded_value == rhs.encoded_value)
            return 0;

        auto lhs_complex_type = SPARQL_COMPLEX_TYPES::NONE;
        auto rhs_complex_type = SPARQL_COMPLEX_TYPES::NONE;

        std::unique_ptr<CharIter> lhs_iter;
        std::unique_ptr<CharIter> rhs_iter;

        std::string lhs_decimal_string;
        std::string rhs_decimal_string;

        switch(lhs.type) {
            case GraphObjectType::STR_INLINED: {
                lhs_iter = std::make_unique<StringInlineIter>(lhs.encoded_value);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::STRING;
                break;
            }
            case GraphObjectType::STR_EXTERNAL: {
                lhs_iter = string_manager.get_char_iter(GraphObjectInterpreter::get<StringExternal>(lhs).external_id);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::STRING;
                break;
            }
            case GraphObjectType::STR_TMP: {
                lhs_iter = std::make_unique<StringTmpIter>(*GraphObjectInterpreter::get<StringTmp>(lhs).str);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::STRING;
                break;
            }
            case GraphObjectType::IRI_INLINED: {
                const auto& iri_inl = GraphObjectInterpreter::get<IriInlined>(lhs);

                auto& prefix = rdf_model.catalog().prefixes[iri_inl.prefix_id];

                lhs_iter = std::make_unique<IriInlineIter>(prefix, iri_inl.id);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::IRI;
                break;
            }
            case GraphObjectType::IRI_EXTERNAL: {
                uint64_t external_id = GraphObjectInterpreter::get<IriExternal>(lhs).external_id;
                uint64_t iri_id      = external_id & 0x0000'FFFF'FFFF'FFFFUL;
                uint8_t  prefix_id   = (external_id & 0x00FF'0000'0000'0000UL) >> 48;

                auto& prefix = rdf_model.catalog().prefixes[prefix_id];

                lhs_iter = std::make_unique<IriExternalIter>(prefix, iri_id);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::IRI;
                break;
            }
            case GraphObjectType::IRI_TMP: {
                lhs_iter = std::make_unique<StringTmpIter>(*GraphObjectInterpreter::get<IriTmp>(lhs).str);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::IRI;
                break;
            }
            case GraphObjectType::LITERAL_DATATYPE_INLINED: {
                const auto& lit_dt_inl = GraphObjectInterpreter::get<LiteralDatatypeInlined>(lhs);

                auto& datatype = rdf_model.catalog().datatypes[lit_dt_inl.datatype_id];

                lhs_iter = std::make_unique<LiteralDatatypeInlineIter>(lit_dt_inl.id, datatype);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_DATATYPE;
                break;
            }
            case GraphObjectType::LITERAL_DATATYPE_EXTERNAL: {
                uint64_t external_id = GraphObjectInterpreter::get<LiteralDatatypeExternal>(lhs).external_id;
                uint64_t literal_id  = external_id & 0x0000'00FF'FFFF'FFFFUL;
                uint16_t datatype_id = (external_id & 0x00FF'FF00'0000'0000UL) >> 40;

                auto& datatype = rdf_model.catalog().datatypes[datatype_id];

                lhs_iter = std::make_unique<LiteralDatatypeExternalIter>(literal_id, datatype);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_DATATYPE;
                break;
            }
            case GraphObjectType::LITERAL_DATATYPE_TMP: {
                auto ld = GraphObjectInterpreter::get<LiteralDatatypeTmp>(lhs).ld;

                lhs_iter = std::make_unique<StringTmpIter>(ld->str + ld->datatype);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_DATATYPE;
                break;
            }
            case GraphObjectType::LITERAL_LANGUAGE_INLINED: {
                const auto& lit_lang_inl = GraphObjectInterpreter::get<LiteralLanguageInlined>(lhs);

                auto& language = rdf_model.catalog().languages[lit_lang_inl.language_id];

                lhs_iter = std::make_unique<LiteralLanguageInlineIter>(lit_lang_inl.id, language);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_LANGUAGE;
                break;
            }
            case GraphObjectType::LITERAL_LANGUAGE_EXTERNAL: {
                uint64_t external_id = GraphObjectInterpreter::get<LiteralLanguageExternal>(lhs).external_id;
                uint64_t literal_id  = external_id & 0x0000'00FF'FFFF'FFFFUL;
                uint16_t language_id = (external_id & 0x00FF'FF00'0000'0000UL) >> 40;

                auto& language = rdf_model.catalog().languages[language_id];

                lhs_iter = std::make_unique<LiteralLanguageExternalIter>(literal_id, language);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_LANGUAGE;
                break;
            }
            case GraphObjectType::LITERAL_LANGUAGE_TMP: {
                auto ll = GraphObjectInterpreter::get<LiteralLanguageTmp>(lhs).ll;

                lhs_iter = std::make_unique<StringTmpIter>(ll->str + ll->language);
                lhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_LANGUAGE;
                break;
            }
            case GraphObjectType::DECIMAL_INLINED: {
                lhs_decimal_string = GraphObjectInterpreter::get<DecimalInlined>(lhs).get_value_string();
                lhs_complex_type = SPARQL_COMPLEX_TYPES::DECIMAL;
                break;
            }
            case GraphObjectType::DECIMAL_EXTERNAL: {
                std::stringstream ss;
                string_manager.print(ss, GraphObjectInterpreter::get<DecimalExternal>(lhs).external_id);
                lhs_decimal_string = ss.str();
                lhs_complex_type = SPARQL_COMPLEX_TYPES::DECIMAL;
                break;
            }
            case GraphObjectType::DECIMAL_TMP: {
                lhs_decimal_string = *GraphObjectInterpreter::get<DecimalTmp>(lhs).str;
                lhs_complex_type = SPARQL_COMPLEX_TYPES::DECIMAL;
                break;
            }
            default:
                break;
        }

        switch(rhs.type) {
            case GraphObjectType::STR_INLINED: {
                rhs_iter = std::make_unique<StringInlineIter>(rhs.encoded_value);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::STRING;
                break;
            }
            case GraphObjectType::STR_EXTERNAL: {
                rhs_iter = string_manager.get_char_iter(GraphObjectInterpreter::get<StringExternal>(rhs).external_id);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::STRING;
                break;
            }
            case GraphObjectType::STR_TMP: {
                rhs_iter = std::make_unique<StringTmpIter>(*GraphObjectInterpreter::get<StringTmp>(rhs).str);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::STRING;
                break;
            }
            case GraphObjectType::IRI_INLINED: {
                const auto& iri_inl = GraphObjectInterpreter::get<IriInlined>(rhs);

                auto& prefix = rdf_model.catalog().prefixes[iri_inl.prefix_id];

                rhs_iter = std::make_unique<IriInlineIter>(prefix, iri_inl.id);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::IRI;
                break;
            }
            case GraphObjectType::IRI_EXTERNAL: {
                uint64_t external_id = GraphObjectInterpreter::get<IriExternal>(rhs).external_id;
                uint64_t iri_id      = external_id & 0x0000'FFFF'FFFF'FFFFUL;
                uint8_t  prefix_id   = (external_id & 0x00FF'0000'0000'0000UL) >> 48;

                auto& prefix = rdf_model.catalog().prefixes[prefix_id];

                rhs_iter = std::make_unique<IriExternalIter>(prefix, iri_id);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::IRI;
                break;
            }
            case GraphObjectType::IRI_TMP: {
                rhs_iter = std::make_unique<StringTmpIter>(*GraphObjectInterpreter::get<IriTmp>(rhs).str);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::IRI;
                break;
            }
            case GraphObjectType::LITERAL_DATATYPE_INLINED: {
                const auto& lit_dt_inl = GraphObjectInterpreter::get<LiteralDatatypeInlined>(rhs);

                auto& datatype = rdf_model.catalog().datatypes[lit_dt_inl.datatype_id];

                rhs_iter = std::make_unique<LiteralDatatypeInlineIter>(lit_dt_inl.id, datatype);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_DATATYPE;
                break;
            }
            case GraphObjectType::LITERAL_DATATYPE_EXTERNAL: {
                uint64_t external_id = GraphObjectInterpreter::get<LiteralDatatypeExternal>(rhs).external_id;
                uint64_t literal_id  = external_id & 0x0000'00FF'FFFF'FFFFUL;
                uint16_t datatype_id = (external_id & 0x00FF'FF00'0000'0000UL) >> 40;

                auto& datatype = rdf_model.catalog().datatypes[datatype_id];

                rhs_iter = std::make_unique<LiteralDatatypeExternalIter>(literal_id, datatype);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_DATATYPE;
                break;
            }
            case GraphObjectType::LITERAL_DATATYPE_TMP: {
                auto ld = GraphObjectInterpreter::get<LiteralDatatypeTmp>(rhs).ld;

                rhs_iter = std::make_unique<StringTmpIter>(ld->str + ld->datatype);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_DATATYPE;
                break;
            }
            case GraphObjectType::LITERAL_LANGUAGE_INLINED: {
                const auto& lit_lang_inl = GraphObjectInterpreter::get<LiteralLanguageInlined>(rhs);

                auto& language = rdf_model.catalog().languages[lit_lang_inl.language_id];

                rhs_iter = std::make_unique<LiteralLanguageInlineIter>(lit_lang_inl.id, language);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_LANGUAGE;
                break;
            }
            case GraphObjectType::LITERAL_LANGUAGE_EXTERNAL: {
                uint64_t external_id = GraphObjectInterpreter::get<LiteralLanguageExternal>(rhs).external_id;
                uint64_t literal_id  = external_id & 0x0000'00FF'FFFF'FFFFUL;
                uint16_t language_id = (external_id & 0x00FF'FF00'0000'0000UL) >> 40;

                auto& language = rdf_model.catalog().languages[language_id];

                rhs_iter = std::make_unique<LiteralLanguageExternalIter>(literal_id, language);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_LANGUAGE;
                break;
            }
            case GraphObjectType::LITERAL_LANGUAGE_TMP: {
                auto ll = GraphObjectInterpreter::get<LiteralLanguageTmp>(rhs).ll;

                rhs_iter = std::make_unique<StringTmpIter>(ll->str + ll->language);
                rhs_complex_type = SPARQL_COMPLEX_TYPES::LITERAL_LANGUAGE;
                break;
            }
            case GraphObjectType::DECIMAL_INLINED: {
                rhs_decimal_string = GraphObjectInterpreter::get<DecimalInlined>(rhs).get_value_string();
                rhs_complex_type = SPARQL_COMPLEX_TYPES::DECIMAL;
                break;
            }
            case GraphObjectType::DECIMAL_EXTERNAL: {
                std::stringstream ss;
                string_manager.print(ss, GraphObjectInterpreter::get<DecimalExternal>(rhs).external_id);
                rhs_decimal_string = ss.str();
                rhs_complex_type = SPARQL_COMPLEX_TYPES::DECIMAL;
                break;
            }
            case GraphObjectType::DECIMAL_TMP: {
                rhs_decimal_string = *GraphObjectInterpreter::get<DecimalTmp>(rhs).str;
                rhs_complex_type = SPARQL_COMPLEX_TYPES::DECIMAL;
                break;
            }
            default:
                break;
        }

        if (lhs_complex_type == rhs_complex_type) {
            switch (lhs_complex_type) {
            case SPARQL_COMPLEX_TYPES::STRING:
            case SPARQL_COMPLEX_TYPES::IRI:
            case SPARQL_COMPLEX_TYPES::LITERAL_DATATYPE:
            case SPARQL_COMPLEX_TYPES::LITERAL_LANGUAGE:
                return StringManager::compare(*lhs_iter, *rhs_iter);
            case SPARQL_COMPLEX_TYPES::DECIMAL: {
                // NOTE: It is assumed that the format of both strings follows the following pattern:
                // (-)?(0|[1-9][0-9]*).(0|[0-9]*[1-9])

                // 1. Compare signs
                bool lhs_neg = lhs_decimal_string[0] == '-';
                bool rhs_neg = rhs_decimal_string[0] == '-';
                int sign_diff = lhs_neg - rhs_neg;
                if (sign_diff != 0) {
                    return sign_diff > 0 ? -1 : 1;
                }
                // 2. Compare integer part
                size_t lhs_sep = lhs_decimal_string.find('.');
                size_t rhs_sep = rhs_decimal_string.find('.');
                // Compare integer part length
                int intlen_diff = lhs_sep - rhs_sep;
                if (intlen_diff != 0) {
                    return lhs_neg ? -intlen_diff : intlen_diff;
                }
                // NOTE: lhs_sep == rhs_sep at this point
                // Compare integer part digits
                int int_diff = strncmp(lhs_decimal_string.c_str(), rhs_decimal_string.c_str(), lhs_sep);
                if (int_diff != 0) {
                    return lhs_neg ? -int_diff : int_diff;
                }
                // 3. Compare fractional part
                // Compare fractional part digits
                int frac_diff = strcmp(lhs_decimal_string.c_str() + lhs_sep, rhs_decimal_string.c_str() + lhs_sep);
                if (frac_diff != 0) {
                    return lhs_neg ? -frac_diff : frac_diff;
                }
                return 0;
            }
            default:
                break;
            }
        }
        if (lhs.type == rhs.type) {
            switch (lhs.type) {
            case GraphObjectType::NULL_OBJ:
                return 0;
            case GraphObjectType::NOT_FOUND:
                return 0;
            case GraphObjectType::ANON:
                return GraphObjectInterpreter::get<AnonymousNode>(lhs).id
                     - GraphObjectInterpreter::get<AnonymousNode>(rhs).id;
            case GraphObjectType::BOOL:
                return GraphObjectInterpreter::get<bool>(lhs) - GraphObjectInterpreter::get<bool>(rhs);
            case GraphObjectType::DATETIME: {
                const auto& lhs_id = GraphObjectInterpreter::get<DateTime>(lhs).id;
                const auto& rhs_id = GraphObjectInterpreter::get<DateTime>(rhs).id;

                // Check sign bit
                uint64_t sign_mask = 1ULL << 55;
                int64_t  sign_diff = (lhs_id & sign_mask) - (rhs_id & sign_mask);
                if (sign_diff != 0) {
                    return sign_diff > 0 ? -1 : 1;
                }
                // From now on, both numbers have the same sign
                int64_t diff = 0;
                // Check precision bit
                uint64_t precision_mask = 1ULL << 54;
                int64_t  precision_diff = (lhs_id & precision_mask) - (rhs_id & precision_mask);
                if (precision_diff != 0) {
                    diff = precision_diff > 0 ? 1 : -1;
                }
                // Handle same precision
                else {
                    uint64_t datetime_mask = 0x00'3F'FFFF'FFFF'FFFF;
                    int64_t  datetime_diff = (lhs_id & datetime_mask) - (rhs_id & datetime_mask);
                    diff = datetime_diff > 0 ? 1 : -1;
                }
                // Flip if the result if both signs were negative
                if (lhs_id & sign_mask) {
                    return -diff;
                } else {
                    return diff;
                }
            }
            default:
                throw LogicException("Unmanaged case compare_rdf");
            }
        }
        else {
            return (lhs.type < rhs.type) ? -1 : 1;
        }
    }

    static GraphObject sum(const GraphObject& lhs, const GraphObject& rhs) {
        if (lhs.type == GraphObjectType::INT) {
            if (rhs.type == GraphObjectType::INT) {
                return GraphObjectFactory::make_int(GraphObjectInterpreter::get<int64_t>(lhs)
                                                    + GraphObjectInterpreter::get<int64_t>(rhs));
            } else if (rhs.type == GraphObjectType::FLOAT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<int64_t>(lhs)
                                                      + GraphObjectInterpreter::get<float>(rhs));
            }
        } else if (lhs.type == GraphObjectType::FLOAT) {
            if (rhs.type == GraphObjectType::INT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<float>(lhs)
                                                      + GraphObjectInterpreter::get<int64_t>(rhs));
            } else if (rhs.type == GraphObjectType::FLOAT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<float>(lhs)
                                                      + GraphObjectInterpreter::get<float>(rhs));
            }
        }
        return GraphObjectFactory::make_null();
    }

    static GraphObject minus(const GraphObject& lhs, const GraphObject& rhs) {
        if (lhs.type == GraphObjectType::INT) {
            if (rhs.type == GraphObjectType::INT) {
                return GraphObjectFactory::make_int(GraphObjectInterpreter::get<int64_t>(lhs)
                                                    - GraphObjectInterpreter::get<int64_t>(rhs));
            } else if (rhs.type == GraphObjectType::FLOAT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<int64_t>(lhs)
                                                      - GraphObjectInterpreter::get<float>(rhs));
            }
        } else if (lhs.type == GraphObjectType::FLOAT) {
            if (rhs.type == GraphObjectType::INT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<float>(lhs)
                                                      - GraphObjectInterpreter::get<int64_t>(rhs));
            } else if (rhs.type == GraphObjectType::FLOAT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<float>(lhs)
                                                      - GraphObjectInterpreter::get<float>(rhs));
            }
        }
        return GraphObjectFactory::make_null();
    }

    static GraphObject multiply(const GraphObject& lhs, const GraphObject& rhs) {
        if (lhs.type == GraphObjectType::INT) {
            if (rhs.type == GraphObjectType::INT) {
                return GraphObjectFactory::make_int(GraphObjectInterpreter::get<int64_t>(lhs)
                                                    * GraphObjectInterpreter::get<int64_t>(rhs));
            } else if (rhs.type == GraphObjectType::FLOAT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<int64_t>(lhs)
                                                      * GraphObjectInterpreter::get<float>(rhs));
            }
        } else if (lhs.type == GraphObjectType::FLOAT) {
            if (rhs.type == GraphObjectType::INT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<float>(lhs)
                                                      * GraphObjectInterpreter::get<int64_t>(rhs));
            } else if (rhs.type == GraphObjectType::FLOAT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<float>(lhs)
                                                      * GraphObjectInterpreter::get<float>(rhs));
            }
        }
        return GraphObjectFactory::make_null();
    }

    static GraphObject divide(const GraphObject& lhs, const GraphObject& rhs) {
        if (rhs.type == GraphObjectType::INT) {
            // avoid division by 0
            if (GraphObjectInterpreter::get<int64_t>(rhs) == 0)
                return GraphObjectFactory::make_null();
            if (lhs.type == GraphObjectType::INT) {
                return GraphObjectFactory::make_int(GraphObjectInterpreter::get<int64_t>(lhs)
                                                    / GraphObjectInterpreter::get<int64_t>(rhs));
            } else if (lhs.type == GraphObjectType::FLOAT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<float>(lhs)
                                                      / GraphObjectInterpreter::get<int64_t>(rhs));
            }
        } else if (rhs.type == GraphObjectType::FLOAT) {
            // avoid division by 0
            if (GraphObjectInterpreter::get<float>(rhs) == 0.0f)
                return GraphObjectFactory::make_null();
            if (lhs.type == GraphObjectType::INT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<int64_t>(lhs)
                                                      / GraphObjectInterpreter::get<float>(rhs));
            } else if (lhs.type == GraphObjectType::FLOAT) {
                return GraphObjectFactory::make_float(GraphObjectInterpreter::get<float>(lhs)
                                                      / GraphObjectInterpreter::get<float>(rhs));
            }
        }
        return GraphObjectFactory::make_null();
    }

    static GraphObject modulo(const GraphObject& lhs, const GraphObject& rhs) {
        if (lhs.type == GraphObjectType::INT && rhs.type == GraphObjectType::INT
            && GraphObjectInterpreter::get<int64_t>(rhs) != 0)
        {
            return GraphObjectFactory::make_int(GraphObjectInterpreter::get<int64_t>(lhs)
                                                % GraphObjectInterpreter::get<int64_t>(rhs));
        }
        return GraphObjectFactory::make_null();
    }

    static void print_datatype_rdf(std::ostream& os, uint64_t datatype_id){
        uint64_t tmp_mask = 0x8000; // 1000 0000 0000 0000
        os << "\"^^<";
        if ((datatype_id & tmp_mask) == 0) os << rdf_model.catalog().datatypes[datatype_id];
        else temporal_manager.print_dtt(os, (datatype_id & 0x7FFF)); // 0111 1111 1111 1111
        os << '>';
    }

    static void print_language_rdf(std::ostream& os, uint64_t language_id){
        uint64_t tmp_mask = 0x8000; // 1000 0000 0000 0000
        os << "\"@";
        if ((language_id & tmp_mask) == 0) os << rdf_model.catalog().languages[language_id];
        else temporal_manager.print_lan(os, (language_id & 0x7FFF)); // 0111 1111 1111 1111
    }
};
