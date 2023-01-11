#pragma once

#include "base/ids/object_id.h"
#include "storage/string_manager.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>


class Conversions {
public:
    static constexpr uint64_t DECIMAL_SIGN_MASK     = 0x00'80'00'00'00'00'00'00UL;
    static constexpr uint64_t DECIMAL_NUMBER_MASK   = 0x00'7F'FF'FF'FF'FF'FF'F0UL;
    static constexpr uint64_t DECIMAL_SPARATOR_MASK = 0x00'00'00'00'00'00'00'0FUL;
    static constexpr uint64_t DECIMAL_MAX           = 0x00'07'FF'FF'FF'FF'FF'FFUL;
    /*
    Implemented according to the Effective Boolean Value rules:
    https://www.w3.org/TR/2017/REC-xquery-31-20170321/#dt-ebv

    If the ObjectId can not be converted to boolean, it returns a Null ObjectId
    This Null ObjectId represents the Error Type according to the following:
    https://www.w3.org/TR/sparql11-query/#evaluation
    */
    static ObjectId to_boolean(ObjectId oid) {
        uint64_t type  = oid.id & ObjectId::TYPE_MASK;
        uint64_t value = oid.id & ObjectId::VALUE_MASK;
        // Boolean
        if (type == ObjectId::MASK_BOOL) {
            return oid;
        }
        // String
        // Note: Extern strings will never be empty
        else if (type == ObjectId::MASK_STRING_INLINED)
        {
            return ObjectId(ObjectId::MASK_BOOL | (value != 0));
        } else if (type == ObjectId::MASK_STRING_EXTERN) {
            return ObjectId(ObjectId::BOOL_TRUE);
        }
        // Integer
        else if (type == ObjectId::MASK_NEGATIVE_INT || type == ObjectId::MASK_POSITIVE_INT)
        {
            return ObjectId(ObjectId::MASK_BOOL | (value != 0));
        }
        // Float
        else if (type == ObjectId::MASK_FLOAT)
        {
            float    f;
            uint8_t* dest = (uint8_t*) &f;
            dest[0]       = value & 0xFF;
            dest[1]       = (value >> 8) & 0xFF;
            dest[2]       = (value >> 16) & 0xFF;
            dest[3]       = (value >> 24) & 0xFF;
            return ObjectId(ObjectId::MASK_BOOL | (f != 0 && !std::isnan(f)));
        }
        // Decimal
        // Note: Extern decimals will never be zero
        else if (type == ObjectId::MASK_DECIMAL_INLINED)
        {
            return ObjectId(ObjectId::MASK_BOOL | (value != 0));
        } else if (type == ObjectId::MASK_DECIMAL_EXTERN) {
            return ObjectId(ObjectId::BOOL_TRUE);
        }
        // Can not be converted to boolean
        else
        {
            return ObjectId::get_null();
        }
    }

    static ObjectId positive_int_to_decimal(ObjectId oid) {
        assert((oid.id & ObjectId::TYPE_MASK) == ObjectId::MASK_POSITIVE_INT);

        auto value = oid.id & ObjectId::VALUE_MASK;

        if (value > DECIMAL_MAX) {
            auto str = std::to_string(value).append(".0");
            auto eid = string_manager.get_str_id(str, true);
            return ObjectId(ObjectId::MASK_DECIMAL_EXTERN | eid);
        } else {
            return ObjectId(ObjectId::MASK_DECIMAL_INLINED | (value << 4));
        }
    }

    static ObjectId negative_int_to_decimal(ObjectId oid) {
        assert((oid.id & ObjectId::TYPE_MASK) == ObjectId::MASK_NEGATIVE_INT);

        auto value = (~oid.id) & ObjectId::VALUE_MASK;

        if (value > DECIMAL_MAX) {
            auto str = std::to_string(value).insert(0, "-").append(".0");
            auto eid = string_manager.get_str_id(str, true);
            return ObjectId(ObjectId::MASK_DECIMAL_EXTERN | eid);
        } else {
            return ObjectId(ObjectId::MASK_DECIMAL_INLINED | DECIMAL_SIGN_MASK | (value << 4));
        }
    }

