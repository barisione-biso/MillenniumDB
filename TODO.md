- Merge Join y pruebas
- Caso conexiones de un nodo consigo mismo
    - crear índice (ambas permutaciones?)
    - crear Op (operador lógico)
    - crear join optimizer correspondiente
- mejorar printer para plan físico
- cambiar nombre de plan físico a executor?, plan ya no es ejecutable

- Estimación de tuplas de output y de costo
    - Materializar resultados que se usarán varias veces?
    - Selinger
- Reemplazar el B+Tree de hash=>Id por una estructura de datos para diccionarios.


- cambiar asserts por macro que pueda desactivarlos
    - definir funcion en módulo test
    - por defecto hace el assert tanto en RELEASE como DEBUG
    - debería ser fácil quitar el assert para RELEASE
- Añadir Databases de pruebas como archivo comprimido al repositorio
- Rehacer dependencia de objetos globales:
    - sacar nifty counter
    - Modelo los inicializa y destruye?
    - Ejecutables inicializan Modelo. Usar factory para parámetros?

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
