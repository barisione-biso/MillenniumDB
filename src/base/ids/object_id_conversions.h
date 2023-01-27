#pragma once

#include "base/graph_object/decimal_inlined.h"
#include "base/ids/object_id.h"
#include "base/query/sparql/decimal.h"
#include "import/inliner.h"
#include "storage/string_manager.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>


class Conversions {
public:
    static constexpr uint64_t DECIMAL_SIGN_MASK     = 0x0080'0000'0000'0000UL;
    static constexpr uint64_t DECIMAL_NUMBER_MASK   = 0x007F'FFFF'FFFF'FFF0UL;
    static constexpr uint64_t DECIMAL_SPARATOR_MASK = 0x0000'0000'0000'000FUL;
    static constexpr uint64_t DECIMAL_MAX           = 0x0007'FFFF'FFFF'FFFFUL;
    static constexpr uint64_t FLOAT_MASK            = 0x0000'0000'FFFF'FFFFUL;
    static constexpr uint64_t FLOAT_SIGN_MASK       = 0x0000'0000'8000'0000UL;
    static constexpr int64_t  INTEGER_MAX           = 0x00FF'FFFF'FFFF'FFFFL;

    // The order, int < dec < flt < inv is important
    static constexpr uint8_t OPTYPE_INTEGER = 0x01;
    static constexpr uint8_t OPTYPE_DECIMAL = 0x02;
    static constexpr uint8_t OPTYPE_FLOAT   = 0x03;
    static constexpr uint8_t OPTYPE_INVALID = 0x04;

    /*
    Implemented according to the Effective Boolean Value rules:
    https://www.w3.org/TR/2017/REC-xquery-31-20170321/#dt-ebv

    If the ObjectId can not be converted to boolean, it returns a Null ObjectId
    This Null ObjectId represents the Error Type according to the following:
    https://www.w3.org/TR/sparql11-query/#evaluation
    */
    static ObjectId to_boolean(ObjectId oid) {
        uint64_t type  = oid.get_type();
        uint64_t value = oid.get_value();

        switch (type) {
        // Boolean
        case ObjectId::MASK_BOOL:
            return oid;
        // String
        // Note: Extern strings will never be empty
        case ObjectId::MASK_STRING_INLINED:
            return ObjectId(ObjectId::MASK_BOOL | (value != 0));
        case ObjectId::MASK_STRING_EXTERN:
            return ObjectId(ObjectId::BOOL_TRUE);
        // Integer
        case ObjectId::MASK_NEGATIVE_INT:
        case ObjectId::MASK_POSITIVE_INT:
            return ObjectId(ObjectId::MASK_BOOL | (value != 0));
        // Float
        case ObjectId::MASK_FLOAT: {
            auto f = unpack_float(oid);
            return ObjectId(ObjectId::MASK_BOOL | (f != 0 && !std::isnan(f)));
        }
        // Decimal
        // Note: This assumes 0 is never represented as 0.0, 0.00, etc
        case ObjectId::MASK_DECIMAL_INLINED:
            return ObjectId(ObjectId::MASK_BOOL | (value != 0));
        // Note: Extern decimals will never be zero
        case ObjectId::MASK_DECIMAL_EXTERN:
            return ObjectId(ObjectId::BOOL_TRUE);
        // Can not be converted to boolean
        default:
            return ObjectId::get_null();
        }
    }

    /*
    Steps to evaluate an expression:
        - Calculate the datatype the operation should use (calculate_optype)
        - unpack operands (unpack_x)
        - convert operands to previously calculated datatype
        - evaluate operation
        - pack result (pack_x)

    Type promotion and type substitution order:
        integer -> decimal -> float (-> double)

    Conversion:
        int64_t -> Decimal (Decimal constructor)
        int64_t -> float (cast)
        Decimal -> float (Decimal member function)
    */

    /**
     *  @brief Unpacks the positive_int inside an ObjectId.
     *  @param oid The ObjectId to unpack.
     *  @return The value inside the ObjectId.
     */
    static int64_t unpack_positive_int(ObjectId oid) {
        assert(oid.get_type() == ObjectId::MASK_POSITIVE_INT);

        return static_cast<int64_t>(oid.get_value());
    }


