from SPARQLWrapper import SPARQLWrapper, JSON, GET, POST
import pandas as pd
import datetime
import subprocess as sp
import os
from time import sleep
import time
from dotenv import load_dotenv
import os
import gc

load_dotenv()

# list of session variables that are needed
system_variables = ["BLAZEGRAPH_JAR", "BLAZEGRAPH_PROPERTIES", "JENA_FUSEKI_SERVER", "JENA_TDBLOADER2",
                    "VIRTUOSO_T", "VIRTUOSO_ISQL", "VIRTUOSO_INI", "MILLENIUMDB_CREATE", "MILLENIUMDB_SERVER",
                    "MILLENIUMDB_QUERY"]

# for each of these variables
for var in system_variables:

    # add variable to code ('x' to x) by setting it to env variable
    exec(f"{var}=os.getenv('{var}')")

    # check that variable is not None
    if eval(var) is None:
        print(f"environment {var} not defined.")
        quit()

# Save current path
CURR_PATH = os.path.abspath("")

def create_if_missing(directory):
    if not os.path.exists(directory):
            os.makedirs(directory)

def run_command(command, output, stderr=None, wait_for="file"):
    """
    Runs a specific command and waits until the process returns an output.
    command  : string with a command (or argv list).
    output   : output to wait for
    stderr   : where to redirect the process error messages.
    wait_for : specifies wether to wait for process or not
    """
    if type(command) == str:
        command = command.split(" ")
    gc.collect()
    if wait_for == "file":
        with open("out.txt", 'w') as f:
            start = time.time()
            process = sp.Popen(command, stdout=f, stderr=stderr)

        """
        with open("out.txt", 'r') as f:

            while True:
                for line in f:
                    if output in line:
                        return process, time.time() - start
        """
        while True:
            with open("out.txt", 'r') as f:
                text = f.read()
                if output in text:
                    return process, time.time() - start

    elif wait_for == "process":
        start = time.time()
        process = sp.Popen(command, stdout=sp.PIPE, stderr=stderr)
        while True:
            nextline = process.stdout.readline()
            if output in nextline.decode('utf-8'):
                return process, time.time() - start


BG_ENDPOINT = "http://localhost:9999/blazegraph/namespace/kb/sparql"
JENA_ENDPOINT = "http://localhost:3030/NAME/query"
VIRTUOSO_DB = "db"
VIRTUOSO_ENDPOINT = "http://localhost:1122/sparql"

def kill(pid):
    try:
        sp.run(["kill", str(pid)])
    except:
        pass

# Wrapper to go into folder and exit
def change_dirs(fn, *args, **kwargs):
    def wrapped(self=None, *args, **kwargs):
        os.chdir(self.database_path)
        result = fn(self, *args, **kwargs)
        os.chdir(CURR_PATH)
    return wrapped

class Wrapper():

    def __init__(self, endpoint):
        self.query_file = None
        self.database_file = None
        self.server = None
        self.server_pid = None
        self.input = None
        self.endpoint = endpoint
        self.sparql_wrapper = SPARQLWrapper(self.endpoint)
        self.sparql_wrapper.setUseKeepAlive()
        self.time = -1

    def set_database_path(self, database_path):
        create_if_missing(os.path.abspath(database_path))
        self.database_path = database_path

    # Need overwrite
    def bulk_import(self, input_file):
        pass

    # Need overwrite
    def start(self):
        pass

    def query(self, query_file=None):
        if query_file is not None:
            self.query_file = query_file
        with open(self.query_file, 'r') as f:
            query = f.read()
        self.sparql_wrapper.setMethod(GET)
        self.sparql_wrapper.setQuery(query)
        self.sparql_wrapper.setReturnFormat(JSON)
        start = time.time()
        results = self.sparql_wrapper.query()
        self.time = time.time() - start
        results = results.convert()
        return self.process_results(results)

    def process_results(self, results):
        new_results = []
        for binding in results['results']['bindings']:
            new_binding = dict()
            for variable in binding:
                value = binding[variable]["value"]
                value_index = value.rfind("/")
                value = value[value_index + 1:]
                new_binding[f"?{variable}"] = value
            new_results.append(new_binding)

        # Transform to set of tuples
        for i in range(len(new_results)):
            new_results[i] = tuple(sorted(new_results[i].items()))

        return new_results

    def stop(self):
        if self.server_pid is not None:
            kill(self.server_pid)
            self.server_pid = None

class MilleniumDBWrapper(Wrapper):

    @change_dirs
    def bulk_import(self, input_file):

        command = f"{MILLENIUMDB_CREATE} {input_file} {self.database_path}"
        run_command(command, "Bulk Import duration")



    @change_dirs
    def start(self):
        command = f"{MILLENIUMDB_SERVER} -d {self.database_path}"
        process, exec_time = run_command(command, "Server running", stderr=sp.STDOUT, wait_for="file")
        self.server_pid = process.pid
        self.time = exec_time

    def query(self, query_file=None):
        if query_file is None and self.query_file is None:
            print("No query file defined.")
            return

        if query_file is not None:
            self.query_file = query_file

        command = f"{MILLENIUMDB_QUERY} {self.query_file}".split(" ")
        start = time.time()
        output = sp.run(command, stdout=sp.PIPE)
        self.time = time.time() - start
        output = output.stdout.decode()
        return self.process_results(output)

    def process_results(self, results):
        time_index = results.find("Execution time: ") + len("Execution time: ")
        time = results[time_index:]
        time = time[:time.find(" ")]
        if "{" not in results:
            return []
        new_results = []
        start = results.find("{")
        end = results.rfind("}")
        results = results[start:end+1]
        results = results.split("\n")
        for r in range(len(results)):
            new_binding = dict()
            results[r] = results[r][1:-1] # Delete {}
            results[r] = results[r].split(",") # Separate variables
            for tup in results[r]:
                var, value = tup.split(":") # Separate variable - value
                if value != "null": # Supose no 'null' strings (although should work: null != 'null')
                    if "(" in value:
                        value = value.replace("(", "")
                    if ")" in value:
                        value = value.replace(")", "")
                    new_binding[var] = value
            new_results.append(new_binding)

        # Transform to set of tuples
        for i in range(len(new_results)):
            new_results[i] = tuple(sorted(new_results[i].items()))

        return new_results

