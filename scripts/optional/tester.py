import subprocess as sp
from time import sleep
from generate_query import *
from graph import Graph
from wrapper import *
import sys
import matplotlib.pyplot as plt
import pandas as pd

"""
check input
"""
if len(sys.argv) < 2:
    print("Temporary directory needed:")
    print("python3 tester.py <tmp>")
    quit()

"""
Constants
"""
# Directory where all information will be stored
OUTPUT_FOLDER = sys.argv[1]
OUTPUT_FOLDER = os.path.abspath(OUTPUT_FOLDER)

# Execution of tests
HOTRUN = 3
N_QUERIES = 10
DBS_DIR = f"{OUTPUT_FOLDER}/dbs"
QUERIES_DIR = f"{OUTPUT_FOLDER}/queries"

# Endpoints
BG_ENDPOINT = "http://localhost:9999/blazegraph/namespace/kb/sparql"
JENA_ENDPOINT = "http://localhost:3030/NAME/query"
VIRTUOSO_ENDPOINT = "http://localhost:1122/sparql"

# Graph creation

MIN_NODES = 100
MAX_NODES = MIN_NODES
N_NODES = random.randint(MIN_NODES, MAX_NODES)

MIN_EDGES = MIN_NODES * 10
MAX_EDGES = MIN_NODES * 10
N_EDGES = random.randint(MIN_EDGES, MAX_EDGES)

MIN_RELATIONS = 3
MAX_RELATIONS= 3
N_RELATIONS = random.randint(MIN_RELATIONS, MAX_RELATIONS)

# Query creation
N_VARS = 6
MIN_CHILDS = 0
MAX_CHILDS = 3
MAX_DEPTH = 2


def compare_results(r1, r2):
    if len(r1) != len(r2):
        return False
    return sorted(r1) == sorted(r2)

def create_database(sparql_path, mdb_path, dot_graph_path):

    print("-" * 80)
    print("Creating database...")
    g = Graph(n_nodes = N_NODES,
            n_edges = N_EDGES,
            n_relations = N_RELATIONS)
    g.export(mdb_path, to="milleniumDB")
    g.export(sparql_path, to="SPARQL")

    print("Database generated and exported.")
    print(f"Database size: {round(os.path.getsize(sparql_path)/(1024*1024), 3)} MB.")
    return g

def create_query(sparql_query_path, mdb_query_path):

    # Create Optional Query Tree
    OptNode.min_childs = MIN_CHILDS
    OptNode.max_childs = MAX_CHILDS
    OptNode.max_depth = MAX_DEPTH
    n = OptNode(None, relations=[f"P{i}" for i in range(MIN_RELATIONS)], root=True, n_vars=N_VARS)

    # Generate milleniumDB query
    milleniumdb_query = n.generate_query(output="milleniumdb") # n.generate_output

    # Generate sparql query
    sparql_query = n.generate_query(output="sparql")

    # Export queries
    n.export(mdb_query_path, "milleniumdb")
    n.export(sparql_query_path, "sparql")

# Create the folders if they do not exist
create_if_missing(OUTPUT_FOLDER)
create_if_missing(DBS_DIR)
create_if_missing(QUERIES_DIR)

# Save each engine in a dictionary
engines = dict()
engine_list = ['blazegraph', 'jena', 'virtuoso', 'milleniumdb']
engines['blazegraph'] = BlazeGraphWrapper(BG_ENDPOINT)
engines['milleniumdb'] = MilleniumDBWrapper(None)
engines['jena'] = JenaWrapper(JENA_ENDPOINT)
engines['virtuoso'] = VirtuosoWrapper(VIRTUOSO_ENDPOINT)

# Set database path (e.g. <folder>/virtuoso, <folder>/blazegraph...)
for engine in engines:
    engines[engine].set_database_path(f"{OUTPUT_FOLDER}/{engine}")

# Save execution time of server start and query
logger = dict()
logger['mount_time'] = dict()
logger['query_time'] = dict()

# Save results to check for correctness
results = dict()
for engine in engine_list:
    results[engine] = dict()

