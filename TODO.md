- Cosas para discutor prox reunion:
    - Sintaxis del @
    - Conexiones directas a valores
    - Json como posible valores
        - como hashear json?
    - Esquema de tipos para los properties (values)
    - Estadisticas de out e in degree por Nodos más conectados
        - usar promedio para el restante
        - usar sampling?

ACORDARSE:
    - El problema de la semantica con neo4j al hacer MATCH (?x) con el type de los edges
      se soluciona simplemente si al hacer el import no se guarda en la tabla el tipo del edge

- Estadisticas de out e in degree por Nodos más conectados
    - usar promedio para el restante
    - usar sampling?
- Cambios si hacemos nuestra propuesta de nuevo modelo basado en quads:
    - Eliminar lo relacionado con edges EdgeLabel, EdgeProperty, etc
    - Crear nueva "tabla" (E)SPO
    - Import nuevo:
        - Logica de import debería pasar a base (estructuras de parser)
            - Escritura de índices quedaría en relational model
        - Nodos y conexiones pueden estar en un solo archivo
        - Ya no se hace mapping de id de archivo a id interno, el IRI será el ID.
            - Una conexión se puede definir con nodos no declarados, el ID del nodo será creado
              en ese momento.
        - ¿Intentar paralelizar?
    - Crear operador filtro que use la tabla (E)SPO
    - Actualizar query optimizer
        - Cuando La conexión (SPOE) tiene asignado el E y algo más usar filtro en vez de
          index nested loop join

- Filtros con Id, pasar hacia Join

- Crear catálogo después que se importo el grafo (basta una pasada lineal por KVE y KVN)
    - Problema: no estan agrupados por grafo (si al hacer bulk import)
        - Se podría crear un ordered file y que queden ordenados por Grafo > Key > Value.

- Proyeccion de properties en manual plan

- optimizar BPT?:
    - usar solo 2 (1?) bytes para key_count
____________________________________________________________________
- Soportar cuasi-esquema en el catálogo (si son menos de X puedo tener distribuciones)
    - O tal vez agregar la selectividad del peor caso para una key (y si no existe se asume que es total)

- Support new formats:
    - datetime
    - long
    - decimal

- Escaped characters in grammar
    - make common grammar in base

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
