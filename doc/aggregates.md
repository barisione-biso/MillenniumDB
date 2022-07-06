# Aggregates

## Aggregation Functions
The available aggregate functions are:
- `COUNT(*)`: count all bindings.
- `COUNT(DISTINCT *)`: count distinct bindings (a binding is distinct form other if at least one of its declared variables is different, anonymous variables are not considered).
- `COUNT(?x)`: count bindings where `?x` is not `NULL`.
- `COUNT(DISTINCT ?x)` count bindings where `?x` is not `NULL`.
- `AVG(?x)`: average of values of type `int` and `float`, others are ignored. Returns `NULL` if no values were considered.
- `MAX(?x)`: maximum of values of type `int` and `float`, others are ignored. Returns `NULL` if no values were considered.
- `MIN(?x)`: minimum of values of type `int` and `float`, others are ignored. Returns `NULL` if no values were considered.
- `SUM(?x)`: sum of values of type `int` and `float`, others are ignored. Returns `0` if no values were considered.

All functions that apply to `?x` may apply to `?x.key` as well. If `?x` is `NULL` `?x.key` will be `NULL` as well.

## Types of statements that may use aggregates
All the following types of queries must have a `MATCH` statement, and may have `SET` or `WHERE` statements.

Note: *iff*: if and only if.

1. `RETURN` with aggregates:

    Example:
    ```
    MATCH (?x)->(?y)
    RETURN AVG(?x.age), MAX(?x.age)
    ```
    Rules:
    - If there is one aggregate function in the return items, all return items must be aggregates.
        - `RETURN AVG(?x.age), ?y` breaks this rule.

2. `GROUP BY` + `RETURN`

    Example:
    ```
    MATCH (?x)->(?y)->(?z)
    GROUP BY ?x, ?y.name
    RETURN ?x, ?x.name, ?y.name, COUNT(?z), MAX(?z.age)
    ```

    Rules:
    - A variable and its property or two different properties of the same variable cannot appear at the same time in the `GROUP BY`
        - `GROUP BY ?x, ?x.key` breaks this rule.
        - `GROUP BY ?x.key1, ?x.key2` breaks this rule.

    - A return item can be a variable (`?x`) *iff* the variable (`?x`) is declared in the `GROUP BY`.
        - `GROUP BY ?x.key RETURN ?x` breaks this rule.

    - A return item can be a variable property (`?x.key`) *iff* the variable (`?x`) or the same variable property (`?x.key`) is declared in the `GROUP BY`.
        - `GROUP BY ?x.key1 RETURN ?x.key2` breaks this rule.

    - A return item can be an aggregate of a variable (`F(?x)`) *iff* the variable is **NOT** declared in the `GROUP BY`.
        - `GROUP BY ?x RETURN AVG(?x)` breaks this rule.
        - `GROUP BY ?x.key RETURN AVG(?x)` **DOES NOT** break this rule.

    - A return item can be an aggregate of a variable property (`F(?x.key)`) *iff* the same variable property (`?x.key`) is NOT declared in the `GROUP BY`.
        - `GROUP BY ?x.key RETURN AVG(?x.key)` breaks this rule.
        - `GROUP BY ?x.key1 RETURN AVG(?x.key2)` **DOES NOT** break this rule.

3. `GROUP BY` + `ORDER BY` + `RETURN`
    Example:
    ```
    MATCH (?x)->(?y)->(?z)
    GROUP BY ?x, ?y.name
    ORDER BY ?x, ?y.name
    RETURN ?x, ?x.name, ?y.name, COUNT(?z), MAX(?z.age)
    ```
    Rules:
    - All the rules from `GROUP BY` + `RETURN` still apply.
    - Rules from ruteurn items applies to order by items.

`ORDER BY` (with or without aggregates) + `RETURN` with aggregates is incompatible

Examples:
```
MATCH (?x)->(?y)
ORDER BY ?x
RETURN AVG(?x.age), MAX(?x.age)
```
```
MATCH (?x)->(?y)
ORDER BY AVG(?x.age)
RETURN AVG(?x.age), MAX(?x.age)
```