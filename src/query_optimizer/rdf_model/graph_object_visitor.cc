#include "graph_object_visitor.h"

#include "base/exceptions.h"
#include "query_optimizer/rdf_model/rdf_model.h"

using namespace SPARQL;

GraphObjectVisitor::GraphObjectVisitor(bool create_if_not_exists) :
    create_if_not_exists (create_if_not_exists) { }

ObjectId GraphObjectVisitor::operator()(const GraphObject& obj) const {
    // TODO: implement
    // switch (obj.type) {
    //     case GraphObjectType::STR_INLINED: {
    //         std::string str(obj.value.str_inlined.id);

    //         uint64_t res = 0;
    //         int shift_size = 8*6;
    //         for (uint8_t byte : str) {
    //             uint64_t byte64 = static_cast<uint64_t>(byte);
    //             res |= byte64 << shift_size;
    //             shift_size -= 8;
    //         }
    //         return ObjectId(res | ObjectId::VALUE_INLINE_STR_MASK);
    //     }
    //     case GraphObjectType::STR_TMP: {
    //         if (create_if_not_exists) {
    //             bool created;
    //             auto external_id = rdf_model.strings_hash().get_or_create_id(*obj.value.str_tmp.str, &created);
    //             return ObjectId(external_id | ObjectId::VALUE_EXTERNAL_STR_MASK);
    //         } else {
    //             auto external_id = rdf_model.strings_hash().get_id(*obj.value.str_tmp.str);
    //             if (external_id == ObjectId::OBJECT_ID_NOT_FOUND) {
    //                 return ObjectId::get_not_found();
    //             } else {
    //                 return ObjectId(external_id | ObjectId::VALUE_EXTERNAL_STR_MASK);
    //             }
    //         }
    //     }
    //     case GraphObjectType::IRI_INLINED: {
    //         std::string str(obj.value.iri_inlined.id);
    //         uint64_t shifted_prefix_id = static_cast<uint64_t>(obj.value.iri_inlined.prefix_id) << 48;

    //         uint64_t res = 0;
    //         int shift_size = 8*5;
    //         for (uint8_t byte : str) {
    //             uint64_t byte64 = static_cast<uint64_t>(byte);
    //             res |= byte64 << shift_size;
    //             shift_size -= 8;
    //         }
    //         return ObjectId(res | ObjectId::MASK_IRI_INLINED | shifted_prefix_id);
    //     }
    //     case GraphObjectType::IRI_TMP: {
    //         if (create_if_not_exists) {
    //             // TODO: Implement this
    //             return ObjectId::get_null();
    //         } else {
    //             auto& prefixes = rdf_model.catalog().prefixes;
    //             std::string str = *obj.value.iri_tmp.str;
    //             uint8_t prefix_id = 0;
    //             for (size_t i = 0; i < prefixes.size(); ++i) {
    //                 if (str.compare(0, prefixes[i].size(), prefixes[i]) == 0) {
    //                     str = str.substr(prefixes[i].size(), str.size() - prefixes[i].size());
    //                     prefix_id = i;
    //                     break;
    //                 }
    //             }
    //             uint64_t external_id = rdf_model.strings_hash().get_id(str);
    //             uint64_t shifted_prefix_id = static_cast<uint64_t>(prefix_id) << 48;
    //             if (external_id == ObjectId::OBJECT_ID_NOT_FOUND) {
    //                 return ObjectId::get_not_found();
    //             } else {
    //                 return ObjectId(external_id | ObjectId::MASK_IRI_EXTERN | shifted_prefix_id);
    //             }
    //         }
    //     }
    //     case GraphObjectType::LITERAL_DATATYPE_INLINED: {
    //         std::string str(obj.value.literal_datatype_inlined.id);
    //         uint64_t shifted_datatype_id = static_cast<uint64_t>(obj.value.literal_datatype_inlined.datatype_id) << 40;

    //         uint64_t res = 0;
    //         int shift_size = 8*4;
    //         for (uint8_t byte : str) {
    //             uint64_t byte64 = static_cast<uint64_t>(byte);
    //             res |= byte64 << shift_size;
    //             shift_size -= 8;
    //         }
    //         return ObjectId(res | ObjectId::MASK_STRING_DATATYPE_INLINED | shifted_datatype_id);
    //     }
    //     case GraphObjectType::LITERAL_DATATYPE_TMP: {
    //         if (create_if_not_exists) {
    //             // TODO: Implement this
    //             return ObjectId::get_null();
    //         } else {
    //             // TODO: Hanlde datatype_id not found ?
    //             auto& datatypes      = rdf_model.catalog().datatypes;
    //             std::string str      = (*obj.value.literal_datatype_tmp.ld).str;
    //             std::string datatype = (*obj.value.literal_datatype_tmp.ld).datatype;

    //             uint16_t datatype_id = 0;
    //             for (size_t i = 0; i < datatypes.size(); ++i) {
    //                 if (datatype == datatypes[i]) {
    //                     datatype_id = i;
    //                     break;
    //                 }
    //             }
    //             uint64_t external_id = rdf_model.strings_hash().get_id(str);
    //             uint64_t shifted_datatype_id = static_cast<uint64_t>(datatype_id) << 40;
    //             if (external_id == ObjectId::OBJECT_ID_NOT_FOUND) {
    //                 return ObjectId::get_not_found();
    //             } else {
    //                 return ObjectId(external_id | ObjectId::MASK_STRING_DATATYPE_EXTERN | shifted_datatype_id);
    //             }
    //         }
    //     }
    //     case GraphObjectType::LITERAL_LANGUAGE_INLINED: {
    //         std::string str(obj.value.literal_language_inlined.id);
    //         uint64_t shifted_language_id = static_cast<uint64_t>(obj.value.literal_language_inlined.language_id) << 40;

    //         uint64_t res = 0;
    //         int shift_size = 8*4;
    //         for (uint8_t byte : str) {
    //             uint64_t byte64 = static_cast<uint64_t>(byte);
    //             res |= byte64 << shift_size;
    //             shift_size -= 8;
    //         }
    //         return ObjectId(res | ObjectId::MASK_STRING_LANG_INLINED | shifted_language_id);
    //     }
    //     case GraphObjectType::LITERAL_LANGUAGE_TMP: {
    //         if (create_if_not_exists) {
    //             // TODO: Implement this
    //             return ObjectId::get_null();
    //         } else {
    //             // TODO: Hanlde language_id not found ?
    //             auto& languages      = rdf_model.catalog().languages;
    //             std::string str      = (*obj.value.literal_language_tmp.ll).str;
    //             std::string language = (*obj.value.literal_language_tmp.ll).language;

    //             uint16_t language_id = 0;
    //             for (size_t i = 0; i < languages.size(); ++i) {
    //                 if (language == languages[i]) {
    //                     language_id = i;
    //                     break;
    //                 }
    //             }
    //             uint64_t external_id = rdf_model.strings_hash().get_id(str);
    //             uint64_t shifted_language_id = static_cast<uint64_t>(language_id) << 40;
    //             if (external_id == ObjectId::OBJECT_ID_NOT_FOUND) {
    //                 return ObjectId::get_not_found();
    //             } else {
    //                 return ObjectId(external_id | ObjectId::MASK_STRING_LANG_EXTERN | shifted_language_id);
    //             }
    //         }
    //     }
    //     case GraphObjectType::DATETIME: {
    //         return ObjectId(obj.value.datetime.id | ObjectId::MASK_DATETIME);
    //     }
    //     case GraphObjectType::DECIMAL: {
    //         return ObjectId(obj.value.decimal.id | ObjectId::MASK_DECIMAL);
    //     }
    //     case GraphObjectType::BOOLEAN: {
    //         return ObjectId(obj.value.boolean.id | ObjectId::VALUE_BOOLEAN_MASK);
    //     }
    //     default:
    //         return ObjectId::get_not_found();
    // }
    // unreachable
    return ObjectId::get_null();
}