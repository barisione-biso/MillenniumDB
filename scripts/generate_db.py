import sys
from random import randint, choice

"""
* Args:
-> path_to_output: path to '.txt' file
-> size: amount of nodes
-> graph_type: clique, line, bipartite, cyclic
-> multiple: indicates multiple connection types (0: single, 1: multiple)
"""

try:
    file_path, size, graph_type, multiple = sys.argv[1:5]
    graph = {}
    for index in range(int(size)):
        graph[f'Q{index}'] = []
    if graph_type == 'clique':
        for node in graph:
            graph[node] += graph.keys()
    elif graph_type == 'line':
        for node in list(graph.keys())[:-1]:
            graph[node].append(f'Q{int(node[1:]) + 1}')
    elif graph_type == 'bipartite':
        for index in range(int(size)):
            identifier = f'Q{index}'
            if index <= int(size) / 2:
                graph[identifier].append(f'Q{randint(0, int(int(size)/2))}')
            else:
                graph[identifier].append(
                    f'Q{randint(int(int(size)/2) + 1, int(size) - 1)}')
    elif graph_type == 'cyclic':
        for node in list(graph.keys())[:-1]:
            graph[node].append(f'Q{int(node[1:]) + 1}')
        for node in graph.keys():
            graph[node].append(f'Q{randint(0, int(node[1:]))}')
    node_type = 'person'
    connection_types = ['knows', 'follows']
    with open(file_path, 'w', encoding='utf-8') as output:
        for node in graph:
            output.write(f'{node} :{node_type}\n')
        for node in graph:
            for connected_node in graph[node]:
                current_type = 'knows'
                if int(multiple):
                    current_type = choice(connection_types)
                output.write(f'{node}->{connected_node} :{current_type}\n')
except Exception as e:
    print(e)
