- Hacer documento explicando modelo de datos para wikidata en property graphs

- Revisar RDF*
    - al final es lo mismo que standard reification, solo cambia la sintaxis
    - Al parecer en GraphDB hacen algo más (porque pesa bastante menos que standard reification)

- Revisar GQL
    - Nuestro modelo puede aplicarse cuando edge_label_flag indica
      que todas los edges deben tener 1 Label.
    - Podríamos tener distintos índixes y query optimizer dependiendo de los
      flags del grafo
    - La idea de que puedan salir conexiones desde un edge entra en confilcto
      con la idea de tener tipos de edges/nodos. Aunque solo para el caso del
      tipo de nodos conectados, el tipo podría adaptarse para soportar
      properties y labels.

- Filtros con Id, pasar hacia Join

- Crear catálogo después que se importo el grafo (basta una pasada lineal por KVE y KVN)
    - Problema: no estan agrupados por grafo (si al hacer bulk import)
        - Se podría crear un ordered file y que queden ordenados por Grafo > Key > Value.

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
