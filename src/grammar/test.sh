#!/bin/bash

for filename in test_queries/succeed/q*.txt; do
  echo "Testing $(basename $filename):"
  bin/parse "$filename" > /dev/null && echo "Success!"
  echo ""
done
