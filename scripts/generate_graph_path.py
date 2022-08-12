# PYTHON 3
import sys
import random

SCALE_PARAM = 1000

with open(f"graph_path_{SCALE_PARAM}.txt", mode="w") as graph_mdb_file, \
     open(f"graph_path_{SCALE_PARAM}_Neo4J_nodes.csv", mode="w")  as graph_neo4j_nodes_file, \
     open(f"graph_path_{SCALE_PARAM}_Neo4J_edges.csv", mode="w")  as graph_neo4j_edges_file, \
     open(f"graph_path_{SCALE_PARAM}.nt", mode="w")  as graph_nt_file:

    graph_neo4j_nodes_file.write(":ID|iri:STRING\n")
    graph_neo4j_edges_file.write(":START_ID|:END_ID|:TYPE\n")

    graph_neo4j_nodes_file.write(f"0|\"<http://e.cl/N0>\"\n")

    for i in range(0, SCALE_PARAM, 3):
        # HORIZONTAL DIAMOND:
        # (N_i)-[:A]->(N_i+1)
        # (N_i)-[:A]->(N_i+2)
        # (N_i+1)-[:B]->(N_i+3)
        # (N_i+2)-[:B]->(N_i+3)
        graph_mdb_file.write(f'N{i}->N{i+1} :A\n')
        graph_mdb_file.write(f'N{i}->N{i+2} :A\n')
        graph_mdb_file.write(f'N{i+1}->N{i+3} :B\n')
        graph_mdb_file.write(f'N{i+2}->N{i+3} :B\n')

        graph_nt_file.write(f'<http://e.cl/N{i}> <http://e.cl/A> <http://e.cl/N{i+1}> .\n')
        graph_nt_file.write(f'<http://e.cl/N{i}> <http://e.cl/A> <http://e.cl/N{i+2}> .\n')
        graph_nt_file.write(f'<http://e.cl/N{i+1}> <http://e.cl/B> <http://e.cl/N{i+3}> .\n')
        graph_nt_file.write(f'<http://e.cl/N{i+2}> <http://e.cl/B> <http://e.cl/N{i+3}> .\n')

        graph_neo4j_nodes_file.write(f"{i+1}|\"<http://e.cl/N{i+1}>\"\n")
        graph_neo4j_nodes_file.write(f"{i+2}|\"<http://e.cl/N{i+2}>\"\n")
        graph_neo4j_nodes_file.write(f"{i+3}|\"<http://e.cl/N{i+3}>\"\n")

        graph_neo4j_edges_file.write(f'{i}|{i+1}|A\n')
        graph_neo4j_edges_file.write(f'{i}|{i+2}|A\n')
        graph_neo4j_edges_file.write(f'{i+1}|{i+3}|B\n')
        graph_neo4j_edges_file.write(f'{i+2}|{i+3}|B\n')


    for i in range(SCALE_PARAM):
        # VERTICAL DIAMOND + LINE
        # (S)-[:A]->(X_i)
        # (S)-[:A]->(Y_i)
        # (X_i)-[:B]->(E_0)
        # (Y_i)-[:B]->(E_0)
        # (E_i)-[:C]->(E_i+1)
        graph_mdb_file.write(f'S->X{i} :A\n')
        graph_mdb_file.write(f'S->Y{i} :A\n')
        graph_mdb_file.write(f'X{i}->E0 :B\n')
        graph_mdb_file.write(f'Y{i}->E0 :B\n')
        graph_mdb_file.write(f'E{i}->E{i+1} :C\n')

        graph_nt_file.write(f'<http://e.cl/S> <http://e.cl/A> <http://e.cl/X{i+1}> . \n')
        graph_nt_file.write(f'<http://e.cl/S> <http://e.cl/A> <http://e.cl/Y{i+2}> . \n')
        graph_nt_file.write(f'<http://e.cl/X{i}> <http://e.cl/B> <http://e.cl/E0> . \n')
        graph_nt_file.write(f'<http://e.cl/Y{i}> <http://e.cl/B> <http://e.cl/E0> . \n')
        graph_nt_file.write(f'<http://e.cl/E{i}> <http://e.cl/C> <http://e.cl/E{i+1}> . \n')


