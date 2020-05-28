node_id = 1
with open("nodes.txt", "w") as nodes_file:
    for i in range(100_000):
        nodes_file.write("({})".format(node_id))
        nodes_file.write(" :LabelA")
        nodes_file.write(" key1:\"value_{}\"".format(node_id) )
        nodes_file.write(" is_pair:")
        if node_id % 2 == 0:
            nodes_file.write("true")
        else:
            nodes_file.write("false")
        nodes_file.write(" mod7:{}".format(node_id%7) )
        nodes_file.write("\n")
        node_id += 1

    for i in range(200_000):
        nodes_file.write("({})".format(node_id))
        nodes_file.write(" :LabelB")
        nodes_file.write(" key1:\"value_{}\"".format(node_id) )
        nodes_file.write(" is_pair:")
        if node_id % 2 == 0:
            nodes_file.write("true")
        else:
            nodes_file.write("false")
        nodes_file.write(" mod7:{}".format(node_id%7) )
        nodes_file.write("\n")
        node_id += 1

    for i in range(300_000):
        nodes_file.write("({})".format(node_id))
        nodes_file.write(" :LabelC")
        nodes_file.write(" key1:\"value_{}\"".format(node_id) )
        nodes_file.write(" is_pair:")
        if node_id % 2 == 0:
            nodes_file.write("true")
        else:
            nodes_file.write("false")
        nodes_file.write(" mod7:{}".format(node_id%7) )
        nodes_file.write("\n")
        node_id += 1

    for i in range(400_000):
        nodes_file.write("({})".format(node_id))
        nodes_file.write(" :LabelD")
        nodes_file.write(" key1:\"value_{}\"".format(node_id) )
        nodes_file.write(" is_pair:")
        if node_id % 2 == 0:
            nodes_file.write("true")
        else:
            nodes_file.write("false")
        nodes_file.write(" mod7:{}".format(node_id%7) )
        nodes_file.write("\n")
        node_id += 1

with open("edges.txt", "w") as edges_file:
    # Node Loops
    for i in range(1, 1_001):
        edges_file.write("({})->({})".format(i, i))
        edges_file.write(" :LabelE")
        edges_file.write(" key1:\"value_{}\"".format(i) )
        edges_file.write(" is_pair:")
        if i % 2 == 0:
            edges_file.write("true")
        else:
            edges_file.write("false")
        edges_file.write("\n")

    for i in range(1_001, 100_001):
        edges_file.write("({})->({})".format(i, i+1))
        edges_file.write(" :LabelE")
        edges_file.write(" key1:\"value_{}\"".format(i) )
        edges_file.write(" is_pair:")
        if i % 2 == 0:
            edges_file.write("true")
        else:
            edges_file.write("false")
        edges_file.write("\n")