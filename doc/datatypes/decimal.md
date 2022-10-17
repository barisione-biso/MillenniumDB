# MillenniumDB: Decimal

## Codification
The 56 bits are divided into:
```
[1 sign][51 number][4 separator position]
```

## Details
1. The separator position is the position of the separator from the right (e.g. if the separator is 3, the number could be 0.123, 1234.555, 100.654, etc).
2. If the number has more than 51 bits of representation, we throw an error.

## Comparison
For comparing two decimals (lhs < rhs) the algorithm does:
1. if lhs == rhs, then both decimals are equal.
2. if signs are different, then the decimal with the negative sign is smaller.
3. if separator positions are different, then we move the lowest separator position to the highest separator position. Then the numbers are compared as a regular number.
