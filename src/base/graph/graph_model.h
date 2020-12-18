#ifndef BASE__GRAPH_MODEL_H_
#define BASE__GRAPH_MODEL_H_

#include <memory>

#include "base/binding/binding_iter.h"
#include "base/ids/object_id.h"
#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/grammar/manual_plan/manual_plan_ast.h"

class GraphModel {
public:
    static constexpr auto MAX_INLINED_BYTES =  7; // Ids have 8 bytes, 1 for type and 7 remaining
    static constexpr auto TYPE_OFFSET       = 56; // total_bits - bits_for_type: 64 - 8

    static constexpr uint64_t TYPE_MASK                  = 0xFF'00000000000000UL;
    static constexpr uint64_t VALUE_MASK                 = 0x00'FFFFFFFFFFFFFFUL;

    // TYPES
    static constexpr uint64_t IDENTIFIABLE_EXTERNAL_MASK = 0x00'00000000000000UL;
    static constexpr uint64_t IDENTIFIABLE_INLINED_MASK  = 0x01'00000000000000UL;
    static constexpr uint64_t ANONYMOUS_NODE_MASK        = 0x02'00000000000000UL;
    static constexpr uint64_t CONNECTION_MASK            = 0x03'00000000000000UL;

    static constexpr uint64_t VALUE_EXTERNAL_STR_MASK    = 0x04'00000000000000UL;
    static constexpr uint64_t VALUE_INLINE_STR_MASK      = 0x05'00000000000000UL;

    static constexpr uint64_t VALUE_NEGATIVE_INT_MASK    = 0x06'00000000000000UL;
    static constexpr uint64_t VALUE_POSITIVE_INT_MASK    = 0x07'00000000000000UL;

    static_assert(VALUE_NEGATIVE_INT_MASK < VALUE_POSITIVE_INT_MASK,
        "INLINED INTEGERS WON'T BE ORDERED PROPERTLY IN THE BPT.");

    // TODO: big numbers not supported yet
    // static constexpr uint64_t VALUE_EXTERN_POS_INT_MASK = 0x08'00000000000000UL;
    // static constexpr uint64_t VALUE_EXTERN_NEG_INT_MASK = 0x09'00000000000000UL;

    static constexpr uint64_t VALUE_FLOAT_MASK           = 0x0A'00000000000000UL;
    static constexpr uint64_t VALUE_BOOL_MASK            = 0x0B'00000000000000UL;

    virtual ~GraphModel() = default;

    virtual std::unique_ptr<BindingIter> exec(OpSelect&) = 0;
    virtual std::unique_ptr<BindingIter> exec(manual_plan::ast::ManualRoot&) = 0;

    virtual ObjectId get_object_id(const GraphObject&) = 0;
    virtual GraphObject get_graph_object(ObjectId) = 0;
    virtual GraphObject get_property_value(GraphObject& var, const ObjectId key) = 0;
};

#endif // BASE__GRAPH_MODEL_H_