#pragma once

#include <ostream>
#include <type_traits>
#include <iostream>

enum class GraphObjectType;

class GraphObject {
friend class GraphObjectFactory;
public:
    uint64_t encoded_value;

    GraphObjectType type;

    static int (*graph_object_cmp)(const GraphObject&, const GraphObject&);
    static bool (*graph_object_eq)(const GraphObject&, const GraphObject&);
    static std::ostream& (*graph_object_print)(std::ostream&, const GraphObject&);
    static GraphObject (*graph_object_sum)     (const GraphObject&, const GraphObject&);
    static GraphObject (*graph_object_minus)   (const GraphObject&, const GraphObject&);
    static GraphObject (*graph_object_multiply)(const GraphObject&, const GraphObject&);
    static GraphObject (*graph_object_divide)  (const GraphObject&, const GraphObject&);
    static GraphObject (*graph_object_modulo)  (const GraphObject&, const GraphObject&);

    GraphObject() : encoded_value(0), type(GraphObjectType(0)) { }

    static GraphObject make_iri_external(uint64_t external_id) {
        return GraphObject(GraphObjectUnion(IriExternal(external_id)), GraphObjectType::IRI_EXTERNAL);
    }

    // must receive an array of size 7, terminating in '\0' and an 1 byte prefix_id
    static GraphObject make_iri_inlined(const char* str, uint8_t prefix_id) {
        return GraphObject(GraphObjectUnion(IriInlined(str, prefix_id)), GraphObjectType::IRI_INLINED);
    }

    static GraphObject make_iri_tmp(const std::string& str) {
        return GraphObject(GraphObjectUnion(IriTmp(str)), GraphObjectType::IRI_TMP);
    }

    static GraphObject make_literal_datatype_external(uint64_t external_id) {
        return GraphObject(GraphObjectUnion(LiteralDatatypeExternal(external_id)), GraphObjectType::LITERAL_DATATYPE_EXTERNAL);
    }

    static GraphObject make_literal_datatype_inlined(const char* str, uint16_t datatype_id) {
        return GraphObject(GraphObjectUnion(LiteralDatatypeInlined(str, datatype_id)), GraphObjectType::LITERAL_DATATYPE_INLINED);
    }

    static GraphObject make_literal_datatype_tmp(const LiteralDatatype& ld) {
        return GraphObject(GraphObjectUnion(LiteralDatatypeTmp(ld)), GraphObjectType::LITERAL_DATATYPE_TMP);
    }

    static GraphObject make_literal_language_external(uint64_t external_id) {
        return GraphObject(GraphObjectUnion(LiteralLanguageExternal(external_id)), GraphObjectType::LITERAL_LANGUAGE_EXTERNAL);
    }

    static GraphObject make_literal_language_inlined(const char* str, uint16_t language_id) {
        return GraphObject(GraphObjectUnion(LiteralLanguageInlined(str, language_id)), GraphObjectType::LITERAL_LANGUAGE_INLINED);
    }

    static GraphObject make_literal_language_tmp(const LiteralLanguage& ll) {
        return GraphObject(GraphObjectUnion(LiteralLanguageTmp(ll)), GraphObjectType::LITERAL_LANGUAGE_TMP);
    }

    static GraphObject make_datetime(uint64_t datetime_id) {
        return GraphObject(GraphObjectUnion(DateTime(datetime_id)), GraphObjectType::DATETIME);
    }

    static GraphObject make_decimal(uint64_t decimal_id) {
        return GraphObject(GraphObjectUnion(Decimal(decimal_id)), GraphObjectType::DECIMAL);
    }

    static GraphObject make_boolean(uint64_t boolean_id) {
        return GraphObject(GraphObjectUnion(Boolean(boolean_id)), GraphObjectType::BOOLEAN);
    }

    friend std::ostream& operator<<(std::ostream& os, const GraphObject& graph_obj) {
        return graph_object_print(os, graph_obj);
    }

    inline bool operator==(const GraphObject& rhs) const noexcept {
        return graph_object_eq(*this, rhs);
    }

    inline bool operator!=(const GraphObject& rhs) const noexcept {
        return !graph_object_eq(*this, rhs);
    }

    inline bool operator<=(const GraphObject& rhs) const noexcept {
        return graph_object_cmp(*this, rhs) <= 0;
    }

    inline bool operator<(const GraphObject& rhs) const noexcept {
        return graph_object_cmp(*this, rhs) < 0;
    }

    inline bool operator>=(const GraphObject& rhs) const noexcept {
        return graph_object_cmp(*this, rhs) >= 0;
    }

    inline bool operator>(const GraphObject& rhs) const noexcept {
        return graph_object_cmp(*this, rhs) > 0;
    }

    inline GraphObject operator+(const GraphObject& rhs) const noexcept {
        return graph_object_sum(*this, rhs);
    }

    inline GraphObject operator-(const GraphObject& rhs) const noexcept {
        return graph_object_minus(*this, rhs);
    }

    inline GraphObject operator*(const GraphObject& rhs) const noexcept {
        return graph_object_multiply(*this, rhs);
    }

    inline GraphObject operator/(const GraphObject& rhs) const noexcept {
        return graph_object_divide(*this, rhs);
    }

    inline GraphObject operator%(const GraphObject& rhs) const noexcept {
        return graph_object_modulo(*this, rhs);
    }

private:
    GraphObject(int64_t encoded_value, GraphObjectType type) : encoded_value(encoded_value), type(type) { }
};

class GraphObjectInterpreter {
public:
    template<typename T>
    static T get(const GraphObject graph_object);
};

static_assert(std::is_trivially_copyable<GraphObject>::value);

static_assert(std::is_trivially_assignable<GraphObject, GraphObject>::value);

static_assert(std::is_trivially_move_constructible<GraphObject>::value);

static_assert(sizeof(GraphObject) <= 16,
              "GraphObject size should be small, if it needs to increase the size avoid copies");
