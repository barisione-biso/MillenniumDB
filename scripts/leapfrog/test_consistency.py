import glob
import os
import subprocess
import sys
from time import time
from time import sleep
import signal

aux_filename = '.out_query.tmp'

def start_server(name: str, bd_path: str, buffer_size: int, port: int):
    server_process = subprocess.Popen(
                    [f'build/Release/bin/server_{name} -d {bd_path} -p {port} -b {buffer_size}'],
                    shell=True,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                    preexec_fn=os.setsid)
    return server_process


def check_consistency(results: dict):
    for query in results:
        for server_1 in results[query]:
            for server_2 in results[query]:
                if results[query][server]['size'] != results[query][server_2]['size']:
                    size_1 = results[query][server_1]['size']
                    size_2 = results[query][server_2]['size']
                    print(f'ERROR: {query} has not output consistency between {server_1} ({size_1}) and {server_2} ({size_2})')


def run(queries_folder: str, port: int, server: str, warming_repetitions: int, repetitions: int, results: dict):
    # query_files = os.listdir(queries_folder)
    query_files = glob.glob(f'{queries_folder}/*.txt')
    for query_file in query_files:
        execution_times = list()

        if query_file not in results:
            results[query_file] = dict()
        results[query_file][server] = {'avg': 0, 'size': None}

        # warming repetions to avoid variability due to cache, disk read, etc
        for _ in range(warming_repetitions):
            query_execution = subprocess.Popen(
                ['./build/Release/bin/query', query_file, '-p', f'{port}'],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL)
            query_execution.wait()

        # Run a query and take the time
        for _ in range(repetitions):
            start_time = time()
            with open(aux_filename, 'w') as file:
                query_execution = subprocess.Popen(
                    ['./build/Release/bin/query', query_file, '-p', f'{port}'],
                    stdout=file,
                    stderr=subprocess.DEVNULL)
                query_execution.wait()
            execution_time = time() - start_time
            execution_times.append(round(execution_time*1000))

        # We only read the output once, we are supponsing all executions gave the same results
        with open(aux_filename, 'r') as file:
            # read output: count results and save bindings
            output_size = 0 # having this variable is redundant, it's equal to len(results[query_file][server]['bindings'])
                            # but maybe in the future we won't save bindings
            results[query_file][server]['bindings'] = []
            for line in file:
                if line[0] == '{':
                    results[query_file][server]['bindings'].append(line[1:-2]) # ignore '{' and '}\n'
                    output_size += 1

            results[query_file][server]['bindings'].sort()
            results[query_file][server]['size'] = output_size


        # if skip and len(execution_times) > 2:
        #     execution_times.sort()
        #     execution_times.pop(0)
        #     execution_times.pop(len(execution_times)- 1)
        avg_time = str(round(sum(execution_times) / len(execution_times)))
        results[query_file][server]['avg'] = avg_time


if len(sys.argv) < 3:
    print('Wrong number of arguments.')
    print('Usage: python3 test_consistency [queries_folder] [database_folder]')
    sys.exit()

queries_folder = sys.argv[1]  # The path to the folder of a query
db_path        = sys.argv[2]

# TODO: Adjust param according to server and database
# server_launch_delay = 90
# buffer_size = 900000
buffer_size = 300000
server_launch_delay = 1

port = 8080
repetitions = 10
warming_repetitions = 3


servers_list = ['nested_loop', 'leapfrog']
results = dict()
for server in servers_list:
    server_process = start_server(server, db_path, buffer_size, port)
    # Wait for server launch.
    sleep(server_launch_delay)

    run(queries_folder, port, server, warming_repetitions, repetitions, results)
    os.killpg(server_process.pid, signal.SIGINT)

# Check results are consistent
for query in results:
    if results[query]["leapfrog"]["size"] != results[query]["nested_loop"]["size"]:
        print(f'Error in {os.path.basename(query)} output size is different.')

    if results[query]["leapfrog"]["bindings"] != results[query]["nested_loop"]["bindings"]:
        print(f'Error in {os.path.basename(query)} bindings are different.')

# Print summary as CSV
print('query', end='')
for server in servers_list:
    print(f',{server} avg,{server} result count', end='')
print()

for query in results:
    print(os.path.basename(query), end='')
    for server in servers_list:
        print(f',{results[query][server]["avg"]},{results[query][server]["size"]}', end='')
    print()

# remove output file
subprocess.Popen(['rm', aux_filename], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
