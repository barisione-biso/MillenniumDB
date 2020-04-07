#!/bin/bash

#execute from the project root
for filename in ./tests/succeed/q*.txt; do
  printf "Testing $(basename $filename): "
  ./build/Release/bin/query "$filename" > /dev/null && echo "Success!"
  # compare using diff and ignoring last line (time execution)
  # diff <(head -n -1 result) solution # TODO: ver si < funciona o debo ajustarlo a bash
done
