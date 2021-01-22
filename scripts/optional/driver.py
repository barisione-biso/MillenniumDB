from wrapper import *
from graph import *
import time

def compare_dicts(d1, d2):
    return sorted(d1.items()) == sorted(d2.items())

# Graph creation
POSSIBLE_RELATIONS = ["acquintanceOf", "followerOf", "friendOf", "worksWith", "livesWith", "parentOf"]
MIN_NODES = 15
MAX_NODES = 20
MIN_EDGES = 10
MAX_EDGES = 20
MIN_RELATIONS = 2
MAX_RELATIONS= 2

# Query creation
N_VARS = 4
MAX_CHILDS = 4
MAX_DEPTH = 3

def create_database():
    option = input(f"Use default parameters ({MIN_NODES}-{MAX_NODES} nodes, {MIN_EDGES}-{MAX_EDGES} edges, {MIN_RELATIONS}-{MAX_RELATIONS} relations)? (y/n): ")
    if option == "y":
        min_nodes = MIN_NODES
        max_nodes = MAX_NODES
        min_edges = MIN_EDGES
        max_edges = MAX_EDGES
        min_relations = MIN_RELATIONS
        max_relations = MAX_RELATIONS
    else:
        print("Parameters:")
        min_nodes = int(input("min nodes: "))
        max_nodes = int(input("max nodes: "))
        min_edges = int(input("min edges: "))
        max_edges = int(input("max edges: "))
        min_relations = int(input("min relations: "))
        max_relations = int(input("max relations: "))

    sparql_path = input("path to save SPARQL database: ")
    if sparql_path == "":
        return
    mdb_path = input("path to save MilleniumDB database: ")
    if mdb_path == "":
        return

    # Create graph
    print("Creating graph...")
    s = time.time() # time execution
    g = Graph(min_nodes = min_nodes,
            max_nodes = max_nodes,
            min_edges = min_edges,
            max_edges = max_edges,
            min_relations = min_relations,
            max_relations= max_relations)
    print("Graph created in",time.time()-s,"seconds.")
    print(g)
    print("Saving files...")

    # Export to MDB and SPARQL
    # g.export("milleniumdb-db.txt", to="milleniumDB")
    # g.export("sparql-db.txt", to="SPARQL")
    g.export(mdb_path, to="milleniumDB")
    g.export(sparql_path, to="SPARQL")
    print("Files saved.")

def import_database():

    sparql_path = input("path to load SPARQL database: ")
    mdb_path = input("path to load MilleniumDB database: ")
    mdb_db_path = input("path to run MilleniumDB database: ")

    if mdb_path == "" or sparql_path == "" or mdb_db_path == "":
        return

    # Clear previous database
    sparql_clear()
    milleniumdb_clear(mdb_db_path)

    # LOAD SPARQL
    with open(sparql_path, 'r') as f:
        data = f.read()
        sparql_update(data)

    # LOAD MDB
    # milleniumdb_update("milleniumdb-db.txt", "./tests/dbs/test")
    milleniumdb_update(mdb_path, mdb_db_path)

    print(f"Please restart MilleniumDB server with the following command: 'build/Debug/bin/server -d {mdb_db_path}'")

    # Get number of relations (Needed for query creation)
    with open(mdb_path, 'r') as f:
        lines = f.readlines()
    relations = set()
    for l in lines:
        l = l.strip("\n")
        if '->' in l:
            index = l.find(":")
            relations.add(l[index+1:])
    return list(relations), mdb_db_path

def create_query(relations):
    if relations is None:
        print("Can't create query without importing graph first.")
        return

    sparql_query_path = input("Sparql query path: ")
    mdb_query_path = input("MilleniumDB query path: ")
    if sparql_query_path == "" or mdb_query_path == "":
        return

    option = input(f"Use default parameters (N° possible vars: {N_VARS}, N° max children per node: {MAX_CHILDS}, Max depth: {MAX_DEPTH}? (y/n): ")
    if option == "y":
        n_vars = N_VARS
        max_childs = MAX_CHILDS
        max_depth = MAX_DEPTH
    else:
        print("Parameters:")
        n_vars = int(input("N° possible vars: "))
        max_childs = int(input("N° max children per node: "))
        max_depth = int(input("Max depth: "))
    
    # Create variables
    variable_letters = set(string.ascii_lowercase)
    i = 0
    while len(variable_letters) < n_vars:
        variable_letters = variable_letters.union(set(f"{x}{i}" for x in string.ascii_lowercase))
        i += 1
    variables = set(f"?{i}" for i in random.sample(variable_letters, n_vars))


    # Create Optional Query Tree
    OptNode.max_childs = max_childs
    OptNode.max_depth = max_depth
    print("RELATIONS!", relations)
    n = OptNode(variables, relations=relations, root=True)

    # Generate children
    n.generate_children(MAX_DEPTH)
    n.display()

    # Generate milleniumDB query
    milleniumdb_query = n.generate_query(output="milleniumdb")
    print("MilleniumDB query generated:")
    print(milleniumdb_query)

    # Generate sparql query
    sparql_query = n.generate_query(output="sparql")
    print("\nSPARQL query generated:")
    print(sparql_query)

    # Export queries
    n.export(mdb_query_path, "milleniumdb")
    n.export(sparql_query_path, "sparql")

    return milleniumdb_query, sparql_query

