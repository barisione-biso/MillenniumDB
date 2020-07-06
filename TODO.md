- Tratar de optimizar BPT:
    - usar solo 2 (1?) bytes para key_count

- Agregar quad Edge+Label
    - modificar Query Optimizer para incluir el quad

- Pensar como dar el plan de la consulta sin tener que modificar el codigo y recompilar
    - nuevo ejecutable recibe directo el join plan

- Crear catálogo después que se importo el grafo (basta una pasada lineal por KVE y KVN)
    - LoopNode en catalogo
    - guardar total y distinct keys
    - por cada key guardar total values y distict values
    - Problema: no estan agrupados por grafo
        - Se podría crear un ordered file y que queden ordenados por Grafo > Key > Value.
        - Si se esta importando un solo grafo se puede hacer un ordered file solo con Key > Value

- BUG:
    // El where funciona mal cuando ?p1.id y ?p2.id no estan en el select
    // debería poder expresar ?p1 != ?p2
    SELECT *
    MATCH (?p1 :Person)-[:STUDY_AT]->(?u :Organisation {type:"university" age:18})<-[:STUDY_AT]-(?p2 :Person)
    WHERE ?p1.id != ?p2.id

- Pensar que indices nuevos necesitamos (listas ligadas + quad)
____________________________________________________________________
- Soportar cuasi-esquema en el catálogo (si son menos de X puedo tener distribuciones)
    - O tal vez agregar la selectividad del peor caso para una key (y si no existe se asume que es total)

- Support new formats:
    - datetime
    - long
    - decimal

- Operaciones que faltan para Short Reads:
    - Order By
    - Aliases (SELECT X as Y)
    - edge neo4j: *0..
    - Optional Match
    - COALESCE / CASE exists(attr)
    - edges sin dirección (o unión)

- Catalogo:
    - guardar tiempos de busqueda
    - al terminar import y opcionalmente cuando se quiera

- Añadir Databases de pruebas como archivo comprimido al repositorio

- automatizar test con script bash
- Reescritura de consulta (igualdades de where hacia match)
- Ampliar el diagrama para explicar el prefijo de grafos
- Hacer más tests de cosas en conjunto (ej: label+property)
- hacer diagramas de flujo de las siguientes operaciones:
    - bplustree search

- Importar varios grafos en el mismo bulk_import
- Presentar plan pointer swizzling (cuántos punteros como máximo permitir?)
- Blob para objetos muy grandes, guardar fuera de object file, en su propio archivo.
- Buffer manager:
    - allocating more space if necesary ?
- Bulk Import:
    - use disk (ordered file) if importing more than X nodes/edges?
- Catalog:
    - Stop if having key for node/edge more than X different keys.
- Usar BufferManager para OrderedFiles ?
- Al buscar property el value solo puede ser del valor dado. ¿Eliminar posibilidad de valores numéricos en match? ¿Eliminar solo float?
- ver si string B-Tree puede ser útil https://algo2.iti.kit.edu/download/2014_-_Bachelor-Thesis_Fellipe_Lima.pdf
