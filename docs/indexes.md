| Node Labels | Edge Labels | ObjectLabel Index | Triple | Quad  |
|------------:|------------:|:-----------------:|:------:|:-----:|
|  = 0        |  = 0        | No*               | Yes    | No    |
|  = 0        |  = 1        | No*               | No     | Yes   |
|  = 0        |    ?        | Yes               | Yes    | Maybe |
|  = 1        |  = 0        | Yes               | Yes    | No    |
|  = 1        |  = 1        | Yes               | No     | Yes   |
|  = 1        |    ?        | Yes               | Yes    | Maybe |
|    ?        |  = 0        | Yes               | Yes    | No    |
|    ?        |  = 1        | Yes               | No     | Yes   |
|    ?        |    ?        | Yes               | Yes    | Maybe |

\* If we don't have the ObjectLabelIndex we can still search for labels
using the quad, because only Connection objects have a label.

Maybe means it would speed up queries but is not strictly necessary.