try:

    # Define database paths
    sparql_db_path = f"{DBS_DIR}/sparql.ttl"
    mdb_db_path = f"{DBS_DIR}/mdb"
    dot_graph_path = f"{DBS_DIR}/sparql.ttl.graph"

    # if directory does not exist or is empty, we need to generate.
    replace = False
    if not os.path.exists(DBS_DIR) or not os.listdir(DBS_DIR):
        replace = True

    # Create database if does not exist
    if replace:
        g = create_database(sparql_db_path, mdb_db_path, dot_graph_path)

    # Create queries
    for q in range(N_QUERIES):

        # Define query paths
        sparql_query_path = f"{QUERIES_DIR}/sparql_{q}"
        mdb_query_path = f"{QUERIES_DIR}/mdb_{q}"

        # Create queries if need be
        if replace:
            print("-" * 80)
            print(f"Generating query {q}...")
            create_query(sparql_query_path, mdb_query_path)
            print("Query generated.")

        # Define time logger
        logger['query_time'][q] = dict()
        for engine in engine_list:
            logger['query_time'][q][engine] = dict()

    # foreach engine
    for engine in engine_list:

        # Set database path
        db_path = sparql_db_path
        if engine == "milleniumdb":
            db_path = mdb_db_path

        # Start engine
        print("-" * 80)
        print(f"Mounting {engine}...")

        if replace:
            engines[engine].bulk_import(db_path)
        engines[engine].start()
        logger['mount_time'][engine] = round(engines[engine].time * 1000, 1)
        print(f"{engine} started in {engines[engine].time} seconds.")

        # For each query
        q = 0
        while q < N_QUERIES:

            # Get path
            query_path = f"{QUERIES_DIR}/sparql_{q}"
            if engine == "milleniumdb":
                query_path = f"{QUERIES_DIR}/mdb_{q}"

            print("-" * 80)
            print(f"Executing query {q} on {engine}...")
            times = []

            try:    # TODO: Check beforehand
                # HOT RUN: Execute HOTRUN + 1 times, get average of 1 to HOTRUN+1
                for i in range(HOTRUN + 1):
                    results[engine][q] = engines[engine].query(query_file=query_path)
                    if i > 0:
                        logger['query_time'][q][engine][i - 1] = engines[engine].time
                        times.append(engines[engine].time)
                print("Number of results:", len(results[engine][q]))

                # Get avg time
                time = sum(times)/HOTRUN
                print(f"Done in {time} seconds average on {HOTRUN} executions.")
                q += 1
            except Exception as e:
                print("Error", e)
            except KeyboardInterrupt:
                print("\nKeyboard Interrupt: Stopping databases.")
                for engine in engine_list:
                    engines[engine].stop()
                    quit()

        # Stop engine
        engines[engine].stop()

    # save query logs
    with open(f"{OUTPUT_FOLDER}/query_log.csv", 'w') as f:
        f.write("query,N Results (BlazeGraph),BlazeGraph,N Results (Jena),Jena,N Results (Virtuoso),Virtuoso,N Results (MDB),MDB,Correct results\n")
        for q in range(N_QUERIES):
            correct_results = bool(compare_results(results['milleniumdb'][q], results['jena'][q]))
            for i in range(HOTRUN):
                text = f"{q},"
                for engine in engine_list:
                    text += f"{len(results[engine][q])},{logger['query_time'][q][engine][i]},"
                text += str(correct_results) + "\n"
                f.write(text)

    # save mount logs
    with open(f"{OUTPUT_FOLDER}/mount_log.csv", "w") as f:
        f.write("N nodes, N relations, size, MDB, BlazeGraph, Virtuoso, Jena\n")
        size = round(os.path.getsize(mdb_db_path)/(1024*1024), 3)
        text = f"{N_NODES},{N_EDGES},{size},"
        for engine in engine_list:
            text += f"{logger['mount_time'][engine]},"
        f.write(text[:-1] + "\n")

    # Generate graphs
    df = pd.read_csv(f"{OUTPUT_FOLDER}/query_log.csv")
    df.drop(['N Results (BlazeGraph)', 'N Results (Jena)', 'N Results (Virtuoso)', 'N Results (MDB)', 'Correct results'], axis=1, inplace=True)

    # Average query runs
    ax = df.groupby(['query']).mean().plot(kind='bar', rot=90)
    ax.set_xlabel("Query")
    ax.set_ylabel("Average time (ms)")
    plt.savefig(f'{OUTPUT_FOLDER}/average_query_time.png')
    plt.clf()

    # All query runs
    df.drop(['query'], axis=1, inplace=True)
    ax = df.plot(kind='bar', rot=90)
    ax.set_xlabel("Query")
    ax.set_ylabel("Time (ms)")
    x = [i for i in range(HOTRUN * N_QUERIES)]
    plt.xticks(x, [i//HOTRUN for i in range(N_QUERIES * HOTRUN)])
    plt.savefig(f'{OUTPUT_FOLDER}/all_queries_time.png')
    plt.clf()

    # Average engine runs
    ax = df.mean().plot(kind='bar', rot=0)
    ax.set_xlabel("Engine")
    ax.set_ylabel("Average time (ms)")
    plt.savefig(f'{OUTPUT_FOLDER}/average_engine_time.png')
    plt.clf()
    print("All finished.")

    # Number of results per query
    x = [i for i in range(N_QUERIES)]
    y= [len(results['milleniumdb'][i]) for i in range(N_QUERIES)]
    plt.bar(x, y)
    plt.xlabel("Query")
    plt.ylabel("Number of results")
    plt.xticks(x, x)
    plt.savefig(f'{OUTPUT_FOLDER}/n_results_per_query.png')


except KeyboardInterrupt:
    print("\nKeyboard Interrupt: Stopping databases.")
    for engine in engine_list:
        engines[engine].stop()