#include "graph_object_visitor.h"

GraphObjectVisitor::GraphObjectVisitor(QuadModel& model, bool create_if_not_exists) :
    model (model),
    create_if_not_exists (create_if_not_exists) { }


ObjectId GraphObjectVisitor::operator()(const IdentifiableInlined& identifiable_inlined) const {
    std::string str(identifiable_inlined.id);
    uint64_t res = 0;
    int shift_size = 0;
    for (uint64_t byte : str) { // MUST convert to 64bits or shift (shift_size >=32) is undefined behaviour
        res |= byte << shift_size;
        shift_size += 8;
    }
    return ObjectId(res | GraphModel::IDENTIFIABLE_INLINED_MASK);
}


ObjectId GraphObjectVisitor::operator()(const IdentifiableExternal& identifiable_external) const {
    std::string str(identifiable_external.id);
    auto external_id = model.get_external_id(str, create_if_not_exists);
    if (external_id == ObjectId::OBJECT_ID_NOT_FOUND) {
        return ObjectId(external_id);
    } else {
        return ObjectId(external_id | GraphModel::IDENTIFIABLE_EXTERNAL_MASK);
    }
}


ObjectId GraphObjectVisitor::operator()(const Edge& edge) const {
    return ObjectId(GraphModel::CONNECTION_MASK | edge.id);
}


ObjectId GraphObjectVisitor::operator()(const AnonymousNode& anonymous_node) const {
    return ObjectId(GraphModel::ANONYMOUS_NODE_MASK | anonymous_node.id);
}


ObjectId GraphObjectVisitor::operator()(const StringInlined& string_inlined) const {
    std::string str(string_inlined.id);
    auto string_len = str.length();

    uint64_t res = 0;
    int shift_size = 0;
    std::size_t i = 0;
    for (; i < string_len; i++) { // MUST convert to 64bits or shift (shift_size >=32) is undefined behaviour
        uint64_t byte = static_cast<uint8_t>(str[i]); // IMPORTANT static_cast, str[i] can be negative
        res |= byte << shift_size;
        shift_size += 8;
    }
    for (; i < 8; i++) { // MUST convert to 64bits or shift (shift_size >=32) is undefined behaviour
        uint64_t byte = static_cast<uint8_t>('\0'); // IMPORTANT static_cast, str[i] can be negative
        res |= byte << shift_size;
        shift_size += 8;
    }
    return ObjectId(res | GraphModel::VALUE_INLINE_STR_MASK);
}


ObjectId GraphObjectVisitor::operator()(const StringExternal& string_external) const {
    std::string str(string_external.id);
    auto external_id = model.get_external_id(str, create_if_not_exists);
    if (external_id == ObjectId::OBJECT_ID_NOT_FOUND) {
        return ObjectId(external_id);
    } else {
        return ObjectId(external_id | GraphModel::VALUE_EXTERNAL_STR_MASK);
    }
}


ObjectId GraphObjectVisitor::operator()(const NullGraphObject&) const {
    return ObjectId::get_null();
}


ObjectId GraphObjectVisitor::operator()(const NotFoundObject&) const {
    return ObjectId::get_not_found();
}


ObjectId GraphObjectVisitor::operator()(const int64_t n) const {
    int64_t int_value = n;
    uint64_t mask = GraphModel::VALUE_POSITIVE_INT_MASK;
    if (int_value < 0) {
        int_value *= -1;
        mask = GraphModel::VALUE_NEGATIVE_INT_MASK;
    }

    // check if it needs more than 7 bytes
    if ( (int_value & 0xFF00'0000'0000'0000UL) == 0) {
        return ObjectId(mask | int_value);
    } else {
        // VALUE_EXTERNAL_INT_MASK
        throw std::logic_error("NOT SUPPORTED YET");
    }
}


ObjectId GraphObjectVisitor::operator()(const bool value_bool) const {
    if (value_bool) {
        return ObjectId(GraphModel::VALUE_BOOL_MASK | 0x01);
    } else {
        return ObjectId(GraphModel::VALUE_BOOL_MASK | 0x00);
    }
}


ObjectId GraphObjectVisitor::operator()(const float value_float) const {
    static_assert(sizeof(value_float) == 4);
    unsigned char bytes[sizeof(value_float)];
    memcpy(bytes, &value_float, sizeof(value_float));

    uint64_t res = 0;
    int shift_size = 0;
    for (int i = 0; i < sizeof(bytes); ++i) {
        uint64_t byte = bytes[i];
        res |= byte << shift_size;
        shift_size += 8;
    }
    return ObjectId(GraphModel::VALUE_FLOAT_MASK | res);
}
