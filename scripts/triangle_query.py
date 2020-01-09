from random import seed
from random import randint

# PARAMETERS
N = 100 # each relation will have 2N-1 tuples

with open("nodes.txt", mode="w") as nodes_file:
    for n in range(N):
        nodes_file.write("({}) :Person".format(n+1))
        nodes_file.write("\n")

with open("edges.txt", mode="w") as edges_file:
    # R = (1,1), (1,2),..., (1,N) ; (1,N), (2,N), ... (N,N)
    for n in range(N):
        edges_file.write("({})->({}) :R".format(1, n+1))
        edges_file.write("\n")
    for n in range(1, N):
        edges_file.write("({})->({}) :R".format(n+1, N))
        edges_file.write("\n")

    # S = (1,1), (2,1),..., (N,1) ; (N,1), (N,2), ... (N,N)
    for n in range(N):
        edges_file.write("({})->({}) :S".format(n+1, 1))
        edges_file.write("\n")
    for n in range(1, N):
        edges_file.write("({})->({}) :S".format(N, n+1))
        edges_file.write("\n")

    # T = (1,1), (1,2),..., (1,n) ; (1,n), (2,n), ... (n,n)
    for n in range(N):
        edges_file.write("({})->({}) :T".format(1, n+1))
        edges_file.write("\n")
    for n in range(1, N):
        edges_file.write("({})->({}) :T".format(n+1, N))
        edges_file.write("\n")
