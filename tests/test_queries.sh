#!/bin/bash

#execute from the project root
for filename in ./tests/queries/*.query; do
  printf "Testing $(basename $filename): "
  ./build/Release/bin/query "$filename" > /dev/null && echo "Success!"
  # TODO: compare result with diff
done
