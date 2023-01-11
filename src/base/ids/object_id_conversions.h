#pragma once

#include <cmath>
#include <iostream>

#include "base/ids/object_id.h"

class Conversions {
public:
    /*
    Implemented according to the Effective Boolean Value rules:
    https://www.w3.org/TR/2017/REC-xquery-31-20170321/#dt-ebv

    If the ObjectId can not be converted to boolean, it returns a Null ObjectId
    This Null ObjectId represents the Error Type according to the following:
    https://www.w3.org/TR/sparql11-query/#evaluation
    */
    static ObjectId to_boolean(ObjectId obj_id) {
        uint64_t type  = obj_id.id & ObjectId::TYPE_MASK;
        uint64_t value = obj_id.id & ObjectId::VALUE_MASK;
        // Boolean
        if (type == ObjectId::MASK_BOOL) {
            return obj_id;
        }
        // String
        // Note: Extern strings will never be empty
        else if (type == ObjectId::MASK_STRING_INLINED) {
            return ObjectId(ObjectId::MASK_BOOL | (value != 0));
        } else if (type == ObjectId::MASK_STRING_EXTERN) {
            return ObjectId(ObjectId::BOOL_TRUE);
        }
        // Integer
        else if (type == ObjectId::MASK_NEGATIVE_INT || type == ObjectId::MASK_POSITIVE_INT) {
            return ObjectId(ObjectId::MASK_BOOL | (value != 0));
        }
        // Float
        else if (type == ObjectId::MASK_FLOAT) {
            float f;
            uint8_t* dest = (uint8_t*)&f;
            dest[0] =  value        & 0xFF;
            dest[1] = (value >> 8)  & 0xFF;
            dest[2] = (value >> 16) & 0xFF;
            dest[3] = (value >> 24) & 0xFF;
            return ObjectId(ObjectId::MASK_BOOL | (f != 0 && !std::isnan(f)));
        }
        // Decimal
        // Note: Extern decimals will never be zero
        else if (type == ObjectId::MASK_DECIMAL_INLINED) {
            return ObjectId(ObjectId::MASK_BOOL | (value != 0));
        } else if (type == ObjectId::MASK_DECIMAL_EXTERN) {
            return ObjectId(ObjectId::BOOL_TRUE);
        }
        // Can not be converted to boolean
        else {
            return ObjectId::get_null();
        }
    }

    static ObjectId to_integer(ObjectId) {
        // TODO: Implement this
        return ObjectId::get_null();
    }

    static ObjectId to_decimal(ObjectId) {
        // TODO: Implement this
        return ObjectId::get_null();
    }

    static ObjectId to_float(ObjectId) {
        // TODO: Implement this
        return ObjectId::get_null();
    }
};