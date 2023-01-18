#include "response_parser.h"

#include <string>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/range/combine.hpp>

#include "query_optimizer/rdf_model/rdf_model.h" // TODO moverlo
#include "storage/string_manager.h" // TODO moverlo
#include "storage/temporal_manager.h" // TODO moverlo

Parser::Parser(Format format) : format (format) {}

void Parser::begin(std::string response) {
    switch(format){
        case Format::json:{
            boost::property_tree::ptree pt;
            std::stringstream ss{response};
            boost::property_tree::read_json(ss, pt);
            results = pt.get_child("results.bindings");
            result = results.begin();
            break;
        }
        case Format::xml:{
            boost::property_tree::ptree pt;
            std::stringstream ss{response};
            boost::property_tree::read_xml(ss, pt);
            results = pt.get_child("sparql.results");
            result = results.begin();
            break;
        }
        case Format::csv:
        case Format::tsv:{
            response_token = response; // need to save the response, tokenizer stores a pointer
            char separator = (format==Format::csv) ? ',' : '\t';
            token_sep = std::make_unique<boost::escaped_list_separator<char>>('\0', separator, '\"');
            boost::escaped_list_separator<char> line_sep('\0', '\n', '\"');
            line_tok = std::make_unique<boost::tokenizer<boost::escaped_list_separator<char>>>(response_token, line_sep);
            line = line_tok->begin();
            std::string strp_line = line.current_token();
            if (!strp_line.empty() && strp_line[strp_line.size()-1]=='\r') strp_line.erase(strp_line.size()-1, 1);
            if (!strp_line.empty() && strp_line[0]=='\r') strp_line.erase(0, 1);
            header_line = strp_line;
            ++line;
            break;
        }
    }
}


bool Parser::next(std::map<std::string, ObjectId>& binding) {
    binding.clear();
    switch(format){
        case Format::json:{
            if (result == results.end()) return false;
            for (auto &attr: result->second.get_child("")){
                std::string attr_type = attr.second.get<std::string>("type");
                std::string attr_value = attr.second.get<std::string>("value");
                std::string extra_data = "";
                if (attr_type == "literal" && attr.second.count("datatype")) {
                    extra_data = attr.second.get<std::string>("datatype");
                    attr_type = "datatype";
                } else if (attr_type == "literal" && attr.second.count("xml:lang")) {
                    extra_data = attr.second.get<std::string>("xml:lang");
                    attr_type = "lang";
                }
                ObjectId object_id;
                get_object_id(object_id, std::move(attr_type), std::move(attr_value), std::move(extra_data));
                binding.insert({attr.first, object_id});
            }
            ++result;
            return true;
        }
        case Format::xml:{
            if (result == results.end()) return false;
            if (result->first =="<xmlattr>") ++result;
            if (result == results.end()) return false;
            for (auto &attr: result->second.get_child("")){
                if (attr.first != "<xmlattr>"){
                    std::string attr_name = attr.second.get<std::string>("<xmlattr>.name");
                    std::string attr_type, attr_value;
                    std::string extra_data = "";
                    if (attr.second.count("literal")) {
                        attr_type = "literal";
                        auto var = attr.second.get_child("literal");
                        if (var.count("<xmlattr>") && var.get_child("<xmlattr>").count("datatype")) {
                            extra_data = var.get<std::string>("<xmlattr>.datatype");
                            attr_type = "datatype";
                        } else if (var.count("<xmlattr>") && var.get_child("<xmlattr>").count("xml:lang")) {
                            extra_data = var.get<std::string>("<xmlattr>.xml:lang");
                            attr_type = "lang";
                        }
                        attr_value = var.get<std::string>("");
                    } else if (attr.second.count("uri")) {
                        attr_type = "uri";
                        attr_value = attr.second.get<std::string>("uri");
                    } else {
                        attr_type = "literal";
                        attr_value = attr.second.get<std::string>("bnode");
                    }
                    ObjectId object_id;
                    get_object_id(object_id, std::move(attr_type), std::move(attr_value), std::move(extra_data));
                    binding.insert({attr_name, object_id});
                }
            }
            ++result;
            return true;
        }
        case Format::csv:
        case Format::tsv:{
            auto next_line = line;
            if (++next_line == line_tok->end()) return false;

            std::string strp_line = line.current_token();
            if (!strp_line.empty() && strp_line[strp_line.size()-1]=='\r') strp_line.erase(strp_line.size()-1, 1);
            if (!strp_line.empty() && strp_line[0]=='\r') strp_line.erase(0, 1);

            boost::tokenizer<boost::escaped_list_separator<char>> header_tok(header_line, *token_sep), token_tok(strp_line, *token_sep);
            for (auto const &attr: boost::combine(header_tok, token_tok)){
                std::string attr_name = attr.get_head(), attr_value = attr.get_tail().get_head();
                if (!attr_name.empty() && !attr_value.empty()) {
                    std::string extra_data = "", attr_type;
                    if (attr_name[0]=='?') attr_name.erase(0, 1);
                    if (attr_value[0]=='<' && attr_value[attr_value.size()-1]=='>') {
                        attr_type = "uri";
                        attr_value = attr_value.substr(1, attr_value.size()-2);
                    } else {
                        auto at = attr_value.rfind("@"); // reverse find, languages wont have @
                        if (at != std::string::npos) {
                            attr_type = "lang";
                            extra_data = attr_value.substr(at + 1);
                            attr_value = attr_value.substr(0, at);
                        } else {
                            auto dash = attr_value.rfind("^^");
                            if (dash != std::string::npos) {
                                attr_type = "datatype";
                                extra_data = attr_value.substr(dash + 3, attr_value.size() - dash - 4);
                                attr_value = attr_value.substr(0, dash);
                            }
                            else attr_type = "literal";
                        }
                    }
                    ObjectId object_id;
                    get_object_id(object_id, std::move(attr_type), std::move(attr_value), std::move(extra_data));
                    binding.insert({attr_name, object_id});
                }
            }
            ++line;
            return true;
        }
    }
    return false;
}

