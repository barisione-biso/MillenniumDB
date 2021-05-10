import re
from statistics import mean

#test = 8
#parameter = "MAX_BUCKETS"  # -> lowest numbers where better, need to try with bigger querys
#values = [2**3, 2**2, 2**1]

#parameter = "Treshold"
#values = [1024*8, 1024*16, 1024*32] #-->  2048 gives better results, tried with 4096 also but 2048 works better

#parameter = "function"
#values = ["xor", "murmur [1]", "murmur shift"]  ## almost no difference with 3 options 98ms-100ms-99ms averages

parameter = "Join Iter"
values = ["HashJoin", "HashJoinInMemory", "HashJoinInMemory2", "Quad"]  # change last for quad and 1-3

for test in [7,8]:#range(1, 10):  # test number 9 is with triangle db
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
    # hash_execution_times = []
    # print(f"{parameter}: {values[1]}")
    # for j in range(1, 4):
    #     with open(f"tests/outputs/hash_join/hash_m1_{test}.{j}.txt") as file:
    #         lines = file.readlines()
    #         hash_execution_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-2]).group()))
    # print("   Execution times:", hash_execution_times)
    # print("   AVG execution:", mean(hash_execution_times))
    # print()

    hash_execution_times = []
    print(f"{parameter}: {values[2]}")
    for j in range(1, 4):
        with open(f"tests/outputs/hash_join/hash_m2_{test}.{j}.txt") as file:
            lines = file.readlines()
            hash_execution_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-2]).group()))
    print("   Execution times:", hash_execution_times)
    print("   AVG execution:", mean(hash_execution_times))
    print()

    # hash_execution_times = []
    # print(f"{parameter}: {values[3]}")
    # for j in range(1, 4):
    #     with open(f"tests/outputs/hash_join/quad_{test}.{j}.txt") as file:
    #         lines = file.readlines()
    #         hash_execution_times.append(float(re.search(r"[-+]?\d*\.\d+|\d+", lines[-2]).group()))
    # print("   Execution times:", hash_execution_times)
    # print("   AVG execution:", mean(hash_execution_times))
    # print()
