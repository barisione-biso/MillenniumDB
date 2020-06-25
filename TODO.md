+ No podemos hacer el mismo plan que neo4J, ya que hay operaciones que al traspasarlas necesitan más
  de una operación (ej: expandir conexiones por un label)
    + Gran cantidad de resultados intermedios se produce al expandir las conexiones y luego buscar si tienen
    el label
    + Si saco el label en la consulta de cypher usa NodeHashJoin, no expande innecesariamente

- Ver consultas de property paths y ver como se comporta Neo4j
    + tratar de encontrar algunas donde neo4j funcione mal

- Ver como importar Jena solucionando el problema del sort

- Agregar test para recorrer todos los strings y checkear que estan bien en el hash

- terminar de implementar extendable hashing
    - caso split local
    - caso split global
    - usar solo 6 bytes para puntero a string file, dejar hashes juntos
    - no usar puntero para count (escribir en destructor, sin hace make_dirty, cosa de que solo se escriba si se modifico)

- Tratar de optimizar BPT:
    - reempolazar BPTParams por templates
    - reemplazar Record por template
    - eliminar caso del value (o usarlo y ver cómo adaptar el bulk import para que quede bien)
        - Puede mejorar NK|V, KN|V, EK|V, KE|V y E|FT
    - reemplazar punteros a counts por counts reales. Actualizar al destruir

- Crear catálogo después que se importo el grafo (basta una pasada lineal por KVE y KVN)
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

- LoopNode en catalogo
- Catalogo:
    - guardar tiempos de busqueda
    - al terminar import y opcionalmente cuando se quiera

- Estimación de tuplas de output y de costo
    - Materializar resultados que se usarán varias veces?
    - Selinger
- Reemplazar el B+Tree de hash=>Id por una estructura de datos para diccionarios.


- cambiar asserts por macro que pueda desactivarlos
    - definir funcion en módulo test
    - por defecto hace el assert tanto en RELEASE como DEBUG
    - debería ser fácil quitar el assert para RELEASE
- Añadir Databases de pruebas como archivo comprimido al repositorio

- rename en file mananager no edita el buffer_manager (FileId se re asocia).
    - checkear que nuevo nombre no existe en el disco ni en el file manager.
- remove en file manager limpia el buffer manager
    - checkear pin == 0 para las páginas a ser eliminadas

- automatizar test con script bash
- Reescritura de consulta (igualdades de where hacia match)
- Ampliar el diagrama para explicar el prefijo de grafos
- Hacer más tests de cosas en conjunto (ej: label+property)
- hacer diagramas de flujo de las siguientes operaciones:
    - bplustree search

- Usar Buckets pasando por buffer manager con la estructura de hash que guarda el id de un hash md5
- permitir añadir datos a grafo existente -> otro ejecutable?
- permitir cargar varios grafos al mismo tiempo

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
