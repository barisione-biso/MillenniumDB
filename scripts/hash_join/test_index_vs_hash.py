#7 queries 3 times each, repeat with nested loop,  this script makew table to test times results

## TODO: automatizar generación de archivos en un solo script
# make querys without limit and compare outputs -> 7 memory error, 6 more than 30 min
# test hash in memory

import re
from statistics import mean

for i in range(1, 9):
    if i == 6 or i == 7:
        continue
    print(f"Test {i}:")
    answer_hash = set()
    answer_quad = set()
    hash_execution_times = []
    hash_parser_times = []
    print("  Hash:")
    for j in range(1, 4):
        if i == 8 and j == 2:
            break
        with open(f"tests/outputs/hash_join/hash_{i}.{j}.txt") as file:
            lines = file.readlines()
            if j == 1 and i != 7:
                for line in lines[:-3]:
                    line = line.strip()
                    answer_hash.add(line)
                print("   ", lines[-3].strip())
            hash_execution_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-2]).group()))
            hash_parser_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-1]).group()))
    print("   Execution times:", hash_execution_times)
    print("   AVG execution:", mean(hash_execution_times))
    print("   Parser times:", hash_parser_times)
    print("   AVG parser:", mean(hash_parser_times))

    quad_execution_times = []
    quad_parser_times = []
    print("  Index (Quad):")
    for j in range(1, 4):
        if i == 8 and j == 2:
            break
        with open(f"tests/outputs/hash_join/quad_{i}.{j}.txt") as file:
            lines = file.readlines()
            if j == 1 and i != 7:
                for line in lines[:-3]:
                    line = line.strip()
                    answer_quad.add(line)
                print("   ", lines[-3].strip())
            quad_execution_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-2]).group()))
            quad_parser_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-1]).group()))
    print("   Execution times:", quad_execution_times)
    print("   AVG execution:", mean(quad_execution_times))
    print("   Parser times:", quad_parser_times)
    print("   AVG parser:", mean(quad_parser_times))

    if answer_hash.difference(answer_quad) or answer_quad.difference(answer_hash):
        print("->test incorrecto")
        print("Respuestas en hash que no están en quad:")
        print(len(answer_hash.difference(answer_quad)))
        print("Respuestas en quad que no están en hash:")
        print(len(answer_quad.difference(answer_hash)))
    else:
        print("->test correcto")
    print()
    print()
