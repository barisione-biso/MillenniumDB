## Se asume que se generaron los archivos out_hash{i} y out_ordered{i} previamente
## TODO:
## automatizar generación de archivos en un solo script

for i in range(1, 13):
    answer_hash = set()
    answer_ordered = set()
    print(f"Test {i}")
    with open(f"tests/outputs/out_hash{i}.txt") as file:
        lines = file.readlines()
        for line in lines:
            if line[0] != "{":
                print("Hash", lines[-2], end="")
                break
            line = line.strip()
            answer_hash.add(line)

    with open(f"tests/outputs/out_ordered{i}.txt") as file:
        lines = file.readlines()
        for line in lines:
            if line[0] != "{":
                print("Ordered", lines[-2], end="")
                break
            line = line.strip()
            answer_ordered.add(line)

    if answer_hash.difference(answer_ordered) or answer_ordered.difference(answer_hash):
        print("test incorrecto")
        print("Respuestas en hash que no están en ordered:")
        print(answer_hash.difference(answer_ordered))
        print("Respuestas en ordered que no están en hash:")
        print(answer_ordered.difference(answer_hash))
    else:
        print("test correcto")
    print()
