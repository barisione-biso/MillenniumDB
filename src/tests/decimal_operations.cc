#include "base/query/sparql/decimal.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using std::cout;
using std::pair;
using std::string;
using std::vector;

using namespace std::string_literals;

typedef int TestFunction();

int string_tests() {
    vector<pair<string, string>> tests {
        { "1"s, "1"s },
        { "1.00"s, "1"s },
        { "001"s, "1"s },
        { "001.00"s, "1"s },
        { "12.12"s, "12.12"s },
        { "0012.12"s, "12.12"s },
        { "12.1200"s, "12.12"s },
        { "0012.1200"s, "12.12"s },
        { "001200.001200"s, "1200.0012"s },
        { "0000.001200"s, "0.0012"s },
        { "001200.0000"s, "1200"s },
        { "-1"s, "-1"s },
        { "-1.00"s, "-1"s },
        { "-001"s, "-1"s },
        { "-001.00"s, "-1"s },
        { "-12.12"s, "-12.12"s },
        { "-0012.12"s, "-12.12"s },
        { "-12.1200"s, "-12.12"s },
        { "-0012.1200"s, "-12.12"s },
        { "-001200.001200"s, "-1200.0012"s },
        { "-0000.001200"s, "-0.0012"s },
        { "-001200.0000"s, "-1200"s },
        { "-0"s, "0"s },
        { "-0.0"s, "0"s },
        { "-.0"s, "0"s },
        { "-00.00"s, "0"s },
    };

    cout << "string_tests\n";
    for (auto& [in, want] : tests) {
        Decimal dec(in);
        auto    got = dec.to_string();
        if (got != want) {
            cout << "In(" << in << "): got " << got << ", want " << want << "\n";
            return 1;
        }
    }
    return 0;
}

int int64_t_tests() {
    vector<pair<int64_t, string>> tests {
        { 1, "1"s },   { 978, "978"s },   { 1000, "1000"s },   { 978000, "978000"s },
        { -1, "-1"s }, { -978, "-978"s }, { -1000, "-1000"s }, { -978000, "-978000"s },
    };

    cout << "int64_t_tests\n";
    for (auto& [in, want] : tests) {
        Decimal dec(in);
        auto    got = dec.to_string();
        if (got != want) {
            cout << "In(" << in << "): got " << got << ", want " << want << "\n";
            return 1;
        }
    }
    return 0;
}

int double_tests() {
    vector<pair<string, double>> tests {
        { "1"s, 1.0 },
        { "978"s, 978.0 },
        { "1000"s, 1000.0 },
        { "978000"s, 978000.0 },
        { "-1"s, -1.0 },
        { "-978"s, -978.0 },
        { "-1000"s, -1000.0 },
        { "-978000"s, -978000.0 },
        { "0.5"s, 0.5 },
        { "0.000123"s, 0.000123 },
        { "-0.5"s, -0.5 },
        { "-0.000123"s, -0.000123 },
        { "005500.005500", 5500.0055 },
        { "550000000", 550000000.0 },
        { "0.00000000009568", 0.00000000009568 },
    };

    cout << "double_tests\n";
    for (auto& [in, want] : tests) {
        Decimal dec(in);
        auto    got = dec.to_double();
        if (std::abs(got - want) > 1e-19) {
            cout << "In(" << in << "): got " << got << ", want " << want << "\n";
            return 1;
        }
    }
    return 0;
}

std::ostream& operator<<(std::ostream& os, const vector<uint8_t>& vuc) {
    auto old_flags = os.flags();
    os.setf(std::ios::hex, std::ios::basefield);

    os << "[ ";
    for (auto& uc : vuc) {
        os << static_cast<int>(uc) << " ";
    }
    os << "]";

    os.setf(old_flags);
    return os;
}

