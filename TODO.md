- Crear set consultas con los cambios propuestos por Domagoj
? Comparar RDF, creando los mismos índices (como si todo fuese triple, 3 índices)

- Ejecutar cada consulta 10 veces en cada motor, sacar (ignorar) minimo, maximo y promedio y tabular
    - entre cada experimento dar un sleep de 5 segundos
    - si todas son muy selectivas, inventar algunas con muchos resultados o costosas
    - mandar correo
- HASH: MURMUR3
    - implementar extendable hashing

____________________________________________________________________
? Comparar ejecucion de MD5 y otros hash, ejecutar 100.000.000 y medir tiempos
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
