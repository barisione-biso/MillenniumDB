import colorama
import glob
import os
import socket
import subprocess
import sys
import time

colorama.init(autoreset=True)

# Usage:
# python test.py

# Assume that the script is run from the root directory
WORKING_DIR = os.getcwd()
# Executables paths
EXECUTABLES_DIR           = os.path.join(WORKING_DIR,     "build/Release/bin")
CREATE_DB_EXECUTABLE      = os.path.join(EXECUTABLES_DIR, "create_db_sparql")
SERVER_EXECUTABLE         = os.path.join(EXECUTABLES_DIR, "server_sparql")
QUERY_EXECUTABLE          = os.path.join(EXECUTABLES_DIR, "query")
# Base
TESTING_ROOT_DIR   = os.path.join(WORKING_DIR, "tests")
TESTING_SPARQL_DIR = os.path.join(TESTING_ROOT_DIR, "testing/sparql")
TESTING_DBS_DIR    = os.path.join(TESTING_ROOT_DIR, "testing_dbs/sparql")
# Results temp file
RESULTS_TMP_FILE = os.path.join(TESTING_ROOT_DIR, ".sparql_temp")

# Constants
# Port that the server will listen on
PORT        = 8080
# Time between checks for server initialization
SLEEP_DELAY = 2
# Maximum time in seconds that the server will wait for a query
TIMEOUT     = 60


def print_log(message, type=""):
    to_print = ""
    if type == "ok":
        to_print += colorama.Fore.GREEN  + "[ OK! ]"
    elif type == "error":
        to_print += colorama.Fore.RED    + "[ERROR]"
    elif type == "begin":
        to_print += colorama.Fore.WHITE + "[BEGIN]"
    elif type == "end":
        to_print += colorama.Fore.WHITE + "[ END ]"
    elif type == "warning":
        to_print += colorama.Fore.YELLOW + "[WARN!]"
    else:
        to_print += colorama.Fore.CYAN   + "[DEBUG]"
    print(to_print, colorama.Fore.RESET, message)


