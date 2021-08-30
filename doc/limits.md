# Database Limits
- Max Identifiable Nodes: 2<sup>56</sup> (7 bytes to address), also limited by the object file size.
- Max Anonymous Nodes: 2<sup>56</sup> (7 bytes to address)
- Max Connections: 2<sup>56</sup>
- Max properties: TODO: are they limited by the B+Tree?
- Max labels: distinct labels limited by object file size.  TODO: are they limited by the B+Tree?
- Integer limts: TODO:
- B+Tree pages: 2<sup>32</sup>
- Object File Size: 2<sup>48</sup> bytes (limited by extendible_hash) TODO: may be less and collistions may be greater
- Max hash (murmur3) collitions: TODO:
    - TODO: collitions for tuples (hash distinct)?

    ## Object Domain
TODO: put it somewhere else
- **Integer**: integers from -(2<sup>56</sup>-1) to 2<sup>56</sup>-1.
    - Note: we are planning to support a wider range of integers int he future.

- **String**:
 Generic strings are delimited by double quotes (`""`) and they can be a sequence of arbitrary length of any UTF-8 character, but some characters need to be encoded (`\n`, `\t`, `\b`, `\f`, `\r`, `\/`, `\\`, `\"`).
    - **Names** match the regular expression `[A-Za-z][A-Za-z0-9_]*`.

    - **Keys** match the regular expression `[A-Za-z][A-Za-z0-9_]*`.

    - **Labels** match the regular expression `:[A-Za-z][A-Za-z0-9_]*.`

    For example: `foo` may be a **Key** or a **Name** (depends on the context), `:foo` is a label and `"foo"` is a generic string.

- **Float**: same as in C language. Beware, you may lose precision when using them.

- **Boolean** `true` or `false`.

- **Named Nodes**: they must match the regular expresion `[A-Za-z][A-Za-z0-9_]*`. There are exceptions for `true` and `false` because they are interpreted as booleans. Identifiers are case sensitive.

- **Anonymous Nodes** The auto-generated identifier will match the regular expresion `_a[1-9][0-9]*`.

- **Connections** The auto-generated identifier will match the regular expresion `_c[1-9][0-9]*`.