#!/bin/bash

for filename in test_queries/succeed/q*.txt; do
  printf "Testing $(basename $filename): "
  bin/parse "$filename" > /dev/null && echo "Success!"
done
