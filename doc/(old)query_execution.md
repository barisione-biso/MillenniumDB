

# Query Execution

# Resumen

En carpeta **binding** están los distintos tipos de bindings, todos heredando de la clase Binding. Esto significa que tienen los métodos to_string() y operator[] como mínimo.

- to_string(): Retorna la representación en string de los resultados en caso de que el Binding posea resultados, o arroja error si no debería entregar resultados.

- operator[]: Retorna el o los operadores relevantes.

| Binding | to_string() | operator[](VarId var) |
|:----:|:---:|:---:|
| Materialize Id | Lista de variables y objetos asignados a las variables | Objeto asignado por binding_id[var] |
| Select | Todas las asignaciones entre {} | retorna child_binding[var] |
| Where | Error | Encuentra y retorna el objeto respectivo, agregandolo a la cache de ser necesario |

En carpeta **binding_iter**  se indica cuándo parar o seguir en la iteración de cada uno de los bindings. Todas las clases heredan de BindingIter, por lo que tienen los métodos por defecto  get_binding(), next() y analyze().

- get_binding(): Obtiene el binding del iterador. Mismo comportamiento para todos.
- next(): Obtiene el siguiente binding por el cual iterar.
- analyze(): Muestra cosas en consola para debuggear.


| Binding Iter | next() |
|:----:|:---:|
| Match | Retornará true mientras hayan siguientes elementos. |
| Select | Retornará true mientras no se haya pasado el límite de iteración y mientras hayan siguientes elementos. |
| Where | Retornará true mientras hayan siguientes elementos y los elementos cumplan la condición. |

En la carpeta **binding_id_iter**  están las clases que permite iterar sobr BindingId, por lo que todos heredan de BindingIdIter. Esto significa que tienen los métodos begin(), reset(), next() y analyze()

- begin(): Setea lo necesario para comenzar el iterador sobre los Ids
- reset(): Resetea el iterador según binding id
- next(): Asigna el siguiente binding id por el cual iterar
- analyze(): Muestra información en consola para debuggear.

| Binding Id Iter | begin() | next() | reset() |
|:----:|:---:|:---:|:---:|
| Index Scan | Asigna el conjunto de N rangos de ids buscando por el B+ Tree, y lo asigna al iterador. | Itera hasta que no hayan más elementos en el iterador | Vuelve a encontrar todos los rangos, similar a begin()|
| Index Nested Loop Join | Asigna current_right si es que lhs tiene siguiente. | Resetea lhs y si este tiene siguiente, resetea rhs. | Continúa mientras rhs tenga siguientes, o bien lhs tenga siguientes (a lo que resetea rhs). |



# Investigación

## Binding

### binding_materialize_id.h

Tiene a la clase BindingMaterializeId, que hereda de Binding.

Atributos privados:

- GraphModel (model)
- size_t (binding_size)
- BindingId (binding_id)

Funciones:
- Inicializador, que recibe un GraphModel (model), size_t (binding_size), BindingId (binding_id)
- to_string()
- operator[], que recibe un VarId y retorna un puntero a un GraphObject.
- begin que recibe un BindingId y no retorna nada.

### binding_materialize_id.cc

Funciones:

- Inicializador, que recibe model, binding_size y binding_id.
- to_string, que por cada uno de los bindings agrega la representación de string de la variable a un string, y retorna.
- operator, que retorna el objeto del grafo que representa al binding de var_id.

### binding_select.h

Tiene a la clase BindingSelect que hereda de Binding.

Atributos privados:

- vector de string-VarId (projection vars)
- Binding (child_binding)

Funciones:

- Iniicalizador de BindingSelect, que recibe un vector de string-VarId (projection_vars), un Binding (child_binding)
- to_string()
- operator[], que recibe un VarId y retorna un puntero a un GraphObject.

### binding_select.cc

Funciones:

- Inicializador, que recibe projection_vars, child_binding.
- to_string(), que itera por los resultados, asignando la variable y el valor a cada una de las variables. hasta terminar.
- operator[], que recibe un VarId (var) y retorna child_binding[var].

### binding_where.h

Tiene la clase BindingWhere que hereda de Binding.

Atributos privados:

- GraphModel (model)
- VarId (max_var_id_in_child)
- Binding (child_binding)
- diccionario de VarId-GraphObject (cache)
- diccionario de VarId-(VarId,ObjectId) (property_map)

