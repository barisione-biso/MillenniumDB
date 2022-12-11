#include <iostream>
#include <utility>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>
using namespace std;

std::string prefix = "Q"; //MillDB requires alphanumeric IDs starting with a character.

bool get_file_content(string filename, vector<string> & vector_of_strings)
{
    // Open the File
    ifstream in(filename.c_str());
    // Check if object is valid
    if(!in)
    {
        cerr << "Cannot open the File : " << filename << endl;
        return false;
    }
    string str;
    // Read the next line from File until it reaches the end.
    while (getline(in, str))
    {
        // Line contains string of length > 0 then save it in vector
        if(str.size() > 0)
            vector_of_strings.push_back(str);
    }
    //Close The File
    in.close();
    return true;
}


std::string ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(' ');
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(' ');
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string &s) {
    return rtrim(ltrim(s));
}

std::vector<std::string> tokenizer(const std::string &input, const char &delimiter){
    std::stringstream stream(input);
    std::string token;
    std::vector<std::string> res;
    while(getline(stream, token, delimiter)){
        res.emplace_back(trim(token));
    }
    return res;
}

bool is_variable(string & s)
{
    return (s.at(0) == '?');
}

std::string get_var_or_const(string input_str){
    return is_variable(input_str) == true ? input_str : prefix + input_str;
}
void transform_queries(const std::string &queries){
    vector<string> dummy_queries;
    bool result = get_file_content(queries, dummy_queries);
    if(result)
    {
        ofstream ofs("output_queries.txt");
        for (string& query_string : dummy_queries) {
            vector<string> transformed_triples;
            std::unordered_map<std::string, uint8_t> hash_table_vars;
            vector<string> tokens_query = tokenizer(query_string, '.');
            for (string& token : tokens_query) {
                auto triple_patterns = tokenizer(token, ' ');
                std::string mdb_BGP_triple = "("+get_var_or_const(triple_patterns[0])+")-[:"+get_var_or_const(triple_patterns[1])+"]->("+get_var_or_const(triple_patterns[2])+")";
                transformed_triples.push_back(mdb_BGP_triple);
            }

            if(transformed_triples.size() > 0){
                std::string mdb_BGP_graph = "MATCH ";
                for(vector<string>::iterator it = transformed_triples.begin(); it != transformed_triples.end(); ++it) {
                    if(it != transformed_triples.begin()) {
                    mdb_BGP_graph += ", ";
                    }
                    mdb_BGP_graph += *it;
                }
                mdb_BGP_graph += "  RETURN * LIMIT 1000";
                //std::cout << mdb_BGP_graph << std::endl;
                ofs << mdb_BGP_graph;
                ofs << "\n" ;
            }
        }
        ofs.close();
    }
}


int main(int argc, char* argv[])
{
    std::string queries = argv[1];
    transform_queries(queries);

	return 0;
}
