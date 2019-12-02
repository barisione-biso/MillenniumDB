# Compiling
- Prerequisites:
    - `libssl-dev`
    - makefile works only for linux right now.
- `make`

# Debugging
    - sudo sysctl -w kernel.core_pattern=/tmp/core-%e.%p.%h.%t
    - ulimit -c unlimited
    - make clean
    - make cleanfiles
    - make DEBUG=1
    - gdb bin/GraphDB /tmp/core-GraphDB[autocomplete]
        - bt
        - frame <number_from_backtrace>
        - print <variable_name>