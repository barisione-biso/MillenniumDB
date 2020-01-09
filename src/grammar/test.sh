#!/bin/bash

for filename in tests/succeed/q*.txt; do
  printf "Testing $(basename $filename): "
  ../../bin/Grammar "$filename" > /dev/null && echo "Success!"
done
