## create clique db with same nodes as connection type (n**3 connections)
FILE_PATH = "tests/dbs/distinct_db.txt"
NODES = 200


nodes = [f'Q{i}' for i in range(NODES)]

with open(FILE_PATH, 'w', encoding='utf-8') as output:
    n = 0
    end_constants = NODES/10
    for node in nodes:
        if n < end_constants:
            output.write(f'{node} constant:10\n')
        else:
            output.write(f'{node}\n')
        n += 1
    for node1 in nodes:
        for node2 in nodes:
            for node3 in nodes:
                output.write(f'{node1}->{node2} :{node3}\n')

