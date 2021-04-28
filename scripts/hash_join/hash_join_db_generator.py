## create db for hash join testing
import random
FILE_PATH = "tests/dbs/hash_join_db2.txt"

with open(FILE_PATH, 'w', encoding='utf-8') as output:
    # consulta 1 y 2 vacias

    # consulta 3
    for node in range(100):
        output.write(f'Q{node} :label1\n')

    # consulta 4
    output.write(f'Q{100} :label2 :label3\n')

    #consulta 5
    for node in range(100):
        output.write(f'Q{100}->Q{node} :t5\n')

    # consulta 6
    for node in range(101, 1101):
        output.write(f'Q{node}\n')
        for i in range(101, 1101):
            output.write(f'Q{node}->Q{i+2000} :t5\n')
    for node in range(1101, 20000):
        output.write(f'Q{node}\n')

    # consulta 7
    output.write(f"Q{1000}\n")
    for node in range(10000, 15000):
        output.write(f'Q{node}->Q{1000} :t1\n')
    for node in range(20000):
        output.write(f'Q{1000}->Q{node} :t2\n')

    # consulta 8
    for node in range(20000, 20100):
        output.write(f'Q{node} :label4\n')
        for i in random.choices(range(20000), k=10):
            output.write(f'Q{node}->Q{i} :t3\n') # i
            for j in random.choices(range(20000), k=5):
                output.write(f'Q{i}->Q{j} :t4\n')
