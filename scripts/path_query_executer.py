import subprocess
import sys
import os
from time import time
from time import sleep
import signal

'''
path_query executer asumes that a server of Millenium DB
is already running. Example of use (the terminal in the root of the project):

$ python3 scripts/path_query_executer.py tests/queries/property_paths/ 5 1

For each query, execute 1 run as pre run, and take the take
the time of the following 5 executions

'''
'''
TODO: Generate limit 100, 1000
'''




def start_server(name: str, bd_path: str, buffer_size: int, port: int):
    server_process = subprocess.Popen(
                    [f'tests/path_enum_servers/{name}_server -d {bd_path} -p {port} -b {buffer_size}'],
                    shell=True,
                    preexec_fn=os.setsid)
    return server_process


def run(query_path: str, n_test: int, skip: int, port: int, pre_run: int, resume: dict, server: str):
    test_sets = os.listdir(query_path)
    # Each set is a query with that returns full o a percent of a query
    test_sets.sort()
    for folder in test_sets:
        tests_files = os.listdir(f'{query_path}/{folder}')
        tests_files.sort()
        for test_name in tests_files:
            f_test_name = f'{folder}/{test_name}'
            if f_test_name not in resume:
                resume[f_test_name] = dict()
            resume[f_test_name][server] = {'mean': 0, 'size': None}
            execution_times = list()

            # Pre runs query for avoid variability due to cache, disk read, etc
            for _ in range(pre_run):
                query_execution = subprocess.Popen(
                    ['./build/Release/bin/query', f'{query_path}{folder}/{test_name}', '-p', f'{port}'],
                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                query_execution.wait()

            # Run a query and take the time
            for _ in range(n_test):
                t_start = time()
                with open('.tmp_query.txt', 'w') as file:
                    query_execution = subprocess.Popen(
                        ['./build/Release/bin/query', f'{query_path}{folder}/{test_name}', '-p', f'{port}'],
                        stdout=file, stderr=subprocess.DEVNULL)
                    query_execution.wait()
                execution_time = time() - t_start
                read_output = subprocess.Popen(['tail', '-4', '.tmp_query.txt'],
                    stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)

                out, __ = read_output.communicate()
                n_results = out.decode('utf-8').split('\n')
                if len(n_results) >= 4:
                    n_results = int(n_results[-4].split(' ')[1])
                else:
                    n_results = 0

                if resume[f_test_name][server]['size'] is None:
                    resume[f_test_name][server]['size'] = n_results
                elif resume[f_test_name][server]['size'] != n_results:
                    print(f'ERROR: {server}:{test_name} output size is not consistent')

                execution_times.append(execution_time)

            if skip and len(execution_times) > 2:
                execution_times.sort()
                execution_times.pop(0)
                execution_times.pop(len(execution_times)- 1)
            resume[f_test_name][server]['mean'] = str(round(sum(execution_times) / len(execution_times), 4)).replace(".", ".")


def check_consistency(resume: dict):
    for test in resume:
        for server in resume[test]:
            for server_2 in resume[test]:
                if resume[test][server]['size'] != resume[test][server_2]['size']:
                    print(f'ERROR: {test} has not output consistency between {server} and {server_2}')


def make_csv(resume: dict, servers: list):
    with open ('./out.csv', 'w') as file:
        for s in servers:
            file.write(f';{s}')
        file.write(';output size\n')
        for test in resume:
            file.write(f'{test}')
            output_size = 0
            for s in servers:
                mean = resume[test][s]['mean']
                output_size = resume[test][s]['size']
                file.write(f';{mean}')
            file.write(f';{output_size}\n')



if len(sys.argv) < 4:
    print('Error, wrong number of args.Please enter: ')
    print('1) Folder of queries to run. Ex: tests/my_tests')
    print('2) Times each query will be executed: Ex 15')
    print('3) 1 If you want to delete extreme values in results: Ex 1')
    print('4) Path to bd')
    print('5) Number of pre runs of each query. Default 10')
    print('6) Port of the server (optional, default 8080)')
    print('7) Buffer size. Default 1gb')
    sys.exit()

path_folder = sys.argv[1]  # The path to the folder of a query
n_test      = int(sys.argv[2])  # The times of each query will be executed
skip        = int(sys.argv[3])  # Delete max and min of each test
bd_path     = sys.argv[4]
pre_run     = 10
port        = 8080
buffer      = 100000

if len(sys.argv) > 5:
    pre_run = int(sys.argv[5])

if len(sys.argv) > 6:
    port = int(sys.argv[6])

if len(sys.argv) > 7:
    buffer = int(sys.argv[7])


servers_list = ['bfs_simple', 'bfs', 'dfs', 'astar']
resume_executions = dict()
for server in servers_list:
    server_process = start_server(server, bd_path, buffer, port)
    # Wait for server launch. TODO: Adjust param according to server
    sleep(30)
    try:
        run(path_folder, n_test, skip, port, pre_run, resume_executions, server)
        os.killpg(server_process.pid, signal.SIGINT)

    except Exception as error:
        os.killpg(server_process.pid, signal.SIGINT)
        resume_executions[server] = dict()
        print(error)
        sys.exit()

remove_out_file = subprocess.Popen(
                ['rm', '.out_query.txt'],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

check_consistency(resume_executions)
make_csv(resume_executions, servers_list)
