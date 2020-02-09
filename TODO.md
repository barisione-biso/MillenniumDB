- Seg Fault build/Release/bin/CreateDB test_files/graph_creation_example/nodes.txt test_files/graph_creation_example/edges.txt
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
- Importing graph:
    - remove "" to strings
    - add int support
- Object File:
    - use disk if importing more than X nodes?
- Catalog:
    - Stop if having key for node/edge more than X different keys.
- Usar memcpy cuando sea posible (ordered file y bpt)