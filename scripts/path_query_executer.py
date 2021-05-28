import subprocess
import sys
import os
from time import time

'''
path_query executer asumes that a server of Millenium DB
is already running. Example of use (the terminal in the root of the project):

$ python3 scripts/path_query_executer.py tests/queries/property_paths/ 5 1

For each query, execute 1 run as pre run, and take the take
the time of the following 5 executions

'''

PRE_SKIP = 10 # Pre execution to avoid variability due to cache, read to disk, etc

def run_query(query_path: str, n_test: int, skip: int, port: int):
    test_sets = os.listdir(query_path)
    # Each set is a query with that returns full o a percent of a query
    for folder in test_sets:
        tests_files = os.listdir(f'{query_path}/{folder}')
        print(f'<<<<<<<<<<<< {folder} >>>>>>>>>>>>')
        tests_files.sort()
        for test_name in tests_files:
            resume = list()
            # Pre runs query for avoid variability due to cache, disk read, etc
            for _ in range(PRE_SKIP):
                query_execution = subprocess.Popen(
                    ['./build/Release/bin/query', f'{query_path}{folder}/{test_name}', '-p', f'{port}'],
                    stdout=subprocess.DEVNULL)
                query_execution.wait()
            # Run a query and take the time
            for _ in range(n_test):
                t_start = time()
                query_execution = subprocess.Popen(
                    ['./build/Release/bin/query', f'{query_path}{folder}/{test_name}', '-p', f'{port}'],
                    stdout=subprocess.DEVNULL)
                query_execution.wait()

                execution_time = time() - t_start

                resume.append(execution_time)
            if skip:
                resume.sort()
                resume.pop(0)
                resume.pop(len(resume)- 1)
            mean = str(round(sum(resume) / len(resume), 4)).replace(".", ",")
            print(f'{test_name}: {mean}')




if len(sys.argv) < 4:
    print('Error, wrong number of args.Please enter: ')
    print('1) Folder of queries to run. Ex: tests/my_tests')
    print('2) Times each query will be executed: Ex 15')
    print('3) 1 If you want to delete extreme values in results: Ex 1')
    print('4) Port of the server (optional, default 8080)')
    sys.exit()

path_folder = sys.argv[1]  # The path to the folder of a query
n_test      = int(sys.argv[2])  # The times of each query will be executed
skip        = int(sys.argv[3])  # Delete max and min of each test
port        = 8080

if len(sys.argv) > 4:
    port = int(sys.argv[4])

run_query(path_folder, n_test, skip, port)





