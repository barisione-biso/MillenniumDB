# TODO: 
# try to improve times of the slowest ones with parameters: MAX_BUCKETS, hash_function, treshold MAX_SIZE_SMALL_HASH

import re
from statistics import mean

# TODO: test hash join in memory
# >make table with all times, include in memory hash and less buckets
# >check last changes and repeat querys to find bug -> buckets or pages
# start migrating multi map to 1 file
# make table with new multi map
# try to solve bug

test = 8
parameter = "MAX_BUCKETS"  # -> lowest numbers where better, need to try with bigger querys
values = [2**3, 2**2, 2**1]

#parameter = "Treshold"
#values = [1024*8, 1024*16, 1024*32] #-->  2048 gives better results, tried with 4096 also but 2048 works better

#parameter = "function"
#values = ["xor", "murmur [1]", "murmur shift"]  ## almost no difference with 3 options 98ms-100ms-99ms averages

#parameter = "Join Iter"
#values = ["HashJoin", "HashJoinInMemory", "Quad"]  # change last for quad and 1-3

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
