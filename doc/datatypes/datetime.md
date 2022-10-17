# MillenniumDB: Datetime

## Codification
If the year is greater than 9999, then we store just the year divided by ten thousand, losing precision. The 56 bits are divided into:

```
[1 sign][1 precision][54 ten thousand years]
```

In other case, the 56 bits are divided into:

```
[1 sign][1 precision][2 empty][14 year][4 month][5 day][5 hour][6 minute][6 second][1 timezone sign][5 timezone hour][6 timezone minute]
```

## Details
1. If the date format has milliseconds, they are ignored.
2. The month and year can't be zero.
3. We do not check if the day is valid according the month and/or year.
4. Currently, if the dates are the same, but the timezones are different, due the current implementation the order would be `NEGATIVE TIMEZONE > UTC > POSITIVE TIMEZONE`.

## Comparison
For comparing two dates (lhs < rhs) the algorithm does:
1. if lhs == rhs, then both dates are equal.
2. if signs are different, then the date with the negative sign is smaller.
3. if precisions are different, then the date with the higher precision is smaller.
4. else, the date with the smaller value is smaller.
