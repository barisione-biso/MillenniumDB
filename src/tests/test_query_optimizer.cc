#include "test_query_optimizer.h"

#include "base/graph/value/value_string.h"
#include "relational_model/config.h"
#include "relational_model/graph/relational_graph.h"
#include "relational_model/query_optimizer/query_optimizer.h"
#include "relational_model/query_optimizer/query_optimizer_label.h"
#include "relational_model/query_optimizer/query_optimizer_property.h"
#include "relational_model/query_optimizer/query_optimizer_connection.h"

void TestQueryOptimizer::Test1() {
    // This test shows how the next query should execute. (replacing literals with values that actually
    // exists in the databasa example)

    // SELECT ?n.name
    // MATCH (?n:Person {age:"65"} )

    // Label(?n, Person)
    // Property(?n, age, 65)
    // Property(?n, name, ?n.name)

    Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);
    QueryOptimizer optimizer{};

    VarId n        { 0 }; // ?n
    VarId name     { 1 }; // ?n.name
    VarId null_var {-1 };

    // node (7315)
    ObjectId label_person       { graph.get_label_id(Label("uncoupled"))     };
    ObjectId key_age            { graph.get_key_id  (Key("lung"))            };
    ObjectId key_name           { graph.get_key_id  (Key("expositions"))     };
    ObjectId value_sixty_five   { graph.get_value_id(ValueString("peering")) };


    std::vector<QueryOptimizerElement*> elements {
        new QueryOptimizerLabel   (graph, n, null_var,           ElementType::NODE, label_person),
        new QueryOptimizerProperty(graph, n, null_var, null_var, ElementType::NODE, key_age,  value_sixty_five),
        new QueryOptimizerProperty(graph, n, null_var, name,     ElementType::NODE, key_name, ObjectId::get_null())
    };


    auto root = optimizer.get_query_plan(elements);
    auto input = make_shared<BindingId>(2);

    std::vector<std::string> var_names {
        "?n",
        "?n.name"
    };

	root->init(input);
	unique_ptr<BindingId const> b = root->next();
	int count = 0;
	while (b != nullptr) {
        b->print(var_names);
		b = root->next();
		count++;
	}
    cout << "Found " << count << " results.\n";
}

void TestQueryOptimizer::Test2() {
    // MATCH (?n1 :label)->(?n2 :label)

    // Label(?n1, label)
    // Label(?n2, label)
    // Connection(?n1, ?e, ?n2)

    Config config = Config();
	RelationalGraph graph = RelationalGraph(0, config);
    QueryOptimizer optimizer{};

    VarId n1       { 0 };
    VarId n2       { 1 };
    VarId e        { 2 };
    VarId null_var {-1 };

    ObjectId label { graph.get_label_id(Label("greyhounds")) };


    std::vector<QueryOptimizerElement*> elements {
        new QueryOptimizerLabel     (graph, n1, null_var, ElementType::NODE, label),
        new QueryOptimizerLabel     (graph, n2, null_var, ElementType::NODE, label),
        new QueryOptimizerConnection(graph, n1, n2, e)
    };


    auto root = optimizer.get_query_plan(elements);
    auto input = make_shared<BindingId>(3);

    std::vector<std::string> var_names {
        "?n1",
        "?n2",
        "?e"
    };

	root->init(input);
	unique_ptr<BindingId const> b = root->next();
	int count = 0;
	while (b != nullptr) {
        b->print(var_names);
		b = root->next();
		count++;
	}
    cout << "Found " << count << " results.\n";
}