def create_db(rdf_file, prefixes_file):
    print_log("CREATE_DB()", type="begin")
    db_name = os.path.basename(rdf_file).split(".")[0]
    db_dir = os.path.join(TESTING_DBS_DIR, db_name)
    if os.path.exists(db_dir):
        print_log(f"Database directory \"{db_dir}\" already exists", type="warning")
    else:
        CREATE_DB_PROCESS = subprocess.Popen(
            [CREATE_DB_EXECUTABLE, rdf_file, db_dir, "-p", prefixes_file],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        exit_code = CREATE_DB_PROCESS.wait()
        if exit_code == 0:
            print(f"[ OK! ]     Database \"{db_name}\" created")
        else:
            print(f"[ERROR]     Database \"{db_name}\" creation failed")
    print_log("CREATE_DB()", type="end")
    return db_dir

def start_server(db_dir):
    print_log("START_SERVER()", type="begin")

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    location = ("127.0.0.1", PORT)

    # Check if port is already in use
    if s.connect_ex(location) == 0:
        print_log(f"Port {PORT} is already in use", type="error")
        sys.exit(1)

    server_process = subprocess.Popen(
        [SERVER_EXECUTABLE, db_dir, "--timeout", str(TIMEOUT), "--port", str(PORT)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    # Wait for server initialization
    print_log(f"Waiting for the server to listen on port {PORT}...")
    while s.connect_ex(location) != 0:
        time.sleep(SLEEP_DELAY)
    print_log(f"Port {PORT} is listening!")

    print_log(f"START_SERVER() [PID={server_process.pid}]", type="end")
    return server_process


def kill_server(server_process):
    print_log(f"KILL_SERVER()  [PID={server_process.pid}]", type="begin")
    server_process.kill()
    server_process.wait()
    print_log(f"KILL_SERVER()", type="end")

def query_bad(query_file, server_process):
    if server_process.poll() is not None:
        print_log("Server process has terminated unexpectedly!", type="error")
        sys.exit(1)

    with open(RESULTS_TMP_FILE, "w") as rtf, open(query_file, "r") as qf:
        query_process = subprocess.Popen(
            [QUERY_EXECUTABLE],
            stdin=qf,
            stdout=rtf,
            stderr=subprocess.DEVNULL,
        )
    exit_code = query_process.wait()
    return exit_code


def query_good(query_file, server_process):
    if server_process.poll() is not None:
        print_log("Server process has terminated unexpectedly!", type="error")
        sys.exit(1)

    with open(RESULTS_TMP_FILE, "w") as rtf, open(query_file, "r") as qf:
        query_process = subprocess.Popen(
            [QUERY_EXECUTABLE],
            stdin=qf,
            stdout=rtf,
            stderr=subprocess.DEVNULL,
        )
        exit_code = query_process.wait()
    expected_result_file = query_file.replace(".rq", ".txt")
    header_size = 2
    stats_size  = 4
    with open(RESULTS_TMP_FILE, "r") as rtf, open(expected_result_file, "r") as erf:
        rtf_set = set(rtf.readlines()[header_size:-stats_size])
        erf_set = set(erf.readlines()[header_size:-stats_size])
    return rtf_set == erf_set


def execute_bad_queries(bad_queries_dir, server_process):
    count_ok    = 0
    count_error = 0
    print_log("BAD QUERIES", type="begin")
    to_find = os.path.join(bad_queries_dir, "*")
    for query_category_dir in glob.glob(to_find):
        query_category_name = os.path.basename(query_category_dir)
        to_find = os.path.join(query_category_dir, "*")
        for query_file in glob.glob(to_find):
            query_name = os.path.basename(query_file)
            exit_code = query_bad(query_file, server_process)
            if exit_code == 0:
                print_log(f"\"{query_category_name}/{query_name}\"", type="error")
                count_error += 1
            else:
                print_log(f"\"{query_category_name}/{query_name}\"", type="ok")
                count_ok += 1
    print_log("BAD QUERIES", type="end")
    return (count_ok, count_error)


def execute_good_queries(good_queries_dir, server_process):
    count_ok    = 0
    count_error = 0
    print_log("GOOD QUERIES", type="begin")
    to_find = os.path.join(good_queries_dir, "*")
    for query_category_dir in glob.glob(to_find):
        query_category_name = os.path.basename(query_category_dir)
        to_find = os.path.join(query_category_dir, "*.rq")
        for query_file in glob.glob(to_find):
            query_name = os.path.basename(query_file)
            is_correct = query_good(query_file, server_process)
            if is_correct:
                print_log(f"\"{query_category_name}/{query_name}\"", type="ok")
                count_ok += 1
            else:
                print_log(f"\"{query_category_name}/{query_name}\"", type="error")
                count_error += 1
    print_log("GOOD QUERIES", type="end")
    return (count_ok, count_error)

def execute_queries():
    total_count_ok    = 0
    total_count_error = 0
    to_find = os.path.join(TESTING_SPARQL_DIR, "*")
    test_dirs = glob.glob(to_find)
    for test_dir in test_dirs:
        test_name        = os.path.basename(test_dir)
        print("-" * 50)
        print(f"Running test \"{test_name}\"...")

        file_ttl       = os.path.join(test_dir, f"{test_name}.ttl")
        file_nt        = os.path.join(test_dir, f"{test_name}.nt")
        file_prefixes  = os.path.join(test_dir, f"{test_name}_prefixes.txt")

        if not os.path.exists(file_prefixes):
            print_log(f"Prefixes file not found at \"{file_prefixes}\"", type="warning")
            file_prefixes = ""

        db_dir = None
        if os.path.exists(file_ttl):
            db_dir = create_db(file_ttl, file_prefixes)
        elif os.path.exists(file_nt):
            db_dir = create_db(file_nt, file_prefixes)
        else:
            print_log(f"No RDF file (.ttl or .nt) found in {test_dir}", type="error")
            continue


        server_process = start_server(db_dir)

        bad_queries_dir  = os.path.join(test_dir, "bad_queries")
        count_ok_bad, count_error_bad = execute_bad_queries(bad_queries_dir, server_process)
        total_count_ok    += count_ok_bad
        total_count_error += count_error_bad

        good_queries_dir = os.path.join(test_dir, "good_queries")
        count_ok_good, count_error_good = execute_good_queries(good_queries_dir, server_process)
        total_count_ok    += count_ok_good
        total_count_error += count_error_good
        
        kill_server(server_process)

        test_total_ok    = count_ok_bad + count_ok_good
        test_total_error = count_error_bad + count_error_good
        test_total       = test_total_ok + test_total_error
        print("-" * 50)
        print(f"SUMMARY ({test_name}):")
        print(f"    OK    : {test_total_ok}/{test_total}")
        print(f"    ERROR : {test_total_error}/{test_total}")

    return (total_count_ok, total_count_error)

if __name__ == "__main__":
    # Execution
    total_count_ok, total_count_error = execute_queries()
    total = total_count_ok + total_count_error
    print("-" * 50)
    print("FINAL SUMMARY:")
    print(f"    OK    : {total_count_ok}/{total}")
    print(f"    ERROR : {total_count_error}/{total}")
    print("-" * 50)
    # Remove temp file
    if os.path.exists(RESULTS_TMP_FILE):
        os.remove(RESULTS_TMP_FILE)
