from generate_query import *
import subprocess as sp
from SPARQLWrapper import SPARQLWrapper, JSON
import random
import sys
import time
import os

class Graph:
    def __init__(self, n_nodes, n_edges, n_relations):
        self.nodes = []
        self.edges = set()
        self.relations = [f"P{i}" for i in range(n_relations)]
        for i in range(n_nodes):
            self.nodes.append(f"Q{i}")
        i = 0
        while i < n_edges:
            n1 = random.choice(self.nodes)
            p = random.choice(self.relations)
            n2 = random.choice(self.nodes)
            t = (n1, p, n2)
            if t not in self.edges:
                self.edges.add(t)
                i += 1

    def export(self, file, to="milleniumDB"):
        with open(file, 'w') as f:
            if to == "milleniumDB":
                for node in self.nodes:
                    f.write(f"{node} :Person\n")
                for edge in self.edges:
                    n1, p, n2 = edge
                    f.write(f"{n1}->{n2} :{p}\n")
            elif to == "SPARQL":
                f.write("@prefix : <http://www.example.com/> .\n")
                for node in self.nodes:
                    f.write(f":{node} :label :Person .\n")
                for edge in self.edges:
                    n1, p, n2 = edge
                    f.write(f":{n1} :{p} :{n2} .\n")

    def __repr__(self):
        return f"nodes: {len(self.nodes)}\nedges: {len(self.edges)}\nrelations: {self.relations}"