    /**
     *  @brief Unpacks the negative_int inside an ObjectId.
     *  @param oid The ObjectId to unpack.
     *  @return The value inside the ObjectId.
     */
    static int64_t unpack_negative_int(ObjectId oid) {
        assert(oid.get_type() == ObjectId::MASK_NEGATIVE_INT);

        return static_cast<int64_t>((~oid.id) & ObjectId::VALUE_MASK) * -1;
    }

    /**
     *  @brief Unpacks the internal Decimal of an ObjectId.
     *  @param oid The ObjectId to unpack.
     *  @return The value inside the ObjectId.
     */
    static Decimal unpack_decimal_inlined(ObjectId oid) {
        assert(oid.get_type() == ObjectId::MASK_DECIMAL_INLINED);

        auto sign     = (oid.id & DECIMAL_SIGN_MASK) != 0;
        auto number   = (oid.id & DECIMAL_NUMBER_MASK) >> 4;
        auto decimals = oid.id & DECIMAL_SPARATOR_MASK;

        std::string dec_str;

        if (sign) {
            dec_str += '-';
        } else {
            dec_str += '+';
        }

        auto num_str      = std::to_string(number);
        auto num_str_size = num_str.size();
        if (decimals > 0) {
            if (decimals >= num_str_size) {
                dec_str += "0.";
                if (decimals > num_str_size) {
                    dec_str += std::string(decimals - num_str_size, '0');
                }
                dec_str += num_str;
            } else {
                dec_str += num_str;
                dec_str.insert(dec_str.length() - decimals, ".");
            }
        } else {
            dec_str += num_str;
            dec_str += (".0");
        }

        // TODO: inefficient, str is already normalized, but the constructor of Decimal normalizes again.
        return Decimal(dec_str);
    }

    /**
     *  @brief Unpacks the external Decimal of an ObjectId.
     *  @param oid The ObjectId to unpack.
     *  @return The value referenced by the ObjectId.
     */
    static Decimal unpack_decimal_extern(ObjectId oid) {
        assert(oid.get_type() == ObjectId::MASK_DECIMAL_EXTERN);

        auto ss = std::stringstream();
        string_manager.print(ss, oid.get_value());

        Decimal dec;
        dec.from_external(ss.str());
        return dec;
    }

    /**
     *  @brief Unpacks the float inside an ObjectId.
     *  @param oid The ObjectId to unpack.
     *  @return The value contained in the ObjectId.
     */
    static float unpack_float(ObjectId oid) {
        assert(oid.get_type() == ObjectId::MASK_FLOAT);

        auto  value = oid.id;
        float flt;
        auto  dst = reinterpret_cast<char*>(&flt);

        dst[0] = value & 0xFF;
        dst[1] = (value >> 8) & 0xFF;
        dst[2] = (value >> 16) & 0xFF;
        dst[3] = (value >> 24) & 0xFF;

        return flt;
    }

    /**
     *  @brief Packs an int64_t into an ObjectId.
     *  @param dec The int64_t value that should be packed.
     *  @return An ObjectId (positive_integer or negative_integer) containing the value or null if it does not fit.
     */
    static ObjectId pack_int(int64_t i) {
        uint64_t oid = ObjectId::MASK_POSITIVE_INT;

        if (i < 0) {
            oid = ObjectId::MASK_NEGATIVE_INT;
            i *= -1;
            if (i > INTEGER_MAX) {
                return ObjectId::get_null();
            }
            i = (~i) & ObjectId::VALUE_MASK;
        } else {
            if (i > INTEGER_MAX) {
                return ObjectId::get_null();
            }
        }

        return ObjectId(oid | i);
    }

