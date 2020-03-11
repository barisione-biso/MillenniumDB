- Crear test queries:
    - Busqueda de nodo por label:
        - largo string = 7, sin resultados
        - largo string = 7, 1 resultado
        - largo string = 7, varios resultados
        - largo string = 8, sin resultados
        - largo string = 8, 1 resultado
        - largo string = 8, varios resultados

    - Busqueda de edge por label:
        - largo string = 7, 1 resultado
        - largo string = 7, varios resultados
        - largo string = 8, 1 resultado
        - largo string = 8, varios resultados

    - Busqueda de nodo por labels:
        - largo string = <= 7 + largo string >= 8, varios resultados
        - largo string = <= 7 + largo string >= 8, 1 resultado

    - Busqueda de nodo por labels:
        - largo string = <= 7 + largo string >= 8, varios resultados
        - largo string = <= 7 + largo string >= 8, 1 resultado

    - Busqueda de nodo por property:
        - largo string = 7, 1 resultado
        - largo string = 7, varios resultados
        - largo string = 8, 1 resultado
        - largo string = 8, varios resultados

    - Busqueda de edge por label:
        - largo string = 7, 1 resultado
        - largo string = 7, varios resultados
        - largo string = 8, 1 resultado
        - largo string = 8, varios resultados

    - Busqueda de nodo por properties:
        - largo string = <= 7 + largo string >= 8, varios resultados
        - largo string = <= 7 + largo string >= 8, 1 resultado

    - Busqueda de nodo por properties:
        - largo string = <= 7 + largo string >= 8, varios resultados
        - largo string = <= 7 + largo string >= 8, 1 resultado


- sacar = default de los .h y pasalos al .cc
- add override to virtual functions overrided?
- Probar si usando constructores con rvalue references para unique_ptrs se mejora la performance.
- Arreglar gramatica:
    - (no_skip afecta a comentarios //) ¿sacar no_skip?
    - Support special letters (áéíóúñäëïöüøñ) in labels and keys
    - labels no necesitan estar separadas ej: (?n:Person) parsea bien
- Comentarios Page y OrderedFile

- Ordered file: checkear al cambiar el orden si en verdad es necesario
- Bulk Import: borrar ordered_files al terminar

- Crear Catalogo en bulk import
- FileManager: for each file open, have its open pages?
- Al hacer bulk import hash2id se va insertando 1 a 1, es posible mejorar esto?
- Posible optimización: en ordered file create_run usar merge sort y el insertion sort solo cuando el output_buffer hace flush
  ojo que al ordenar otra permutación se debe ordenar si o si
- Example queries
- Buffer manager:
    - allocating more space if necesary
- Object File:
    - use disk if importing more than X nodes?
- Catalog:
    - Stop if having key for node/edge more than X different keys.
- Usar memcpy cuando sea posible (ordered file y bpt)