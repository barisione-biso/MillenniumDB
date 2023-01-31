# MillenniumDB: Decimal

## Internal
### Codification
The 56 bits are divided into:
```
[1 sign][51 number][4 separator position]
```
### Details
1. The separator position is the position of the separator from the right (e.g. if the separator is 3, the number could be 0.123, 1234.555, 100.654, etc).
2. If the number has more than 51 bits of representation, we throw an error.

### Comparison
For comparing two decimals (lhs < rhs) the algorithm does:
1. if lhs == rhs, then both decimals are equal.
2. if signs are different, then the decimal with the negative sign is smaller.
3. if separator positions are different, then we move the lowest separator position to the highest separator position. Then the numbers are compared as a regular number.

## External
### Codification
```
[1 bit: sign][7 bits: size][8 bits: exponent][size bytes: digits]
sign: 1 means negative, 0 means positive
size: number of bytes used to represent the digits
exponent: int8_t holding the base 10 exponent
digits: 2 digits per byte (4 bits per digit), little endian, no leading or trailing zeros
```

If there are an uneven number of digits, then the last byte of the digits holds only one digit in the least dignificant 4 bits, ie 0x05,
and the last digits is zero. So for an uneven number of digits one trailing zero is allowed, which is ignored.
The value is equal to `<-1 or 1, depending on sign> * <whole number represented by digits> * 10 ^ exponent`
