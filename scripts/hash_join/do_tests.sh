#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Illegal number of parameters"
    exit 2
fi

folder="$1"
iterations="$2"
alg_name="$3"

echo -n "" > $folder/times_$alg_name.txt
for testname in $folder/query*; do
    for (( i=1; i<=$iterations; i++ )); do
        start=$(date +%s.%N)
        outputs=$(build/Release/bin/query $testname | tail -n 3 | head -n 1 | tr -dc '0-9')
        end=$(date +%s.%N)
        time_=$(python -c "print(${end} - ${start})")
        echo "$testname,$time_,$outputs" >> $folder/times_$alg_name.txt
    done
done

python3 scripts/hash_join/do_tests.py $folder $iterations $alg_name