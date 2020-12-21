import sys
import random


'''
Create BD EXAMPLE
python3 scripts/order_db_tests.py create 10000 tests/dbs/order_ex.txt 8000
output -> A file that can be used to create a db MilleniumDB with 1.000.000 tuples with
a key value between 1 and 800.000


Check OUTPUT EXAMPLE
python3 scripts/order_db_tests.py check out_all.txt out.txt 

Before you have to run a query:
SELECT ?x, ?x.value
MATCH (?x)

And save the output in out_all.txt or the name that you choose. Then you have to execute the query
SELECT ?x.v, ?x.value
MATCH (?x)
ORDER BY ?x.value

And save the output in out.txt

That scripts checks that the values in out.txt have the correct order and all the tuples in out_all.txt
exists in out.txt.

'''


def create_test_db():
  n_tuples = int(sys.argv[2])
  name_db = sys.argv[3]
  max_value = int(sys.argv[4])
  with open(name_db, 'w') as file:
    for i in range(1, n_tuples + 1):
      tup = f'Q{i} value:{random.randint(1, max_value)} v:{random.randint(1, max_value)}\n'
      file.write(tup)


def check_output():
  all_tuples_file = sys.argv[2]
  out_file = sys.argv[3]
  all_set = set()
  out_set = set()
  out_list = list()
  all_list = list()
  with open(all_tuples_file, 'r') as file:
    for i in list(file.readlines()[:-3]):
      all_set.add(i.strip().split(',')[0].split(':')[1])
      all_list.append(int(i.strip().split(',')[1].split(':')[1][:-1]))
  with open(out_file, 'r' ) as file:
    for i in list(file.readlines()[:-3]):
      out_set.add(i.strip().split(',')[0].split(':')[1])
      out_list.append(int(i.strip().split(',')[1].split(':')[1][:-1]))
  order_out = sorted(out_list)
  if order_out != out_list:
      for i in range(len(order_out)):
        if order_out[i] != out_list[i]:
          print(order_out[i], out_list[i], i)
      print('Order Error')
      return
  if out_set != all_set or len(all_list) != len(out_list):
    print('Missing Tuples')




if __name__ == '__main__':
  action = sys.argv[1]
  if action == 'create':
    create_test_db()
  elif action == 'check':
    check_output()