    /**
     *  @brief Packs a normalized Decimal into an ObjectId.
     *  @param dec A normalized Decimal that should be packed.
     *  @return An ObjectId (inlined or extern) containing the value.
     */
    static ObjectId pack_decimal(Decimal dec) {
        uint64_t decimal_id = DecimalInlined::get_decimal_id(dec.to_string().c_str());
        uint64_t oid;

        if (decimal_id == DecimalInlined::INVALID_ID) {
            // TODO: Use temporary strings
            oid = string_manager.get_str_id(dec.to_external(), true) | ObjectId::MASK_DECIMAL_EXTERN;
        } else {
            oid = decimal_id | ObjectId::MASK_DECIMAL_INLINED;
        }

        return ObjectId(oid);
    }

    /**
     *  @brief Packs a float into an ObjectId.
     *  @param flt The float value that should be packed.
     *  @return An ObjectId containing the value.
     */
    static ObjectId pack_float(float flt) {
        auto src = reinterpret_cast<unsigned char*>(&flt);

        auto oid = ObjectId::MASK_FLOAT;
        oid |= static_cast<uint64_t>(src[0]);
        oid |= static_cast<uint64_t>(src[1]) << 8;
        oid |= static_cast<uint64_t>(src[2]) << 16;
        oid |= static_cast<uint64_t>(src[3]) << 24;

        return ObjectId(oid);
    }

    /**
     *  @brief Calculates the datatype that should be used for expression evaluation.
     *  @param oid1 ObjectId of the first operand.
     *  @param oid2 ObjectId of the second operand.
     *  @return datatype that should be used or OPTYPE_INVALID if not both operands are numeric.
     */
    static uint8_t calculate_optype(ObjectId oid1, ObjectId oid2) {
        auto optype1 = calculate_optype(oid1);
        auto optype2 = calculate_optype(oid2);
        return std::max(optype1, optype2);
    }

    /**
     *  @brief Calculates the genric datatypes of the operand in an expression.
     *  @param oid ObjectId of the operand involved in an expression.
     *  @return genric numeric datatype of the operand or OPTYPE_INVALID if oid is not numeric
     */
    static uint8_t calculate_optype(ObjectId oid) {
        switch (oid.get_type()) {
        case ObjectId::MASK_POSITIVE_INT:
        case ObjectId::MASK_NEGATIVE_INT:
            return OPTYPE_INTEGER;
        case ObjectId::MASK_DECIMAL_INLINED:
        case ObjectId::MASK_DECIMAL_EXTERN:
            return OPTYPE_DECIMAL;
        case ObjectId::MASK_FLOAT:
            return OPTYPE_FLOAT;
        default:
            return OPTYPE_INVALID;
        }
    }

    /**
     *  @brief Converts an ObjectId to int64_t if permitted.
     *  @param oid ObjectId to convert.
     *  @return an int64_t representing the ObjectId.
     *  @throws LogicException if the ObjectId has no permitted type.
     */
    static int64_t to_integer(ObjectId oid) {
        switch (oid.get_type()) {
        case ObjectId::MASK_POSITIVE_INT:
            return unpack_positive_int(oid);
        case ObjectId::MASK_NEGATIVE_INT:
            return unpack_negative_int(oid);
        default:
            throw LogicException("Called to_integer with incorrect ObjectId type, this should never happen");
        }
    }

    /**
     *  @brief Converts an ObjectId to Decimal if permitted.
     *  @param oid ObjectId to convert.
     *  @return a Decimal representing the ObjectId.
     *  @throws LogicException if the ObjectId has no permitted type.
     */
    static Decimal to_decimal(ObjectId oid) {
        switch (oid.get_type()) {
        case ObjectId::MASK_POSITIVE_INT:
            return Decimal(unpack_positive_int(oid));
        case ObjectId::MASK_NEGATIVE_INT:
            return Decimal(unpack_negative_int(oid));
        case ObjectId::MASK_DECIMAL_INLINED:
            return unpack_decimal_inlined(oid);
        case ObjectId::MASK_DECIMAL_EXTERN:
            return unpack_decimal_extern(oid);
        default:
            throw LogicException("Called to_decimal with incorrect ObjectId type, this should never happen");
        }
    }

