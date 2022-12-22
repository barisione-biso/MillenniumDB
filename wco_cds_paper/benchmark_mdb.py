import os
import socket
import subprocess
import sys
import time

# Port that the server will listen on
PORT = 8080

# Maximum time in seconds that the server will wait for a query
TIMEOUT = 60

print("\nArguments passed:", end = " ")
n = len(sys.argv)
for i in range(1, n):
    print(sys.argv[i], end = " ")

# Assume that the script is run from the root directory
MDB_DIR = sys.argv[1] # TODO: cambiar aca, donde tienes la carpeta de MDB
DBS_FOLDER = os.path.join(MDB_DIR, sys.argv[2]) # TODO: cambiar aca, por la carpeta de la BD (relativo a MDB_DIR)
EXECUTABLES_DIR   = os.path.join(MDB_DIR, "build/Release/bin")
SERVER_EXECUTABLE = os.path.join(EXECUTABLES_DIR, "server")
QUERY_EXECUTABLE  = os.path.join(EXECUTABLES_DIR, "query")

RESULTS_TMP = 'result.tmp'
QUERY_TMP = 'query.tmp'

force_cache_reloading = sys.argv[4]

def start_server(db_dir):
    print("starting server...")

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    location = ("127.0.0.1", PORT)

    # Check if port is already in use
    if s.connect_ex(location) == 0:
        print(f"Port {PORT} is already in use")
        sys.exit(1)

    server_process = subprocess.Popen(
        [SERVER_EXECUTABLE, db_dir, "--timeout", str(TIMEOUT), "--port", str(PORT)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    # Wait for server initialization
    while s.connect_ex(location) != 0:
        time.sleep(1)

    print(f"server started[pid={server_process.pid}]")
    return server_process


def kill_server(server_process):
    print(f"killing server[pid={server_process.pid}]...")
    server_process.kill()
    server_process.wait()
    print(f"server killed")


def execute_query(query, out, pid):
    if os.path.exists(RESULTS_TMP):
        os.remove(RESULTS_TMP)
    with open(QUERY_TMP, "w") as query_file:
        query_file.write(query)

    start_time = time.time_ns()
    with open(RESULTS_TMP, "w") as results_file, \
         open(QUERY_TMP, "r") as query_file:
        query_execution = subprocess.Popen(
            [QUERY_EXECUTABLE],
            stdin=query_file,
            stdout=results_file,
            stderr=subprocess.DEVNULL,
        )
        exit_code = query_execution.wait()
        #elapsed_time = int((time.time() - start_time) * 1000)
        elapsed_time = float((time.time_ns() - start_time)/1000000000)

        if force_cache_reloading == "True":
            os.system("touch "+DBS_FOLDER+"/*")

    p = subprocess.Popen(['wc', '-l', RESULTS_TMP], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    result, _ = p.communicate()
    results_count = int(result.strip().split()[0]) - 6 # 1 line from header + 2 for separators + 3 for stats

    mem_cmd = f'grep ^VmRSS /proc/{pid}/status'.split(' ')
    process = subprocess.Popen(mem_cmd, universal_newlines=True, stdout=subprocess.PIPE)
    out_cmd, err_cmd = process.communicate()
    out_cmd = out_cmd.strip().split()[1]

    if exit_code == 0:
        out.write(f'\'{query}\t{results_count}\tOK\t{elapsed_time}\t{out_cmd}\n')
    else:
        if elapsed_time >= TIMEOUT:
            out.write(f'\'{query}\t{results_count}\tTIMEOUT\t{elapsed_time}\t{out_cmd}\n')
        else:
            out.write(f'\'{query}\t0\tERROR\t{elapsed_time}\t{out_cmd}\n')


if __name__ == "__main__":

    query_file = open(sys.argv[3], 'r')
    queries = query_file.readlines()
    server_process = start_server(f'{DBS_FOLDER}')
    with open(f'benchmark_results.tsv', 'w') as results_file:
        results_file.write('query\tresults\tstatus\ttime\tmax_mem[kB]\n')
        for query in queries:
            print(query.strip('\n'))
            execute_query(query.strip('\n'), results_file, server_process.pid)
    kill_server(server_process)

    # Remove temp files
    if os.path.exists(RESULTS_TMP):
        os.remove(RESULTS_TMP)
    if os.path.exists(QUERY_TMP):
        os.remove(QUERY_TMP)
