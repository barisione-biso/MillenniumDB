from SPARQLWrapper import SPARQLWrapper, JSON
import multiprocessing
import json
import time


def send_query(i):
    query = """
    SELECT ?s ?p ?o
    WHERE {
        ?s ?p ?o .
    }
    LIMIT 2
    """

    try:
        sparql_wrapper = SPARQLWrapper("http://localhost:8080/sparql")

        sparql_wrapper.setMethod("POST")
        sparql_wrapper.setRequestMethod("postdirectly")
        sparql_wrapper.setReturnFormat(JSON)
        sparql_wrapper.setQuery(query)

        response = sparql_wrapper.query()

        response_json = response.convert()
        print(json.dumps(response_json, indent=4))
        count = len(response_json["results"]["bindings"])
        print("Results:", count)
    except Exception as e:
        print("Exception: " + str(e))
    print(f"Finished thread {i}")


if __name__ == "__main__":
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
    print(f"Finished in {round(end-start, 2)} second(s)")
