#include <iostream>
#include <fstream>

using namespace std;

std::string prefix = "Q"; //MillDB requires alphanumeric IDs starting with a character.
void convert_db(const std::string &dataset){
    std::cout << " Reading the DB input file." << std::endl;
    //1. Read the source file.
    std::ifstream ifs(dataset);
    
    std::ofstream output;
    output.open ("output.txt");
    uint64_t s, p , o;
    do {
        ifs >> s >> p >> o;
        output << prefix << s << "->" << prefix << o<< " :"<< prefix << p << "\n";

    } while (!ifs.eof());
    std::cout << " Finished converting the DB." << std::endl;
    output.close();
}

int main(int argc, char **argv)
{

    if(argc != 2){
        std::cout << "Usage: " << argv[0] << " <dataset>" << std::endl;
        return 0;
    }

    std::string dataset = argv[1];
    convert_db(dataset);

    return 0;
}

