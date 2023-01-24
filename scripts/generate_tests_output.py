import glob
import os
import socket
import json
import subprocess
import time
import sys
from SPARQLWrapper import SPARQLWrapper, JSON

"""
This script generates the output of the good queries of the
SPARQL 1.1 test suite. This assumes that the database has
already been created.
"""

# Executables paths
WORKING_DIR = os.getcwd()
EXECUTABLES_DIR = os.path.join(WORKING_DIR, "build/Release/bin")
SERVER_EXECUTABLE = os.path.join(EXECUTABLES_DIR, "server_sparql2")
QUERY_EXECUTABLE = os.path.join(EXECUTABLES_DIR, "query")

# Base
TESTING_ROOT_DIR = os.path.join(WORKING_DIR, "tests")
TESTING_SPARQL_DIR = os.path.join(TESTING_ROOT_DIR, "testing/sparql")
TESTING_DBS_DIR = os.path.join(TESTING_ROOT_DIR, "testing_dbs/sparql")

# Constants
# Host and port that the server will listen on
HOST = "127.0.0.1"
PORT = 8080
ADDRESS = (HOST, PORT)
# Time between checks for server initialization
SLEEP_DELAY = 2
# Maximum time in seconds that the server will wait for a query
TIMEOUT = 60


def start_server(db_dir):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Check if port is already in use
    if sock.connect_ex(ADDRESS) == 0:
        print(f"Port {PORT} is already in use!")
        sys.exit(1)
    # Initialize server
    server_process = subprocess.Popen(
        [SERVER_EXECUTABLE, db_dir, "--timeout", str(TIMEOUT), "--port", str(PORT)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    # Wait for server initialization
    print(f"Waiting for the server to listen on port {PORT}...")
    while sock.connect_ex(ADDRESS) != 0:
        time.sleep(SLEEP_DELAY)
    print("Server is ready!")
    return server_process


def generate_good_queries_output(good_queries_dir, server_process):
    sparql_wrapper = SPARQLWrapper(f"http://{HOST}:{PORT}/sparql")
    sparql_wrapper.setReturnFormat(JSON)
    sparql_wrapper.setMethod("POST")
    sparql_wrapper.setRequestMethod("postdirectly")

    to_find = os.path.join(good_queries_dir, "*")
    for query_category_dir in glob.glob(to_find):
        query_category_name = os.path.basename(query_category_dir)

        to_find = os.path.join(query_category_dir, "*.rq")
        for query_file in glob.glob(to_find):
            query_name = os.path.basename(query_file)
            print(f"    Running query {query_category_name}/{query_name}")
            query = open(query_file, "r").read()
            sparql_wrapper.setQuery(query)
            results = sparql_wrapper.query()
            results_json = results.convert()
            open(query_file.replace(".rq", ".json"), "w").write(
                json.dumps(results_json, separators=(",", ":"))
            )


def generate_tests_output_json():
    to_find = os.path.join(TESTING_SPARQL_DIR, "*")
    test_dirs = glob.glob(to_find)
    for test_dir in test_dirs:
        test_name = os.path.basename(test_dir)
        db_dir = os.path.join(TESTING_DBS_DIR, test_name)

        server_process = start_server(db_dir)
        good_queries_dir = os.path.join(test_dir, "good_queries")
        print(f'Generating JSON for good queries of test "{test_name}"...')
        generate_good_queries_output(good_queries_dir, server_process)
        print(f'JSON for good queries of test "{test_name}" generated!')

        server_process.kill()
        server_process.wait()


if __name__ == "__main__":
    generate_tests_output_json()