class BlazeGraphWrapper(Wrapper):

    @change_dirs
    def bulk_import(self, input_file):

        command = f"java -cp {BLAZEGRAPH_JAR} com.bigdata.rdf.store.DataLoader -namespace kb {BLAZEGRAPH_PROPERTIES} {input_file}"#
        run_command(command, "Total elapsed=")


    @change_dirs
    def start(self):

        # Run server
        command = f"java -server -Xmx4g -jar {BLAZEGRAPH_JAR}"
        process, exec_time = run_command(command, "to get started.")
        self.time = exec_time
        self.server_pid = process.pid

class JenaWrapper(Wrapper):

    def set_database_path(self, database_path):
        self.database_path = database_path

    def bulk_import(self, input_file):

        command = f"{JENA_TDBLOADER2} --loc {self.database_path} {input_file}"
        run_command(command, "TDB Bulk Loader Finish", wait_for="process")

    @change_dirs
    def start(self):

        start = time.time()
        command = f"{JENA_FUSEKI_SERVER} --loc={self.database_path} /NAME"
        process, exec_time = run_command(command, "CLST on port")
        self.time = exec_time
        self.server_pid = process.pid

class VirtuosoWrapper(Wrapper):

    @change_dirs
    def bulk_import(self, input_file):

        # Add allowed directories for .ini for bulk import
        virtuoso_directory = os.path.dirname(VIRTUOSO_INI)
        database_directory = os.path.dirname(os.path.abspath(input_file))
        with open(VIRTUOSO_INI, 'r') as f:
            text = f.read()
        idx_start = text.find("DirsAllowed")
        idx_end = text[idx_start:].find("\n") + idx_start
        new_text = text[:idx_start] + f"DirsAllowed = {virtuoso_directory},{database_directory}" + text[idx_end:]
        with open(VIRTUOSO_INI, 'w') as f:
            f.write(new_text)

        # Start server
        command = f"{VIRTUOSO_T} -c {VIRTUOSO_INI} -f"
        process, exec_time = run_command(command, "Server online", stderr=sp.STDOUT)
        self.server_pid = process.pid

        # Bulk import
        start = time.time()
        command = [VIRTUOSO_ISQL,"1111","dba","dba",f"exec=ld_dir('{database_directory}', '*.ttl', 'http://www.example.com');"]
        run_command(command, "Done")

        # Command 2
        command = f"{VIRTUOSO_ISQL} 1111 dba dba exec=rdf_loader_run();"
        run_command(command, "Done")

        # Command 3
        command = f"{VIRTUOSO_ISQL} 1111 dba dba exec=checkpoint;"
        run_command(command, "Done")
        self.time = time.time() - start

        # Stop server
        self.stop()

    @change_dirs
    def start(self):

        # Start server
        command = f"{VIRTUOSO_T} -c {VIRTUOSO_INI} -f"
        process, exec_time = run_command(command, "Server online", stderr=sp.STDOUT)
        self.time = exec_time
        self.server_pid = process.pid

if __name__=="__main__":
    """
    mdb = MilleniumDBWrapper(None)
    mdb.start(input_file="./tests/optional/dbs/db_mdb_12", database_path = f"./{Wrapper.path}/milleniumdb/db")
    results = mdb.query("./tests/optional/queries/12_mdb_q0")
    print(len(results))
    mdb_r = results
    mdb.stop()


    bg = BlazeGraphWrapper(BG_ENDPOINT)
    bg.start(input_file="./tests/optional/dbs/db_sparql_12.ttl")
    results = bg.query("./tests/optional/queries/12_sparql_q0")
    print(len(results))
    bg_r = results
    bg.stop()

    jena = JenaWrapper(JENA_ENDPOINT)
    jena.start(input_file="./tests/optional/dbs/db_sparql_12.ttl")
    results = jena.query("./tests/optional/queries/12_sparql_q0")
    print(len(results))
    jena_r = results
    jena.stop()
    virtuoso = VirtuosoWrapper(VIRTUOSO_ENDPOINT)
    virtuoso.start(input_file="./tests/optional/dbs/db_sparql_12.ttl")
    results = virtuoso.query("./tests/optional/queries/12_sparql_q0")
    print(len(results))
    virtuoso_r = results
    virtuoso.stop()

    print("End")
    print(sorted(virtuoso_r) == sorted(jena_r))
    print(sorted(virtuoso_r) == sorted(bg_r))
    print(sorted(bg_r) == sorted(jena_r))
    """
    print("Hola")