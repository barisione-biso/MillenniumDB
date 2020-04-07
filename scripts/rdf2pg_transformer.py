# python 3
# usage: python3 rdf2pg_transformer [input.ypg]
import sys
import re

# edge_pattern = re.compile('\((\d+)\)-\[(.+)\]->\((\d+)\)')

with open(sys.argv[1], mode="r") as rdf2pg_file:
    with open("nodes.txt", mode="w") as nodes_file:
        with open("edges.txt", mode="w") as edges_file:
            for line in rdf2pg_file:
                if line[0] == '(': # EDGE (id1)-[label {key1:value1,key2:value2}]->(id2)
                    m = re.match(r"\((\d+)\)-\[(.+)\]->\((\d+)\)", line)
                    if m:
                        edges_file.write("({})->({})".format(m.group(1), m.group(3)))
                        m2 = re.match(r"(\w+)( \{(.*)\})?", m.group(2))
                        if m2:
                            edges_file.write(" :{}".format(m2.group(1)))
                            if m2.group(3):
                                # TODO: por ahora simplemente reemplazar ',' por espacio, podría modificar strings
                                edges_file.write(" {}".format(m2.group(3).replace(",", " ")))
                        else:
                            print("Failed to match edge(2)", m.group(1))
                    else:
                        print("Failed to match edge", m.group(1))
                    edges_file.write("\n")
                else: # NODE
                    m = re.match(r"(\d+)\[(.+)?\]:\{(.*)\}", line)
                    if m:
                        nodes_file.write("({})".format(m.group(1)))
                        if m.group(2):
                            for label in m.group(2).split(','):
                                nodes_file.write(" :{}".format(label))
                        if m.group(3):
                            nodes_file.write(" {}".format(m.group(3).replace(",", " ")))
                    else:
                        print("Failed to match node", m.group(1))
                    nodes_file.write("\n")