def import_query():
    # Import queries' path
    
    sparql_query_path = input("Sparql query path: ")
    mdb_query_path = input("MilleniumDB query path: ")
    if sparql_query_path == "" or mdb_query_path == "":
        return None, None
    return mdb_query_path, sparql_query_path

def execute_query(milleniumdb_db_path, milleniumdb_query_path, sparql_query_path):
    if milleniumdb_db_path is None:
        print("No MilleniumDB database folder.")
        return
    # SPARQL data
    with open(sparql_query_path, 'r') as f:
        query = f.read()

    sparql_query_results = sparql_query(query)
    milleniumdb_query_results = milleniumdb_query(milleniumdb_query_path, milleniumdb_db_path)
    """
    print(f"SPARQL: {len(sparql_query_results)} results found.")
    print(f"MilleniumDB: {len(milleniumdb_query_results)} results found.")
    if input("show_results (y/n)? :") == "y":
        print(f"MilleniumDB results ({len(milleniumdb_query_results)}):")
        for r in milleniumdb_query_results:
            print(r)
        print(f"\nSPARQL results ({len(sparql_query_results)}):")
        for r in sparql_query_results:
            print(r)
    """
    compare_query_results(milleniumdb_query_results, sparql_query_results)
    return milleniumdb_query_results, sparql_query_results

def check_queries(milleniumdb_query_path, sparql_query_path):
    with open(sparql_query_path, 'r') as f:
        sparql_query = f.read()
    with open(milleniumdb_query_path, 'r') as f:
        milleniumdb_query = f.read()

    print("MilleniumDB query generated:")
    print(milleniumdb_query)
    print("\nSPARQL query generated:")
    print(sparql_query)

def check_query_results(milleniumdb_results, sparql_results):
    if milleniumdb_results is None or sparql_results is None:
        print("No valid query executed.")
        return
    print(f"MilleniumDB results ({len(milleniumdb_results)}):")
    for r in milleniumdb_results:
        print(r)
    print(f"\nSPARQL results ({len(sparql_results)}):")
    for r in sparql_results:
        print(r)

def compare_query_results(milleniumdb_results, sparql_results):
    n_results_mdb = len(milleniumdb_results)
    n_results_sparql = len(sparql_results)
    print(f"SPARQL: {len(sparql_results)} results found.")
    print(f"MilleniumDB: {len(milleniumdb_results)} results found.")
    if n_results_mdb != n_results_sparql:
        option = input("Different number of results. Continue comparison? (y(n): ")
        if option == "n":
            return
    missing_results = 0
    for result in sparql_results:
        if result not in milleniumdb_results:
            print(f"{result} not found in milleniumDB results.")
            missing_results += 1
    if missing_results == 0 and len(sparql_results) == len(milleniumdb_results):
        print("Equivalent results.")
        return
    extra_results = len(milleniumdb_results) - (len(sparql_results) - missing_results)
    print(f"missing results: {missing_results} | extra results: {extra_results}.")
    if input("Show results? (y/n): ") == "n":
        return
    print("\nMilleniumDB results:")
    for r in milleniumdb_results:
        print(r)
    print("\nSparql Results:")
    for r in sparql_results:
        print(r)

if __name__ == "__main__":
    relations = None
    milleniumdb_query_body, sparql_query_body = None, None
    milleniumdb_results, sparql_results = None, None
    milleniumdb_db_path = None
    print("Optional Query Checker. Make sure blazegraph server is running (command: 'java -server -Xmx4g -jar ../../blazegraph/blazegraph.jar')")
    while True:
        print("-" * 80)
        print("Choose one:")
        print("(1) Create database")
        print("(2) Import database")
        print("(3) Create query")
        print("(4) Import query")
        print("(5) Execute query")
        print("(6) Check queries")
        print("(7) Check query results")
        print("(8) Compare query results")
        print("(9) Exit")
        option = input("Select number and press ENTER: ")
        print("\n")
        if option == "1":
            create_database()
        elif option == "2":
            relations, milleniumdb_db_path = import_database()
        elif option == "3":
            milleniumdb_query_body, sparql_query_body = create_query(relations)
        elif option == "4":
            mdb_query_path, sparql_query_path = import_query()
            if mdb_query_path is not None:
                milleniumdb_query_path, sparql_query_path = mdb_query_path, sparql_query_path
        elif option == "5":
            #try:
            milleniumdb_results, sparql_results = execute_query(milleniumdb_db_path, milleniumdb_query_path, sparql_query_path)
            #except Exception:
            #    print("SPARQL query malformed. Please create a new query and run the server once again.")
            #    print("command: 'java -server -Xmx4g -jar ../../blazegraph/blazegraph.jar'")
        elif option == "6":
            check_queries(milleniumdb_query_path, sparql_query_path)
        elif option == "7":
            check_query_results(milleniumdb_results, sparql_results)
        elif option == "8":
            compare_query_results(milleniumdb_results, sparql_results)
        elif option == "9":
            break

