SELECT ?y
MATCH (?y)
OPTIONAL { (?y)->(?x {age:65}) }
OPTIONAL { (?y)->(?x {age:42}) }

P1 OPT (P2 OPT P3)
P1 OPT P2 OPT P3
((P1 OPT P2) OPT P3)

SELECT *
MATCH (?x :Person)
OPTIONAL {(?x {age:65})}

SELECT *
MATCH (?x :Person)
OPTIONAL {(?x {age:65})->(?y)}