Funciones:

- Inicializador que recibe un GraphModel (model), Binding (child_binding), VarId (max_var_id_in_child), diccionario VarId-(VarId, ObjectId) (property_map)
- to_string()
- operator[], que recibe un VarId (var_id) y retorna un puntero a un GraphObject.
- clear_cache()

### binding_where.cc

Funciones:

- Inicializador, que recibe model, child_binding, max_var_id_in_child, property_map.
- to_string()
- clear_cache(), que llama a cache.clear()
- operator[], que recibe un var_id. Si este var_id es menor o igual que el máximo, se retorna child_binding[var_id], si no, se intenta encontrar en la caché. Si se encuentra (cache_find != cache.end), se retorna el second, y de caso contrario, se encuentra la var_id en el property_map. se obtiene el var_id y la llave de lo encontrado, se encuentra la variable con child_binding[property_var_id], y se obtiene el value desde get_property_value, insertándolo en la caché. Finalmente se retorna el valor.

## Binding Id Iter

### index_nested_loop_join.h

Tiene el constructor de la clase IndexNestedLoopJoin que hereda de BindingIdIter.

Atributos privados:

- puntero a BindingIdIter (lhs)
- puntero a BindingIdIter (rhs)
- puntero a BindingId (current_left)
- puntero a BindingId (current_right)

Funciones:

- Inicializador que recibe un size_t (binding_size), un puntero único a BindingIdIter (lhs), un puntero único a BindingIdIter (rhs).

- analyze que recibe un int (indent), no retorna nada.
- begin que recibe un BindingId, retorna un BindingId
- reset que no retorna nada
- next que retorna un booleano

### index_nested_loop_join.cc

Funciones:

- Tiene la inicialización que recibe un binding_size, lhs y rhs. Con esto genera un BindingIdIter de tamaño binding_size, un lhs y un rhs.

- begin:  Comienza con el inicio de lhs. Si es que hay un siguiente de lhs, comienza con el inicio de rhs. Retorna my_binding.

- reset: Si lhs tiene un siguiente, resetea rhs.

- next: Continúa iterando mientras que rhs tenga un siguiente, o bien, lhs tenga un siguiente (en tal caso reseta rhs). En estas iteraciones añade lo que está a la izquierda y a la derecha a my_binding.

- analyze: Función para bebuggear?

### index_scan.h

Tiene el constructor de la clase IndexScan que hereda de BindingIdIter, con tipos genéricos de tamaño N.

Atributos privados:
- BPlusTree (bpt)
- BptIter(N) (it)
- BindingId* (my_input)
- Array de punteros a N ScanRange (ranges)
- uint_fast32_t results_found [0]
- uint_fast32_t bpt_searches [0]

Funciones:
- Inicializador que recibe un  std:size_t (binding_size) , BPlusTree<N> (bpt), y un array de punteros de N ScanRanges (ranges)
- analyze que recibe un int (indent) y no retorna nada.
- begin que recibe un BindingId (input)
- reset (no recibe nada ni retorna nada)
- next (no recibe nada) que retorna un booleano.

### index_scan.cc

Tiene 3 clases genéricas con N de tamaño 2, 3 y 4. Para cada template:

- Tiene la función de inicialización que recibe binding_size, bpt, y ranges. Con esto genera un BindingIdIter, bpt y un move(ranges)

Funciones:
- begin: Recibe un BindingId input. Declara N min_ids y N max_ids. Itera por todo los ranges y obtiene el mínimo y máximo de cada range dado el input. Genera un iterador con bpt.get_range del Record del move(min_ids) y move(max_ids), y retorna my_binding (ver binding_id_iter.h).

- next: Itera por el iterador "it", y mientras sea distinto de nulo, agrega todo lo que esté en my_input. Después itera hasta N, intentando asignar a ranges el siguiente ID del objeto con try_assign(Ver scan_range.h)

## Binding Iter

### match.h

Contiene la clase Match que hereda de BindingIdIter.

Atributos privados:
- un objeto de tipo GraphModel (model)
- un puntero a un BindingIdIter (root)
- un BindingId (input)
- un putero a un BindingId (binding_id_root)
- un puntero a BindingMaterializeId (my_binding)

Funciones:

