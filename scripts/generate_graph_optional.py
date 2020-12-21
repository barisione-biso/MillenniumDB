import random
from random_word import RandomWords

words = ["A", "B", "C"]
# Constants
N_NODES = 20
N_LABELS = 5
N_KEYS = 3
MIN_LABELS = 1
MAX_LABELS = 3
MIN_KEYS = 1
MAX_KEYS = 3

def get_random_bool():
    if random.randint(0, 1) == 0:
        return "true"
    else:
        return "false"

def get_random_int():
    return str(random.randint(0, 10000))

def get_random_string():
    return '"{}"'.format(random.choice(words))

func_dict = {0: get_random_string, 1: get_random_bool, 2:get_random_int}

dic = {"labels": [], "keys": []}
labels = ["L" + str(i) for i in range(N_LABELS)]
keys =  ["K" + str(i) for i in range(N_KEYS)]


with open("tests/dbs/optional-db.txt", "w") as f:
    for n in range(N_NODES):
        labels_ = random.sample(labels, random.randint(MIN_LABELS, MAX_LABELS))
        keys_ = random.sample(keys, random.randint(MIN_KEYS, MAX_KEYS))
        text = f"Q{n} "
        for l in labels_:
            text += f":{l} "
        for k in keys_:
            text += f"{k}:{func_dict[random.randint(0,2)]()} "
        text = text[:-1] + "\n"
        f.write(text)