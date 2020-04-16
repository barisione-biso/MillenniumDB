#include <fstream>
#include <iostream>
#include <iterator>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "server/tcp_buffer.h"

using namespace std;
using boost::asio::ip::tcp;
namespace po = boost::program_options;

int main(int argc, char **argv) {
    string query_file;
    string host;
    int port;
    try {
        // Parse arguments
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "show this help message")
            ("host,h", po::value<string>(&host)->default_value("127.0.0.1"), "database server host")
            ("port,p", po::value<int>(&port)->default_value(8080), "database server port")
            ("query-file,q", po::value<string>(&query_file)->required(), "query file")
        ;

        po::positional_options_description p;
        p.add("query-file", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

        if (vm.count("help")) {
            cout << "Usage: query [OPTION]... [QUERY_FILE]\n";
            cout << desc << "\n";
            return 0;
        }
        po::notify(vm);

        // Read query-file
        ifstream in(query_file, ios_base::in);
        if (!in) {
            cerr << "Error: Could not open input file: " << query_file << endl;
            return 1;
        }
        stringstream str_stream;
        str_stream << in.rdbuf();
        string query = str_stream.str();

        boost::asio::io_service io_service;

        tcp::socket s(io_service);
        tcp::resolver resolver(io_service);
        boost::asio::connect(s, resolver.resolve({host, std::to_string(port)})); // TODO: port como string?

        // Send Query
        auto query_length = query.size();
        boost::asio::write(s, boost::asio::buffer(query, query_length));

        // Read results
        unsigned char result_buffer[TcpBuffer::buffer_size];
        do {
            /*size_t reply_length =*/ boost::asio::read(s, boost::asio::buffer(result_buffer, TcpBuffer::buffer_size));
            // std::cout << "Length is: " << reply_length << "\n";
            unsigned int reply_length = 0;
            reply_length += result_buffer[1];
            reply_length += result_buffer[2] << 8;
            std::cout.write(reinterpret_cast<char*>(result_buffer+3), reply_length-3);
        } while ( result_buffer[0] != static_cast<unsigned char>(MessageType::end_plain_text) );
    }
    catch (boost::system::system_error const& e) {
        std::cout << "Error connecting to server: " << e.what() << std::endl;
        return -1;
    }
    catch(exception& e) {
        cerr << e.what() << "\n";
        return -1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
        return -1;
    }
}
