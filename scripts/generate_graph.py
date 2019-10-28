from random import seed
from random import randint

# PARAMETERS
NODES_GENERATED = 100000
EDGES_GENERATED = 1000000

MIN_LABELS_NODES = 1
MAX_LABELS_NODES = 5

MIN_LABELS_EDGES = 1
MAX_LABELS_EDGES = 3

MIN_PROPERTIES_NODES = 2
MAX_PROPERTIES_NODES = 10

MIN_PROPERTIES_EDGES = 2
MAX_PROPERTIES_EDGES = 5


# seed random number generator
seed(1)

# READ WORDS
words = []
with open("/usr/share/dict/words", mode="r") as words_file:
    for line in words_file:
        words.append(line.strip())
word_count = len(words)

with open("nodes.txt", mode="w") as nodes_file:
    for i in range(NODES_GENERATED):
        nodes_file.write("({})".format(i+1))

        # generate random labels
        node_label_count = randint(MIN_LABELS_NODES, MAX_LABELS_NODES)
        labels_already_added = []
        for l in range(node_label_count):
            label = words[randint(0, word_count-1)]
            while label in labels_already_added:
                label = words[randint(0, word_count-1)]
            nodes_file.write(" :{}".format(label))

        # generate random properties
        node_property_count = randint(MIN_PROPERTIES_NODES, MAX_PROPERTIES_NODES)
        keys_already_added = []
        for l in range(node_property_count):
            key = words[randint(0, word_count-1)]
            while key in keys_already_added:
                key = words[randint(0, word_count-1)]
            value = words[randint(0, word_count-1)]
            nodes_file.write(" {}:{}".format(key, value))
        nodes_file.write("\n")

with open("edges.txt", mode="w") as edges_file:
    for i in range(EDGES_GENERATED):
        edges_file.write("({})->({})".format(randint(1, NODES_GENERATED), randint(1, NODES_GENERATED)))

        # generate random labels
        edge_label_count = randint(MIN_LABELS_EDGES, MAX_LABELS_EDGES)
        labels_already_added = []
        for l in range(edge_label_count):
            label = words[randint(0, word_count-1)]
            while label in labels_already_added:
                label = words[randint(0, word_count-1)]
            edges_file.write(" :{}".format(label))

        # generate random properties
        edge_property_count = randint(MIN_PROPERTIES_EDGES, MAX_PROPERTIES_EDGES)
        keys_already_added = []
        for l in range(edge_property_count):
            key = words[randint(0, word_count-1)]
            while key in keys_already_added:
                key = words[randint(0, word_count-1)]
            value = words[randint(0, word_count-1)]
            edges_file.write(" {}:{}".format(key, value))
        edges_file.write("\n")
