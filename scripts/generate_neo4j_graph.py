# PYTHON 3
import sys
import random
from random import randint

# PARAMETERS
DISTINCT_LABELS = 100
DISTINCT_KEYS = 500

NODES_GENERATED = 100000
EDGES_GENERATED = 500000

MIN_LABELS_NODES = 1
MAX_LABELS_NODES = 3

# MIN_LABELS_EDGES = 1
# MAX_LABELS_EDGES = 1

# NO QUALIFIERS ARE ALLOWED
# MIN_QUALIFIERS = 0
# MAX_QUALIFIERS = 5

# MIN_PROPERTIES_NODES = 2
# MAX_PROPERTIES_NODES = 6

# MIN_PROPERTIES_EDGES = 1
# MAX_PROPERTIES_EDGES = 4

def select_random(list):
    return list[randint(0, len(list)-1)]

# seed random number generator
random.seed(1)

# READ WORDS
words = []

if len(sys.argv) != 3:
    print("usage: python3 generate_neo4j_graph nodes_filename edges_filename")
    exit(1)

with open("/usr/share/dict/words", mode="r") as words_file:
    for line in words_file:
        words.append(line.strip())

def select_random_value():
    value_type = randint(0, 3)
    if value_type == 0:   # BOOL
        if randint(0, 1) == 0:
            return "true"
        else:
            return "false"
    elif value_type == 1: # INT
        return str(randint(0, 10000))
    elif value_type == 2: # FLOAT
        return str(random.uniform(0, 10000))
    else:                 # STRING
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

with open(sys.argv[1], mode="w") as nodes_file:
    nodes_file.write(":ID|name:STRING|:LABEL\n")
    for i in range(NODES_GENERATED):
        nodes_file.write("{}|\"Q{}\"|".format(i+1, i+1))

        # generate random labels
        node_label_count = randint(MIN_LABELS_NODES, MAX_LABELS_NODES)
        node_labels = []
        for l in range(node_label_count):
            label = select_random(labels)
            while label in node_labels:
                label = select_random(labels)
            node_labels.append(label)
        nodes_file.write(";".join(node_labels))

        # TODO: generate random properties

        nodes_file.write("\n")

with open(sys.argv[2], mode="w") as edges_file:
    edges_file.write(":START_ID|:END_ID|:TYPE\n")
    for i in range(EDGES_GENERATED):
        random_label = select_random(labels)
        edges_file.write("{}|{}|{}".format(randint(1, NODES_GENERATED), randint(1, NODES_GENERATED), random_label))

        # TODO: generate random properties
        edges_file.write("\n")
