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

    # consultas cx ( 90%-70%-50%-30%-10% de los nodos hace join)
    # MATCH (?x)-[:ta]->(?y)-[:t90b]->(?z)
    T = 1000000 # probar hasta que el tiempo de ejecucion sea cercano a 1 segundo de alguna consulta
    # siempre hay T conexiones a y b,
    # opcion a) si se escogen x, y, z nunca se vuelven a usar. cada 4 (1->2 y 2->3 o 1->2 y 3->4)
    # opcion b) con reposicion (al azar escogemos 4 distintos)

    # sin reposicion
    for i in range(T):
        t = 4 * i
        output.write(f'N{t}->N{t + 1} :ta\n')
        if random.random() < 0.9:
            output.write(f'N{t + 1}->N{t + 2} :t90b\n')
        else:
            output.write(f'N{t + 2}->N{t + 3} :t90b\n')
        if random.random() < 0.7:
            output.write(f'N{t + 1}->N{t + 2} :t70b\n')
        else:
            output.write(f'N{t + 2}->N{t + 3} :t70b\n')
        if random.random() < 0.5:
            output.write(f'N{t + 1}->N{t + 2} :t50b\n')
        else:
            output.write(f'N{t + 2}->N{t + 3} :t50b\n')
        if random.random() < 0.3:
            output.write(f'N{t + 1}->N{t + 2} :t30b\n')
        else:
            output.write(f'N{t + 2}->N{t + 3} :t30b\n')
        if random.random() < 0.1:
            output.write(f'N{t + 1}->N{t + 2} :t10b\n')
        else:
            output.write(f'N{t + 2}->N{t + 3} :t10b\n')
    # con reposicion
    for i in range(T):
        a, b, c, d = [x for x in random.choices(range(T * 4), k=4)]
        output.write(f'NR{a}->NR{b} :tra\n')
        if random.random() < 0.9:
            output.write(f'NR{b}->NR{c} :tr90b\n')
        else:
            output.write(f'NR{c}->NR{d} :tr90b\n')
        if random.random() < 0.7:
            output.write(f'NR{b}->NR{c} :tr70b\n')
        else:
            output.write(f'NR{c}->NR{d} :tr70b\n')
        if random.random() < 0.5:
            output.write(f'NR{b}->NR{c} :tr50b\n')
        else:
            output.write(f'NR{c}->NR{d} :tr50b\n')
        if random.random() < 0.3:
            output.write(f'NR{b}->NR{c} :tr30b\n')
        else:
            output.write(f'NR{c}->NR{d} :tr30b\n')
        if random.random() < 0.1:
            output.write(f'NR{b}->NR{c} :tr10b\n')
        else:
            output.write(f'NR{c}->NR{d} :tr10b\n')
