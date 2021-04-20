# TODO: 
# try to improve times of the slowest ones with parameters: MAX_BUCKETS, hash_function, treshold MAX_SIZE_SMALL_HASH

import re
from statistics import mean

test = 8
parameter = "MAX_BUCKETS" ## error when using 2**10
values = [2**9, 2**10, 2**11]

print(f"Test {test}:")
hash_execution_times = []
print(f"{parameter}: {values[0]}")
for j in range(1, 4):
    with open(f"tests/outputs/hash_join/hash_{test}.{j}.txt") as file:
        lines = file.readlines()
        hash_execution_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-2]).group()))
print("   Execution times:", hash_execution_times)
print("   AVG execution:", mean(hash_execution_times))
print()
hash_execution_times = []
print(f"{parameter}: {values[1]}")
for j in range(4, 7):
    with open(f"tests/outputs/hash_join/hash_{test}.{j}.txt") as file:
        lines = file.readlines()
        hash_execution_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-2]).group()))
print("   Execution times:", hash_execution_times)
print("   AVG execution:", mean(hash_execution_times))
print()

hash_execution_times = []
print(f"{parameter}: {values[2]}")
for j in range(7, 10):
    with open(f"tests/outputs/hash_join/hash_{test}.{j}.txt") as file:
        lines = file.readlines()
        hash_execution_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-2]).group()))
print("   Execution times:", hash_execution_times)
print("   AVG execution:", mean(hash_execution_times))
print()
