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

- Cambiar librería para parser/grammar?
    - X3 no maneja correctamente casos donde se debe hacer backtracking

- Estadisticas de out e in degree para Nodos más conectados
    - usar promedio para el restante
    - usar sampling?

- Filtros con Id, pasar hacia Join

- Rehacer manual plan

- optimizar BPT?:
    - usar solo 2 (1?) bytes para key_count
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

- Presentar plan pointer swizzling (cuántos punteros como máximo permitir?)
- Blob para objetos muy grandes, guardar fuera de object file, en su propio archivo.
- Buffer manager:
    - allocating more space if necesary ?
- Bulk Import:
    - use disk (ordered file) if importing more than X nodes/edges?
- Catalog:
    - Stop if having key for node/edge more than X different keys?
- Usar BufferManager para OrderedFiles ?
- Al buscar property el value solo puede ser del valor dado. ¿Eliminar posibilidad de valores numéricos en match? ¿Eliminar solo float?
- ver si string B-Tree puede ser útil https://algo2.iti.kit.edu/download/2014_-_Bachelor-Thesis_Fellipe_Lima.pdf
