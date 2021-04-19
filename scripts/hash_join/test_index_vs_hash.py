#7 queries 3 times each, repeat with nested loop,  this script makew table to test times results

## TODO: automatizar generación de archivos en un solo script
# make querys without limit and compare outputs
# try to improve times of the slowest ones with parameters: MAX_BUCKETS, hash_function, treshold MAX_SIZE_SMALL_HASH
# test hash in memory

import re
from statistics import mean

for i in range(1, 8):
    print(f"Test {i}:")
    hash_execution_times = []
    hash_parser_times = []
    print("  Hash:")
    for j in range(1, 4):
        with open(f"tests/outputs/hash_join/hash_{i}.{j}.txt") as file:
            lines = file.readlines()
            if j == 1:
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
        with open(f"tests/outputs/hash_join/quad_{i}.{j}.txt") as file:
            lines = file.readlines()
            if j == 1:
                print("   ", lines[-3].strip())
            quad_execution_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-2]).group()))
            quad_parser_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-1]).group()))
    print("   Execution times:", quad_execution_times)
    print("   AVG execution:", mean(quad_execution_times))
    print("   Parser times:", quad_parser_times)
    print("   AVG parser:", mean(quad_parser_times))

    # if answer_hash.difference(answer_ordered) or answer_ordered.difference(answer_hash):
    #     print("test incorrecto")
    #     print("Respuestas en hash que no están en ordered:")
    #     print(answer_hash.difference(answer_ordered))
    #     print("Respuestas en ordered que no están en hash:")
    #     print(answer_ordered.difference(answer_hash))
    # else:
    #     print("test correcto")
    print()
