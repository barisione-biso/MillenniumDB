import random
import string

def aux_transform(l):
    # Transforms a list from [a, b, c, d] to [(a, b), (b, c), (c,d)]
    new_l = []
    for i in range(len(l) - 1):
        new_l.append((l[i], l[i+1]))
    return new_l


class OptNode:

    def __init__(self, variables, relations, root=False, parent=None, n_vars=None):
        if n_vars is not None:
            # Create variables
            variable_letters = set(string.ascii_lowercase)
            i = 0
            while len(variable_letters) < n_vars:
                variable_letters = variable_letters.union(set(f"{x}{i}" for x in string.ascii_lowercase))
                i += 1
            variables = set(f"?{i}" for i in random.sample(variable_letters, n_vars))
        self.variables = variables
        self.relations = relations
        self.used_relations = []
        self.root = root
        self.children = []
        self.used_variables = random.sample(self.variables, random.randint(2, len(self.variables)))
        for _ in range(len(self.used_variables)):
            self.used_relations.append(random.choice(self.relations))
        self.parent = parent
        if self.root:
            self.generate_children(OptNode.max_depth, well_defined=True)

    def generate_children(self, depth, well_defined = False):

        if depth == 0:
            return

        # For each child
        for _ in range(random.randint(OptNode.min_childs, OptNode.max_childs)):

            # Get variables
            if not well_defined:
                child_variables = set(random.sample(self.variables, random.randint(2, len(self.variables))))
            else:
                child_variables = set(random.sample(self.used_variables, random.randint(2, len(self.used_variables))))

            # Create child
            child = OptNode(child_variables, self.relations, root=False, parent=self)

            # Create its children recursively
            child.generate_children(depth - 1, well_defined=well_defined)

            # Get its variables
            child_used_variables = set(child.get_used_variables())

            # In order to ensure Quasi-well defined queries, we cant
            # create new children with variables that were not used by
            # antecessors or self and that were used by children,
            # so we update available variables
            antecessor_variables = set(self.get_used_variables_parent())
            self.variables = (antecessor_variables - child_used_variables).union(antecessor_variables.intersection(child_used_variables))

            # Append child
            self.children.append(child)

    def generate_query(self, output="milleniumdb", spacing=0):
        linestart = " " * spacing * 10
        if output == "milleniumdb":
            query = ""
            if self.root:
                query = "SELECT"
                for variable in self.get_used_variables():
                    query += f" {variable},"
                query = query[:-1] + "\nMATCH "
            for i, variable in enumerate(self.used_variables): # shuffle in-place
                relation = self.used_relations[i]
                query += f"({variable})-[:{relation}]->"
            query = query[:-(len(relation)+6)]
            for child in self.children:
                query += "\n" + linestart + "OPTIONAL {" + child.generate_query(output, spacing + 1)
            if not self.root:
                if len(self.children) > 0: # TODO: This should always happen
                    query += "\n" + linestart + "}"
                else:
                    query += "}"
        elif output == "sparql":
            query = ""
            if self.root:
                query = "PREFIX : <http://www.example.com/>\nSELECT"
                for variable in self.get_used_variables():
                    query += f" {variable} "
                query = query[:-1] + "\nWHERE {"
            i = 0
            for variable_1, variable_2 in aux_transform(self.used_variables):
                relation = self.used_relations[i]
                query += "\n" + linestart + f"{variable_1} :{relation} {variable_2} ."
                i += 1
            for child in self.children:
                query += "\n" + linestart + "OPTIONAL {" + child.generate_query(output, spacing + 1)
            if len(self.children) > 0: # TODO: This should always happen
                query += "\n" + linestart + "}"
                if not self.root:
                    query += " ."
            else:
                query += "}"
                if not self.root:
                    query += " ."
        return query

    def display(self, spacing=0):
        print("\t"*spacing, self.variables)
        for child in self.children:
            child.display(spacing + 1)

    def get_used_variables(self):
        variables = set(self.used_variables)
        for child in self.children:
            variables = variables.union(set(child.get_used_variables()))
        return variables

    def get_used_variables_parent(self):
        if self.parent is None:
            return set(self.used_variables)
        else:
            return set(self.used_variables).union(self.parent.get_used_variables_parent())

    def export(self, file, output="milleniumdb"):
        with open(file, 'w') as f:
            f.write(self.generate_query(output))
