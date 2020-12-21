#ifndef BASE__OBJECT_TYPE_H_
#define BASE__OBJECT_TYPE_H_

#include <ostream>
#include <memory>
#include <string>
#include <variant>

#include "base/graph/anonymous_node.h"
#include "base/graph/identifiable_inlined.h"
#include "base/graph/identifiable_external.h"
#include "base/graph/edge.h"
#include "base/graph/string_inlined.h"
#include "base/graph/string_external.h"

struct NullGraphObject {
    inline bool operator==(const NullGraphObject&) const noexcept {
        return false;
    }

    inline bool operator!=(const NullGraphObject&) const noexcept {
        return false;
    }

    inline bool operator<=(const NullGraphObject&) const noexcept {
        return false;
    }

    inline bool operator>=(const NullGraphObject&) const noexcept {
        return false;
    }

    inline bool operator<(const NullGraphObject&) const noexcept {
        return false;
    }

    inline bool operator>(const NullGraphObject&) const noexcept {
        return false;
    }
};

struct NotFoundObject {
    inline bool operator==(const NotFoundObject&) const noexcept {
        return false;
    }

    inline bool operator!=(const NotFoundObject&) const noexcept {
        return false;
    }

    inline bool operator<=(const NotFoundObject&) const noexcept {
        return false;
    }

    inline bool operator>=(const NotFoundObject&) const noexcept {
        return false;
    }

    inline bool operator<(const NotFoundObject&) const noexcept {
        return false;
    }

    inline bool operator>(const NotFoundObject&) const noexcept {
        return false;
    }
};


using GraphObjectVariant = std::variant<
        IdentifiableInlined,
        IdentifiableExternal,
        Edge,
        AnonymousNode,
        StringInlined,
        StringExternal,
        NullGraphObject,
        NotFoundObject,
        int64_t,
        bool,
        float>;

struct GraphObjectOstreamVisitor {
    std::ostream& os;

    GraphObjectOstreamVisitor(std::ostream& os) :
        os (os) { }

    void operator()(const IdentifiableInlined& i)   const { os << i.id; }
    void operator()(const IdentifiableExternal& i)  const { os << i.id; }
    void operator()(const Edge& e)                  const { os << "_e(" << e.id << ')'; }
    void operator()(const AnonymousNode& a)         const { os << "_a(" << a.id << ')'; }
    void operator()(const StringInlined& s)         const { os << s.id; }
    void operator()(const StringExternal& s)        const { os << s.id; }
    void operator()(const NullGraphObject&)         const { os << "null"; }
    void operator()(const NotFoundObject&)          const { os << "NotFoundObj"; }
    void operator()(const int64_t n)                const { os << n; }
    void operator()(const bool b)                   const { os << b; }
    void operator()(const float f)                  const { os << f; }
};


class GraphObject {
public:
    GraphObjectVariant value;

    static GraphObject make_identifiable_external(const char* str) {
        IdentifiableExternal string_external{ str };
        return GraphObject(string_external);
    }

    static GraphObject make_identifiable_inlined(const char c[8]) {
        IdentifiableInlined idetifiable_inlined{ c };
        return GraphObject(idetifiable_inlined);
    }

    static GraphObject make_anonymous(const uint64_t id) {
        return GraphObject(AnonymousNode(id));
    }

    static GraphObject make_edge(const uint64_t id) {
        return GraphObject(AnonymousNode(id));
    }

    static GraphObject make_int(const int64_t i) {
        return GraphObject(i);
    }

    static GraphObject make_float(const float f) {
        return GraphObject(f);
    }

    static GraphObject make_bool(const bool b) {
        return GraphObject(b);
    }

    static GraphObject make_null() {
        return GraphObject(NullGraphObject());
    }

    static GraphObject make_not_found() {
        return GraphObject(NotFoundObject());
    }

    static GraphObject make_string(const std::string& str) {
        if (str.size() < 8) {
            char c[8];
            std::size_t i = 0;
            for (; i < str.size(); ++i) {
                c[i] = str[i];
            }
            for (; i < 8; ++i) {
                c[i] = '\0';
            }
            return GraphObject::make_string_inlined(c);
        } else {
            return GraphObject::make_string_external(str.c_str());
        }
    }

    static GraphObject make_identifiable(const std::string& str) {
        if (str.size() < 8) {
            char c[8];
            std::size_t i = 0;
            for (; i < str.size(); ++i) {
                c[i] = str[i];
            }
            for (; i < 8; ++i) {
                c[i] = '\0';
            }
            return GraphObject::make_identifiable_inlined(c);
        } else {
            return GraphObject::make_identifiable_external(str.c_str());
        }
    }

    static GraphObject make_string_external(const char* str) {
        StringExternal string_external{ str };
        return GraphObject(string_external);
    }

    // must receive an array of size 8, terminating in '\0'
    static GraphObject make_string_inlined(const char* str) {
        StringInlined string_inlined{ str };
        return GraphObject(string_inlined);
    }

    friend std::ostream& operator<<(std::ostream& os, const GraphObject& graph_obj) {
        std::visit(GraphObjectOstreamVisitor{os}, graph_obj.value);
        return os;
    }

    bool operator==(const GraphObject& rhs) const noexcept {
        return this->value == rhs.value;
    }

    bool operator!=(const GraphObject& rhs) const noexcept {
        return this->value != rhs.value;
    }

    bool operator<=(const GraphObject& rhs) const noexcept {
        return this->value <= rhs.value;
    }

    bool operator>=(const GraphObject& rhs) const noexcept {
        return this->value >= rhs.value;
    }

    bool operator<(const GraphObject& rhs)  const noexcept {
        return this->value < rhs.value;
    }

    bool operator>(const GraphObject& rhs)  const noexcept {
        return this->value > rhs.value;
    }

private:
    GraphObject(GraphObjectVariant value) :
        value (value) { }
};

static_assert(sizeof(GraphObject) <= 16, "GraphObject size should be small, if it needs to increase the size avoid copies");

#endif // BASE__OBJECT_TYPE_H_
