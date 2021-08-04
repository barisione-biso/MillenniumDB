# Query language specification

## MATCH statement

### OPTIONAL

## SELECT statement
A **SELECT statement** specifies the output of the query. It has the keyword `SELECT` followed by a list of variables or variable properties.
For example:
```
SELECT ?x, ?y, ?z.name, ?u.age
```

- A **SELECT statement** that mention a variable not used in the **MATCH statement** is not valid.

- If the binding of a variable does not have the specified property, the returned value will be `null`.

- A special case is `SELECT *` which means you want to select all the variables mentioned in the MATCH statement.

## WHERE statement