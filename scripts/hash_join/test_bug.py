answer_hash = set()
answer_quad = set()

with open(f"tests/outputs/b.txt") as file:
    lines = file.readlines()
    for line in lines[:-3]:
        line = line.strip()
        answer_hash.add(line)

with open(f"tests/outputs/a.txt") as file:
    lines = file.readlines()
    for line in lines[:-3]:
        line = line.strip()
        answer_quad.add(line)

if answer_hash.difference(answer_quad) or answer_quad.difference(answer_hash):
    print("->test incorrecto")
    print("Respuestas en b que no están en a:")
    for answer in answer_hash.difference(answer_quad):
        print(answer)
    print("Respuestas en a que no están en b:")
    for answer in answer_quad.difference(answer_hash):
        print(answer)
else:
    print("->test correcto")
print()
print()