int bytes_tests() {
    vector<pair<string, vector<uint8_t>>> tests {
        { "1", { 1, 0, 1 } },
        { "2", { 1, 0, 2 } },
        { "-5", { 0x81, 0, 5 } },
        { "-5.5", { 0x81, 0b1111'1111, 0x55 } },
        { "-0.55", { 0x81, 0b1111'1110, 0x55 } },
        { "-0.555", { 0x82, 0b1111'1101, 0x55, 0x05 } },
        { "-0.000555", { 0x82, 0b1111'1010, 0x55, 0x05 } },
    };

    cout << "bytes_tests\n";
    for (auto& [in, want] : tests) {
        Decimal dec(in);
        auto    got = dec.to_bytes();
        if (got != want) {
            cout << "In(" << in << "): got " << got << ", want " << want << "\n";
            return 1;
        }
    }
    return 0;
}

int roundtrip_tests() {
    vector<pair<string, string>> tests {
        { "1"s, "1"s },
        { "1.00"s, "1"s },
        { "001"s, "1"s },
        { "001.00"s, "1"s },
        { "12.12"s, "12.12"s },
        { "0012.12"s, "12.12"s },
        { "12.1200"s, "12.12"s },
        { "0012.1200"s, "12.12"s },
        { "001200.001200"s, "1200.0012"s },
        { "0000.001200"s, "0.0012"s },
        { "001200.0000"s, "1200"s },
        { "-1"s, "-1"s },
        { "-1.00"s, "-1"s },
        { "-001"s, "-1"s },
        { "-001.00"s, "-1"s },
        { "-12.12"s, "-12.12"s },
        { "-0012.12"s, "-12.12"s },
        { "-12.1200"s, "-12.12"s },
        { "-0012.1200"s, "-12.12"s },
        { "-001200.001200"s, "-1200.0012"s },
        { "-0000.001200"s, "-0.0012"s },
        { "-001200.0000"s, "-1200"s },
        { "-0"s, "0"s },
        { "-0.0"s, "0"s },
        { "-.0"s, "0"s },
        { "-00.00"s, "0"s },
    };

    cout << "roundtrip_tests\n";
    for (auto& [in, want] : tests) {
        Decimal dec1(in);
        auto    bytes = dec1.to_bytes();

        Decimal dec2(bytes);
        auto    ex_str = dec2.to_external();

        Decimal dec3;
        dec3.from_external(ex_str);
        auto got = dec3.to_string();

        if (got != want) {
            cout << "In(" << in << "): got " << got << ", want " << want << "\n";
            return 1;
        }
    }
    return 0;
}

int less_than_tests() {
    vector<std::tuple<string, string, bool>> tests {
        make_tuple("1"s, "1"s, false),
        make_tuple("1"s, "0"s, false),
        make_tuple("1"s, "-1"s, false),
        make_tuple("1"s, "0.1"s, false),
        make_tuple("-1"s, "1"s, true),
        make_tuple("0"s, "1"s, true),
        make_tuple("0.1"s, "1"s, true),
        make_tuple("33"s, "333"s, true),
        make_tuple("333"s, "33"s, false),
        make_tuple("33.33"s, "33.33"s, false),
        make_tuple("33.033"s, "33.33"s, true),
        make_tuple("33.33"s, "33.033"s, false),
        make_tuple("100"s, "33"s, false),
        make_tuple("1"s, "33"s, true),
    };

    cout << "less_than_tests\n";
    for (auto& [lhs, rhs, want] : tests) {
        Decimal lhs_d(lhs);
        Decimal rhs_d(rhs);

        auto got = lhs_d < rhs_d;
        if (got != want) {
            cout << "(" << lhs_d.to_string() << " < " << rhs_d.to_string() << ") got" << got << ", want " << want << "\n";
            return 1;
        }
    }
    return 0;
}

int greater_than_tests() {
    vector<std::tuple<string, string, bool>> tests {
        make_tuple("1"s, "1"s, false),
        make_tuple("1"s, "0"s, true),
        make_tuple("1"s, "-1"s, true),
        make_tuple("1"s, "0.1"s, true),
        make_tuple("-1"s, "1"s, false),
        make_tuple("0"s, "1"s, false),
        make_tuple("0.1"s, "1"s, false),
        make_tuple("33"s, "333"s, false),
        make_tuple("333"s, "33"s, true),
        make_tuple("33.33"s, "33.33"s, false),
        make_tuple("33.033"s, "33.33"s, false),
        make_tuple("33.33"s, "33.033"s, true),
        make_tuple("100"s, "33"s, true),
        make_tuple("1"s, "33"s, false),
    };

    cout << "less_than_tests\n";
    for (auto& [lhs, rhs, want] : tests) {
        Decimal lhs_d(lhs);
        Decimal rhs_d(rhs);

        auto got = lhs_d > rhs_d;
        if (got != want) {
            cout << "(" << lhs_d.to_string() << " > " << rhs_d.to_string() << ") got " << got << ", want " << want << "\n";
            return 1;
        }
    }
    return 0;
}

int multiplication_tests() {
    vector<std::tuple<string, string, string>> tests {
        std::make_tuple("0", "0", "0"),
        std::make_tuple("0", "1", "0"),
        std::make_tuple("1", "0", "0"),
        std::make_tuple("1", "1", "1"),
        std::make_tuple("-0", "-0", "0"),
        std::make_tuple("-0", "-1", "0"),
        std::make_tuple("-1", "-0", "0"),
        std::make_tuple("-1", "-1", "1"),
        std::make_tuple("-0", "0", "0"),
        std::make_tuple("-0", "1", "0"),
        std::make_tuple("1", "-0", "0"),
        std::make_tuple("1", "-1", "-1"),
        std::make_tuple("10", "100", "1000"),
        std::make_tuple("100", "10", "1000"),
        std::make_tuple("0.1", "1", "0.1"),
        std::make_tuple("0.01", "100", "1"),
        std::make_tuple("0.001", "0.0001", "0.0000001"),
        std::make_tuple("5689", "437217", "2487327513"),
        std::make_tuple("437217", "5689", "2487327513"),
        std::make_tuple("450.00357", "600.0008642", "270002.530893085194"),
        std::make_tuple("45021130.00423234324000357", "623423400234234.0043223408642", "28067225949626021372658.070277771172352968058764885194"),
        std::make_tuple("45021130.00423234324000357", "-623423400234234.0043223408642", "-28067225949626021372658.070277771172352968058764885194"),
        std::make_tuple("-45021130.00423234324000357", "623423400234234.0043223408642", "-28067225949626021372658.070277771172352968058764885194"),
        std::make_tuple("-45021130.00423234324000357", "-623423400234234.0043223408642", "28067225949626021372658.070277771172352968058764885194"),
    };

    cout << "multiplication_tests\n";
    for (auto& [lhs, rhs, want] : tests) {
        auto lhs_d = Decimal(lhs);
        auto rhs_d = Decimal(rhs);
        auto want_d = Decimal(want);

        auto got = lhs_d * rhs_d;
        if (got != want_d) {
            cout << "(" << lhs << " * " << rhs << ") got " << got << ", want " << want << "\n";
            return 1;
        }
    }
    return 0;
}

int quotient_remainder_tests() {
    vector<std::tuple<string, string, uint8_t, string>> tests {
        std::make_tuple("9", "4", 2, "1"),
        std::make_tuple("6", "3", 2, "0"),
        std::make_tuple("7321", "2867", 2, "1587"),
        std::make_tuple("73231231", "73231231", 1, "0"),
        std::make_tuple("73231231", "43231231", 1, "30000000"),
        std::make_tuple("25", "5", 5, "0"),
        std::make_tuple("25", "6", 4, "1"),
        std::make_tuple("250", "50", 5, "0"),
        std::make_tuple("250", "60", 4, "10"),
    };

    cout << "quotient_remainder_tests\n";
    for (auto& [dividend, divisor, quotient, remainder] : tests) {
        Decimal dividend_d(dividend);
        Decimal divisor_d(divisor);
        Decimal remainder_d(remainder);

        auto [got_q, got_r] = Decimal::quotient_remainder(dividend_d, divisor_d);

        if (got_q != quotient || got_r != remainder_d) {
            cout << "(" << dividend_d << " / " << divisor_d << ") got " << std::to_string(got_q) << ", " << got_r << ", want " << std::to_string(quotient) << ", " << remainder_d << "\n";
            return 1;
        }
    }
    return 0;
}

int division_tests() {
    Decimal::MAX_SIGNIFICANT_FIGURES = 100;

    vector<std::tuple<string, string, string>> tests {
        std::make_tuple("8", "4", "2"),
        std::make_tuple("8", "8", "1"),
        std::make_tuple("12", "4", "3"),
        std::make_tuple("100", "25", "4"),
        std::make_tuple("888", "2", "444"),
        std::make_tuple("13123242", "99", "132558"),
        std::make_tuple("1000000", "1000", "1000"),
        std::make_tuple("3", "5", "0.6"),
        std::make_tuple("254262", "46500", "5.468"),
        std::make_tuple("254262", "0.005468", "46500000"),
        std::make_tuple("254262", "46500000", "0.005468"),
        std::make_tuple("0.7125", "95", "0.0075"),
        std::make_tuple("0.00072628364", "9574", "0.00000007586"),
        std::make_tuple("1", "3", "0.3333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333"),
        std::make_tuple("100", "3", "33.33333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333"),
        std::make_tuple("0.001", "3", "0.0003333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333"),
        std::make_tuple("0.0011", "0.0333", "0.03303303303303303303303303303303303303303303303303303303303303303303303303303303303303303303303303303"),
        std::make_tuple("110.00111", "33300.0003333", "0.003303336603573510811680145539549774524566370899760617991284865570713342500938226559678350980756925469"),
        std::make_tuple("9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999", "1", "9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999"),
        std::make_tuple("99999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999994", "1", "99999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999990"),
        std::make_tuple("99999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999995", "1", "100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
        std::make_tuple("0.2", "0.1", "2"),
        std::make_tuple("-0.6", "0.3", "-2"),
        std::make_tuple("-0.04", "-0.008", "5"),
    };

    cout << "division_tests\n";
    for (auto& [dividend, divisor, want] : tests) {
        Decimal dividend_d(dividend);
        Decimal divisor_d(divisor);
        Decimal want_d(want);

        auto got = dividend_d / divisor_d;

        if (got != want_d) {
            cout << "(" << dividend_d << " / " << divisor_d << ") got " << got << ", want " << want_d << "\n";
            return 1;
        }
    }
    return 0;
}

int main() {
    cout.precision(20);

    vector<TestFunction*> tests;

    tests.push_back(&string_tests);
    tests.push_back(&int64_t_tests);
    tests.push_back(&double_tests);
    tests.push_back(&bytes_tests);
    tests.push_back(&roundtrip_tests);
    tests.push_back(&less_than_tests);
    tests.push_back(&greater_than_tests);
    tests.push_back(&multiplication_tests);
    tests.push_back(&quotient_remainder_tests);
    tests.push_back(&division_tests);

    for (auto& test_func : tests) {
        if (test_func()) {
            return 1;
        }
    }
    return 0;
}
