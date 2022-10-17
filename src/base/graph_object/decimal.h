#pragma once

#include <cstring>
#include <cstdint>
#include <iomanip>
#include <math.h>

class Decimal {
public:
    static constexpr uint64_t INVALID_ID = UINT64_MAX;

    static uint64_t get_decimal_id(const char* str) {
        uint64_t ret = 0;
        char* ptr    = const_cast<char*>(str);

        if (*ptr == '-') {
            // Set sign bit
            ret |= 1ULL << 55;
            ptr++;
        }
        // Skip plus sign
        else if (*ptr == '+') {
            ptr++;
        }

        uint64_t number = 0;
        uint64_t sep    = 0;
        if (*ptr != '.') {
            // Handle integer part
            number = strtoull(ptr, &ptr, 10);
            if (number > 0x0007'FFFF'FFFF'FFFF) {
                // Integer overflow
                return INVALID_ID;
            }
        }
        if (*ptr == '.') {
            // Handle fractional part
            char* start = ++ptr;
            uint64_t fraction = strtoull(ptr, &ptr, 10);
            // Store separator only if fractional part is not zero
            if (fraction > 0) {
                // Remove trailing zeros
                while (fraction % 10 == 0) {
                    fraction /= 10;
                    ptr--;
                }

                if (fraction > 0x0007'FFFF'FFFF'FFFF) {
                    // Fractional overflow
                    return INVALID_ID;
                }

                sep = ptr - start;
                uint64_t times = 10;
                while (times < fraction) {
                    times *= 10;
                }
                number = number * times + fraction;
                
                if (number > 0x0007'FFFF'FFFF'FFFF) {
                    // Integer overflow (after adding fractional part)
                    return INVALID_ID;
                }
            }
        }

        ret |= sep;
        ret |= (number << 4);

        return ret;
    }

    static std::string normalize(std::string& str) {
        // Normalized format: (0|[1-9][0-9]*).(0|[0-9]*[1-9])
        std::stringstream ss;

        if (str[0] == '-') {
            ss << '-';
        }
        // Skip leading zeros and signs
        size_t start = str.find_first_not_of("+-0");
        if (start == std::string::npos) {
            // All zeros (e.g. 00000000)
            ss << "0.0";
            return ss.str();
        }

        // Check if the number has a decimal point
        size_t sep = str.find_first_of('.', start);
        // Has not a decimal point (e.g. 123)
        if (sep == std::string::npos) {
            ss << str.substr(start) << ".0";
        }
        // Has a decimal point
        else {
            if (sep == start) {
                // No integer part (e.g. .123)
                ss << '0';
            }
            // Skip trailing zeros
            size_t end = str.find_last_not_of('0');
            // Has not a fractional part (e.g. 123.)
            if (end == sep) {
                ss << str.substr(start, sep - start) << ".0";
            }
            // Has a fractional part (e.g. 123.456)
            else {
                ss << str.substr(start, end - start + 1);
            }
        }

        return ss.str();
    }

    uint64_t id;

    Decimal(uint64_t _id) : id(_id) { }

    inline bool operator==(const Decimal& rhs) const noexcept {
        return this->id == rhs.id;
    }

    inline bool operator!=(const Decimal& rhs) const noexcept {
        return this->id != rhs.id;
    }

    // Returns the decimal string (normalized)
    std::string get_value_string() const {
        uint64_t decimal_id = id;
        std::stringstream ss;

        // Check sign bit
        if (decimal_id & (1ULL << 55)) {
            ss << '-';
            decimal_id &= ~(1ULL << 55);
        }
        uint64_t sep = decimal_id & 0xF;
        uint64_t number = decimal_id >> 4;
        if (sep) {
            ss << number / (uint64_t)std::pow(10, sep)
               << '.'
               // This is necessary for numbers like 0.0001
               << std::setw(sep)
               << std::setfill('0')
               << number % (uint64_t)std::pow(10, sep);
        } else {
            ss << number
               << '.'
               << '0';
        }
        return ss.str();
    }

    std::string to_string() const {
        std::stringstream ss;

        ss << '"';
        ss << get_value_string();
        ss << '"'
           << "^^<http://www.w3.org/2001/XMLSchema#decimal>";
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Decimal& dt) {
        os << dt.to_string();
        return os;
    }
};