# PYTHON 3
import sys
import os
import random
from random import randint

# PARAMETERS
DISTINCT_LABELS = 100
DISTINCT_KEYS   = 500

NODES_GENERATED = 100000
EDGES_GENERATED = 500000

MIN_LABELS_NODES = 1
MAX_LABELS_NODES = 3


def select_random(list):
    return list[randint(0, len(list)-1)]

# seed random number generator
random.seed(1)

# READ WORDS
words = []

with open("/usr/share/dict/words", mode="r") as words_file:
    for line in words_file:
        words.append(line.strip().replace(',', ""))

def get_random_bool():
    if randint(0, 1) == 0:
        return "true"
    else:
        return "false"

def get_random_int():
    return str(randint(0, 10000))

def get_random_string():
    return '"{}"'.format(select_random(words))

# SELECT RANDOM LABELS
labels = []
for i in range(DISTINCT_LABELS):
    random_label = select_random(words).replace("'", "") # delete '
    while random_label in labels:
        random_label = select_random(words)
    labels.append(random_label)

# SELECT RANDOM KEYS
keys = []
for i in range(DISTINCT_KEYS):
    random_key = select_random(words)
    while random_key in keys:
        random_key = select_random(words)
    keys.append(random_key)

if not os.path.exists("tests/dbs/neo4jcomparison"):
    os.makedirs("tests/dbs/neo4jcomparison")

with open("tests/dbs/neo4jcomparison/neo4j_nodes.csv", mode="w") as neo4j_nodes,\
     open("tests/dbs/neo4jcomparison/neo4j_edges.csv", mode="w") as neo4j_edges,\
     open("tests/dbs/neo4jcomparison/graph",       mode="w") as graph_file:

    neo4j_nodes.write(":ID|iri:STRING|name:STRING|age:LONG|bool:BOOLEAN|:LABEL\n")
    for i in range(NODES_GENERATED):
        name    = get_random_string()
        age     = get_random_int()
        boolean = get_random_bool()
        neo4j_nodes.write("{}|\"Q{}\"|{}|{}|{}|".format(i+1, i+1, name, age, boolean))
        graph_file.write("Q{}".format(i+1))

        # generate random labels
        node_label_count = randint(MIN_LABELS_NODES, MAX_LABELS_NODES)
        node_labels = []
        for l in range(node_label_count):
            label = select_random(labels)
            while label in node_labels:
                label = select_random(labels)
            node_labels.append(label)
            graph_file.write(" :{}".format(label))

        neo4j_nodes.write(";".join(node_labels))
        graph_file.write(" name:{} age:{} bool:{}".format(name, age, boolean))

        graph_file.write("\n")
        neo4j_nodes.write("\n")

    neo4j_edges.write(":START_ID|:END_ID|:TYPE\n")
    for i in range(EDGES_GENERATED):
        from_id = randint(1, NODES_GENERATED)
        to_id   = randint(1, NODES_GENERATED)
        type_id = randint(1, min(1000, NODES_GENERATED))
        neo4j_edges.write("{}|{}|Q{}".format(from_id, to_id, type_id))
        graph_file.write("Q{}->Q{} :Q{}".format(from_id, to_id, type_id))

        graph_file.write("\n")
        neo4j_edges.write("\n")
