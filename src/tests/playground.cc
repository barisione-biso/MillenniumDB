#include "base/graph_object/decimal.h"

#include <iostream>

int main() {
    auto d1 = Decimal::get_decimal_id("1.");
    std::cout << Decimal(d1) << std::endl;
    return 0;
}