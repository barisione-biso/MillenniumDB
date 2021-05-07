from random import seed
from random import randint

# PARAMETERS
N = 500 # each relation will have 2N-1 tuples

with open("tests/dbs/triangle.txt", mode="w") as output_file:
    for n in range(N):
        output_file.write("Q{} :Person".format(n+1))
        output_file.write("\n")

    # R = (1,1), (1,2),..., (1,N) ; (1,N), (2,N), ... (N,N)
    for n in range(N):
        output_file.write("Q{}->Q{} :R".format(1, n+1))
        output_file.write("\n")
    for n in range(1, N):
        output_file.write("Q{}->Q{} :R".format(n+1, N))
        output_file.write("\n")

    # S = (1,1), (2,1),..., (N,1) ; (N,1), (N,2), ... (N,N)
    for n in range(N):
        output_file.write("Q{}->Q{} :S".format(n+1, 1))
        output_file.write("\n")
    for n in range(1, N):
        output_file.write("Q{}->Q{} :S".format(N, n+1))
        output_file.write("\n")

    # T = (1,1), (1,2),..., (1,n) ; (1,n), (2,n), ... (n,n)
    for n in range(N):
        output_file.write("Q{}->Q{} :T".format(1, n+1))
        output_file.write("\n")
    for n in range(1, N):
        output_file.write("Q{}->Q{} :T".format(n+1, N))
        output_file.write("\n")
