from SPARQLWrapper import SPARQLWrapper, JSON
import multiprocessing
import time

def send_query(i):
    query= 'SELECT * WHERE { ?s ?p ?o . } LIMIT 500'

    try:
        sparql_wrapper = SPARQLWrapper('http://localhost:8080/sparql')

        # sparql_wrapper.setMethod('POST')
        # sparql_wrapper.setRequestMethod('postdirectly')

        sparql_wrapper.setReturnFormat(JSON)
        sparql_wrapper.setQuery(query)

        results = sparql_wrapper.query()

        # print(results)
        count = 0
        json_results = results.convert()
        print(json_results)
        for _ in json_results["results"]["bindings"]:
            count += 1
        print("results:", count)
    except Exception as e:
        print(e)
    print(f'finished thread {i}')

if __name__ == '__main__':
    start = time.perf_counter()
    p1 = multiprocessing.Process(target=send_query, args=[1])
    # p2 = multiprocessing.Process(target=send_query, args=[2])
    # p3 = multiprocessing.Process(target=send_query, args=[3])
    # p4 = multiprocessing.Process(target=send_query, args=[4])
    # p5 = multiprocessing.Process(target=send_query, args=[5])
    # p6 = multiprocessing.Process(target=send_query, args=[6])
    # p7 = multiprocessing.Process(target=send_query, args=[7])
    p1.start()
    # p2.start()
    # p3.start()
    # p4.start()
    # p5.start()
    # p6.start()
    # p7.start()
    p1.join()
    # p2.join()
    # p3.join()
    # p4.join()
    # p5.join()
    # p6.join()
    # p7.join()
    end = time.perf_counter()
    print(f'Finished in {round(end-start, 2)} second(s)')