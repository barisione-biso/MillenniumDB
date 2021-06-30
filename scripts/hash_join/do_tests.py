import sys

folder, iterations, alg_name = sys.argv[1:4]
iterations = int(iterations)

with open(f"{folder}/times_{alg_name}.txt") as file:
    lines = file.readlines()
resume_lines = []
while lines:
    current_lines = lines[:iterations]
    lines = lines[iterations:]
    name = None
    time_ = 0
    results = None
    for test in current_lines:
        name2, time_2, results2 = test.strip().split(",")
        if name is None:
            name = name2
            time_ += float(time_2)
            try:
                results = int(results2)
            except:
                results = "SIN RESULTADOS"
        else:
            if name2 != name:
                name = "DISTINTO NOMBRE"
            time_ += float(time_2)
            if results != "SIN RESULTADOS" and int(results2) != results:
                results = "CANTIDAD DISTINTA DE RESULTADOS"
    time_ = time_ / iterations
    resume_lines.append(f"{name},{time_},{results}")

with open(f"{folder}/resumen_{alg_name}.txt", "w", encoding="utf-8") as file:
    for line in resume_lines:
        file.write(line)
        file.write("\n")
