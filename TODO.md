- Pruebas con rama refactoring
- Probar agregando 10 verces el small al final
    - orden inverso tambien

- Join order con plan y estimacion de costos

- Reemplazar el B+Tree de hash=>Id por una estructura de datos para diccionarios.
    - Suponer que cabe en memoria (y no usaria buffer manager)
    - Usar std map primera version
    - 24 Bytes por objeto (diccionario 16 bytes => 8 bytes)
    - Incluir en catálogo?

- Añadir Databases de pruebas como archivo comprimido al repositorio
- Rehacer dependencia de objetos globales:
    - sacar nifty counter
    - Modelo los inicializa y destruye?
    - Ejecutables inicializan Modelo. Usar factory para parámetros?
- Testear importacion de diferentes grafos en ambos modelos
    - partir con grafo grande y añadir chicos.
    - partir con grafo chico y añadir grande.

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
- permitir añadir datos a grafo existente -> otro ejecutable?

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
