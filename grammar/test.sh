#!/bin/bash

for filename in test_queries/succeed/q*.txt; do
  echo "Testing $filename ..."
  bin/parse "$filename" > /dev/null && echo "Success at $filename"
  echo ""
done