    static ObjectId positive_int_to_float(ObjectId oid) {
        assert((oid.id & ObjectId::TYPE_MASK) == ObjectId::MASK_POSITIVE_INT);

        auto value = oid.id & ObjectId::VALUE_MASK;
        auto flt   = static_cast<float>(value);
        auto src   = reinterpret_cast<char*>(&flt);

        auto fid = ObjectId::MASK_FLOAT;
        fid |= src[0];
        fid |= src[1] << 8;
        fid |= src[2] << 16;
        fid |= src[3] << 24;

        return ObjectId(fid);
    }

    static ObjectId negative_int_to_float(ObjectId oid) {
        assert((oid.id & ObjectId::TYPE_MASK) == ObjectId::MASK_NEGATIVE_INT);

        auto value = (~oid.id) & ObjectId::VALUE_MASK;
        auto flt   = -static_cast<float>(value);
        auto src   = reinterpret_cast<char*>(&flt);

        auto fid = ObjectId::MASK_FLOAT;
        fid |= src[0];
        fid |= src[1] << 8;
        fid |= src[2] << 16;
        fid |= src[3] << 24;

        return ObjectId(fid);
    }

    static ObjectId decimal_inlined_to_float(ObjectId oid) {
        assert((oid.id & ObjectId::TYPE_MASK) == ObjectId::MASK_DECIMAL_INLINED);

        auto sign     = (oid.id & DECIMAL_SIGN_MASK) != 0 ? -1.0 : 1.0;
        auto number   = static_cast<double>((oid.id & DECIMAL_NUMBER_MASK) >> 4);
        auto decimals = oid.id & DECIMAL_SPARATOR_MASK;

        auto divisor = std::pow(10, decimals);

        auto flt = static_cast<float>(sign * number / divisor);
        auto src = reinterpret_cast<char*>(&flt);

        auto fid = ObjectId::MASK_FLOAT;
        fid |= src[0];
        fid |= src[1] << 8;
        fid |= src[2] << 16;
        fid |= src[3] << 24;

        return ObjectId(fid);
    }

    static ObjectId decimal_extern_to_float(ObjectId oid) {
        assert((oid.id & ObjectId::TYPE_MASK) == ObjectId::MASK_DECIMAL_EXTERN);

        auto ss = std::stringstream();
        string_manager.print(ss, oid.id);

        auto flt = std::stof(ss.str());
        auto src = reinterpret_cast<char*>(&flt);

        auto fid = ObjectId::MASK_FLOAT;
        fid |= src[0];
        fid |= src[1] << 8;
        fid |= src[2] << 16;
        fid |= src[3] << 24;

        return ObjectId(fid);
    }

    static ObjectId to_integer(ObjectId oid) {
        switch (oid.id & ObjectId::TYPE_MASK) {
        case ObjectId::MASK_NEGATIVE_INT:
            return oid;
        case ObjectId::MASK_POSITIVE_INT:
            return oid;
        default:
            return ObjectId::get_null();
        }
    }

    static ObjectId to_decimal(ObjectId oid) {
        switch (oid.id & ObjectId::TYPE_MASK) {
        case ObjectId::MASK_NEGATIVE_INT:
            return negative_int_to_decimal(oid);
        case ObjectId::MASK_POSITIVE_INT:
            return positive_int_to_decimal(oid);
        case ObjectId::MASK_DECIMAL_INLINED:
            return oid;
        case ObjectId::MASK_DECIMAL_EXTERN:
            return oid;
        default:
            return ObjectId::get_null();
        }
    }

    static ObjectId to_float(ObjectId oid) {
        switch (oid.id & ObjectId::TYPE_MASK) {
        case ObjectId::MASK_NEGATIVE_INT:
            return negative_int_to_float(oid);
        case ObjectId::MASK_POSITIVE_INT:
            return positive_int_to_float(oid);
        case ObjectId::MASK_DECIMAL_INLINED:
            return decimal_inlined_to_float(oid);
        case ObjectId::MASK_DECIMAL_EXTERN:
            return decimal_extern_to_float(oid);
        case ObjectId::MASK_FLOAT:
            return oid;
        default:
            return ObjectId::get_null();
        }
    }
};