    /**
     *  @brief Converts an ObjectId to float if permitted.
     *  @param oid ObjectId to convert.
     *  @return a float representing the ObjectId.
     *  @throws LogicException if the ObjectId has no permitted type.
     */
    static float to_float(ObjectId oid) {
        switch (oid.get_type()) {
        case ObjectId::MASK_POSITIVE_INT:
            return static_cast<float>(unpack_positive_int(oid));
        case ObjectId::MASK_NEGATIVE_INT:
            return static_cast<float>(unpack_negative_int(oid));
        case ObjectId::MASK_DECIMAL_INLINED:
            return unpack_decimal_inlined(oid).to_float();
        case ObjectId::MASK_DECIMAL_EXTERN:
            return unpack_decimal_extern(oid).to_float();
        case ObjectId::MASK_FLOAT:
            return unpack_float(oid);
        default:
            throw LogicException("Called to_decimal with incorrect ObjectId type, this should never happen");
        }
    }

    /**
     *  @brief Converts an ObjectId to double if permitted.
     *  @param oid ObjectId to convert.
     *  @return a double representing the ObjectId.
     *  @throws LogicException if the ObjectId has no permitted type.
     */
    static double to_double(ObjectId oid) {
        switch (oid.get_type()) {
        case ObjectId::MASK_POSITIVE_INT:
            return static_cast<double>(unpack_positive_int(oid));
        case ObjectId::MASK_NEGATIVE_INT:
            return static_cast<double>(unpack_negative_int(oid));
        case ObjectId::MASK_DECIMAL_INLINED:
            return unpack_decimal_inlined(oid).to_double();
        case ObjectId::MASK_DECIMAL_EXTERN:
            return unpack_decimal_extern(oid).to_double();
        case ObjectId::MASK_FLOAT:
            return unpack_float(oid);
        default:
            throw LogicException("Called to_decimal with incorrect ObjectId type, this should never happen");
        }
    }

    /**
     * @brief Unpacks the inlined string inside an ObjectId.
     *
     * @param oid ObjectId to unpack.
     * @return std::string string inside the ObjectId.
     */
    static std::string unpack_string_inlined(ObjectId oid) {
        assert(oid.get_type() == ObjectId::MASK_STRING_INLINED);

        char str[8];
        int  shift_size = 6 * 8;
        for (int i = 0; i < ObjectId::MAX_INLINED_BYTES; i++) {
            uint8_t byte = (oid.id >> shift_size) & 0xFF;
            str[i]       = byte;
            shift_size -= 8;
        }
        str[7] = '\0';

        return std::string(str);
    }

    /**
     * @brief Unpacks the external string inside an ObjectId.
     *
     * @param oid ObjectId to unpack.
     * @return std::string string inside the ObjectId.
     */
    static std::string unpack_string_extern(ObjectId oid) {
        assert(oid.get_type() == ObjectId::MASK_STRING_EXTERN);

        // TODO: Check if this is correct
        std::stringstream ss;
        string_manager.print(ss, oid.get_value());

        return ss.str();
    }

    /**
     * @brief Packs a string into an ObjectId.
     *
     * @param str string to pack.
     * @return ObjectId containing the string.
     */
    static ObjectId pack_string(const std::string& str) {
        uint64_t oid;
        if (str.size() < 8) {
            // TODO: Move inliner class to another file?
            oid = Inliner::inline_string(str.c_str()) | ObjectId::MASK_STRING_INLINED;
        } else {
            // TODO: Use temporary strings
            oid = string_manager.get_str_id(str, true) | ObjectId::MASK_STRING_EXTERN;
        }
        return ObjectId(oid);
    }

    /**
     * @brief Pack an IRI string into an ObjectId.
     *
     * @param str IRI string to pack.
     * @return ObjectId containing the IRI.
     */
    static ObjectId pack_iri(const std::string& str) {
        // TODO: Handle prefixes
        uint64_t oid;
        if (str.size() < 7) {
            // TODO: Move inliner class to another file?
            oid = Inliner::inline_iri(str.c_str()) | ObjectId::MASK_IRI_INLINED;
        } else {
            // TODO: Use temporary strings
            oid = string_manager.get_str_id(str, true) | ObjectId::MASK_IRI_EXTERN;
        }
        return ObjectId(oid);
    }
};