uint64_t Parser::get_inline_id(std::string& value, std::uint8_t size){
    uint64_t inline_id = 0;
    int shift_size = 8 * size;
    for (uint8_t byte : value) {
        uint64_t byte64 = static_cast<uint64_t>(byte);
        inline_id |= byte64 << shift_size;
        shift_size -= 8;
    }
    return inline_id;
}

uint8_t Parser::get_prefix_id(std::string& value){
    uint8_t prefix_id = 0;
    for(auto &prefix: rdf_model.catalog().prefixes){
        if (value.compare(0, prefix.size(), prefix) == 0) {
            value = value.substr(prefix.size(), value.size() - prefix.size());
            break;
        }
        ++prefix_id;
    }
    return prefix_id;
}

uint64_t Parser::get_datatype_id(std::string& value){
    uint64_t datatype_id = 0;
    bool found = false;
    for (auto &datatype: rdf_model.catalog().datatypes) {
        if (value == datatype) {
            found = true;
            break;
        }
        ++datatype_id;
    }
    if (found) return datatype_id << 40;
    datatype_id = temporal_manager.get_dtt_id(value);
    uint64_t tmp_mask = 0x8000;  // 1000 0000 0000 0000
    return (tmp_mask | datatype_id) << 40;
}

uint64_t Parser::get_language_id(std::string& value){
    uint64_t language_id = 0;
    bool found = false;
    for (auto &language: rdf_model.catalog().languages) {
        if (value == language) {
            found = true;
            break;
        }
        ++language_id;
    }
    if (found) return language_id << 40;
    language_id = temporal_manager.get_lan_id(value);
    uint64_t tmp_mask = 0x8000;  // 1000 0000 0000 0000
    return (tmp_mask | language_id) << 40;
}

void Parser::get_object_id(ObjectId& object_id, std::string attr_type, std::string attr_value, std::string extra_data){
    if (attr_type == "uri") {
        uint8_t prefix_id = get_prefix_id(attr_value);  // attr_value gets sliced
        uint64_t shifted_prefix_id = static_cast<uint64_t>(prefix_id) << 48;
        if (attr_value.size() < 7) {
            uint64_t inline_id = get_inline_id(attr_value, 5);
            object_id = ObjectId(ObjectId::MASK_IRI_INLINED | shifted_prefix_id | inline_id);
        } else {
            uint64_t external_id = string_manager.get_str_id(attr_value, false);
            if (external_id != ObjectId::OBJECT_ID_NOT_FOUND) {
                object_id = ObjectId(ObjectId::MASK_IRI_EXTERN | shifted_prefix_id | external_id);
            } else {
                uint64_t temporal_id = temporal_manager.get_str_id(attr_value);
                object_id = ObjectId(ObjectId::MASK_IRI_TMP | shifted_prefix_id | temporal_id);
            }
        }
    } else if (attr_type == "literal") {
        if (attr_value.size() < 8) {
            uint64_t inline_id = get_inline_id(attr_value, 6);
            object_id = ObjectId(ObjectId::MASK_STRING_INLINED | inline_id);
        } else {
            uint64_t external_id = string_manager.get_str_id(attr_value, false);
            if (external_id != ObjectId::OBJECT_ID_NOT_FOUND) {
                object_id = ObjectId(ObjectId::MASK_STRING_EXTERN | external_id);
            } else {
                uint64_t temporal_id = temporal_manager.get_str_id(attr_value);
                object_id = ObjectId(ObjectId::MASK_STRING_TMP2 | temporal_id);
            }
        }
    } else if (attr_type == "datatype") {
        uint64_t datatype_id = get_datatype_id(extra_data);
        if (attr_value.size() < 6) {
            uint64_t inline_id = get_inline_id(attr_value, 4);
            object_id = ObjectId(ObjectId::MASK_STRING_DATATYPE_INLINED | datatype_id | inline_id);
        } else {
            uint64_t external_id = string_manager.get_str_id(attr_value, false);
            if (external_id != ObjectId::OBJECT_ID_NOT_FOUND) {
                object_id = ObjectId(ObjectId::MASK_STRING_DATATYPE_EXTERN | datatype_id | external_id);
            } else {
                uint64_t temporal_id = temporal_manager.get_str_id(attr_value);
                object_id = ObjectId(ObjectId::MASK_STRING_DATATYPE_TMP2 | datatype_id | temporal_id);
            }
        }
    } else if (attr_type == "lang") {
        uint64_t language_id = get_language_id(extra_data);
        if (attr_value.size() < 6) {
            uint64_t inline_id = get_inline_id(attr_value, 4);
            object_id = ObjectId(ObjectId::MASK_STRING_LANG_INLINED | language_id | inline_id);
        } else {
            uint64_t external_id = string_manager.get_str_id(attr_value, false);
            if (external_id != ObjectId::OBJECT_ID_NOT_FOUND) {
                object_id = ObjectId(ObjectId::MASK_STRING_LANG_EXTERN | language_id | external_id);
            } else {
                uint64_t temporal_id = temporal_manager.get_str_id(attr_value);
                object_id = ObjectId(ObjectId::MASK_STRING_LANG_TMP2 | language_id | temporal_id);
            }
        }
    }
}


void Parser::reset() {
    switch(format){
        case Format::json:
        case Format::xml:{
            result = results.begin();
            break;
        }
        case Format::csv:
        case Format::tsv:{
            line = line_tok->begin();
        }
    }
}