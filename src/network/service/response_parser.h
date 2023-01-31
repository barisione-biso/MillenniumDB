#pragma once

#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/tokenizer.hpp>

#include "base/ids/object_id.h"

enum class Format {json, xml, csv, tsv};

class Parser {
public:
    Parser(Format format);

    void begin(std::string response);

    bool next(std::map<std::string, ObjectId>& binding);

    void reset();

    uint64_t get_inline_id(std::string& value, std::uint8_t size);

    uint8_t get_prefix_id(std::string& value);
    uint64_t get_datatype_id(std::string& value);
    uint64_t get_language_id(std::string& value);

    void get_object_id(ObjectId& object_id, std::string attr_type, std::string attr_value, std::string extra_data);

private:
    Format format;

    boost::property_tree::ptree results;
    boost::property_tree::ptree::iterator result;

    std::string response_token;
    std::unique_ptr<boost::tokenizer<boost::escaped_list_separator<char>>> line_tok;
    boost::tokenizer<boost::escaped_list_separator<char>>::iterator line;
    std::unique_ptr<boost::escaped_list_separator<char>> token_sep;
    std::string header_line;
};