- Inicializador que recibe un GraphModel (model), un puntero a un BindingIdIter (root), un tamaño (binding_size)
- get_binding() que retorna un Binding
- next() que retorna un booleano
- analyze(indent) para debuggear.

### match.cc

Funciones:

Inicializador que recibe model, root, input y my_binding. Con este último crea un BindingMaterializeId que recibe el model, binding_size, y root->begin(input). (ver binding_materialize_id.h)

- get_binding(), que retorna my_binding

- next(), que retorna el siguiente

- analyze(int indent), para debuggear.

### select.h

Tiene la clase Select que hereda de BindingIdIter.

Atributos privados:

- Un puntero a BindingIdIter (child_iter)
- un uint_fast32_t (limit)
- un uint_fast32_t (count)
- un BindingSelect (my_binding). (ver binding_select.h)

Funciones:
- Inicializador que recibe un puntero a BindingIter (child_iter), un diccionario de string-VarIds (projection_vars), y un uint_fast32_t (limit)
- get_binding() que retorna un Binding
- next() que retorna un booleano
- analyze() para debuggear.


### select.cc

Funciones:

- Inicializador que recibe _child_iter, projection_vars y limit y genera un child_iter, limit y my_binding, el que es un BindingSelect inicializado con projection_vars, y child_iter->get_binding().

- get_binding() que retorna my_binding

- next(), que si el limit es distinto de 0 y count es mayor a limit retorna falso, y de lo contrario, si a child_iter le queda por iterar, itera al siguiente aumentando el count en uno. De lo contrario retorna false.

- analyze para debuggear.

### where.h

Tiene la clase Where que hereda de BindingIdIter.

Atributos privados:

- Un objeto de tipo GraphModel (model)
- un puntero a un BindingIdIter (child_iter)
- un puntero a un Condition (condition)
- un uint64_t (results, inicialmente 0)
- un BindingWhere (my_binding)

Funciones:

- Inicializador que recibe un GraphModel (model), un puntero a un BindingIter (iter), un Condition (condition), un size_t child_binding_size, y un diccionario de variables, con variables y objetos (property_map)

- get_binding() que retorna un Binding
- next() que retorna un booleano
- analyze() para debuggear

### where.cc

Funciones:

- Inicializador que recibe un model, child_iter, condition, child_binding_size y property map. Con esto genera un model, child_iter, condition, y my_binding (inicializandollo con BindingWhere(model, child_iter->get_binding(), VarId (child_binding_size  - 1), propertymap)).
- get_binding(), que retorna el binding.
- next(), que retorna un booleano. Mientras existan siguientes, se limpia el cache del binding, y si al evaluar la condición sobre el binding se cumple, se agrega a resultados y retorna true. Cuando no queda más por iterar retorna false.
- analyze() para debuggear.

## Extras:

move: Forma eficiente y rápida de manejar memoria. Asigna y libera al mismo tiempo. La semántica que se le quiere dar es que uno efectivamente está moviendo.

Después del move al unique pointer, es nulo.

0. Leer paper y pensar un poco como resolver
1. Crear BindingIdIter correspondiente al optional
2. Enchufar lo creado en (1) para la proyeccion de properties (que salga en el select pero no en el match)

```
SELECT ?x.age
MATCH (?x :Person)
```

NodeLabel(?x, Person) INLJ Property(?x, "age", ?x.age)
NodeLabel(?x, Person) OPTIONAL Property(?x, "age", ?x.age)

3. Permitir en el where `?x.age != null`
4. Agregar a la gramática y modificar el optimizador para que genere el plan esperado.
```
SELECT *
MATCH <Pattern1>
OPTIONAL { <Pattern2> }
```
5. 
```
SELECT *
MATCH <Pattern1></Pattern1>
OPTIONAL { <Pattern2> }
OPTIONAL { <Pattern3> }
```

```
SELECT *
MATCH <Pattern1>
OPTIONAL {
    <Pattern2>
    OPTIONAL { <Pattern3> }
}
```
6. ¿Casos no well designed?

SELECT ?x.age, ?c
MATCH (?x :Person)
OPTIONAL {(?x)-[:BORN_AT]->(:City ?c)}

SELECT *
MATCH (?x :Person)
    OPTIONAL (?x)-[:Knows]->(?y)

?x  | ?y
Q1  | Q2
Q1  | Q3
Q1  | Q4
Q2  | null
