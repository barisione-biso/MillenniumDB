#pragma once

#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include "network/sparql/url_helper.h"
#include "network/sparql/sparql_server.h"

namespace SPARQL_SERVER {

class RequestHandler {
public:
    // template<class Body, class Allocator>
    // static
    // boost::beast::http::message_generator
    // handle_request(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req,
    //                std::chrono::seconds timeout)
    // {
    //     // Returns a bad request response
    //     auto const request_error = [&req](const std::string why) {
    //         boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::bad_request, req.version()};
    //         res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    //         res.set(boost::beast::http::field::content_type, "application/json");
    //         res.keep_alive(req.keep_alive());
    //         res.body() = "{\"error\": \"" + why + "\"}";
    //         res.prepare_payload();
    //         return res;
    //     };

    //     std::string content_type;
    //     std::string sparql_query;
    //     std::string url_encoded;
    //     std::string output_format;

    //     for (auto& header : req) {
    //         if (to_string(header.name()) == "Content-Type") {
    //             content_type = header.value();
    //         }
    //     }

    //     bool query_url_encoded = true;
    //     if (req.method() == boost::beast::http::verb::post) {
    //         std::cout << "post body: '" << req.body() << "'\n";
    //         if (content_type == "application/sparql-query") {
    //             sparql_query = req.body();
    //             query_url_encoded = false;
    //         } else if (content_type == "application/x-www-form-urlencoded") {
    //             url_encoded = req.body();
    //             query_url_encoded = true;
    //         } else {
    //             return request_error("Unknown Content-Type");
    //         }
    //     } else if (req.method() == boost::beast::http::verb::get) {
    //         url_encoded = req.target();
    //         query_url_encoded = true;
    //     }

    //     if (query_url_encoded) {
    //         const char* ptr = url_encoded.c_str();

    //         if (*ptr == '/') {
    //             // params_string_encoded is like "/?query=SELECT+%2A+WHERE+%7B+%3Fs+%3Fp+%3Fo+.+%7D+LIMIT+1000&format=json&output=json&results=json"
    //             // skip until params starts
    //             while (*ptr != '?' && *ptr != '\0') {
    //                 ptr++;
    //             }
    //             if (*ptr != '\0') ptr++;
    //         }

    //         // new *ptr is something like "query=SELECT+%2A+WHERE+%7B+%3Fs+%3Fp+%3Fo+.+%7D+LIMIT+1000&format=json&output=json&results=json"
    //         // key1=value1&key2=value2&...&key_n=value_n
    //         // split by '&' and then by '='
    //         while (*ptr != '\0') {
    //             auto beg_key = ptr;
    //             size_t len_key = 0;
    //             while (*ptr != '\0' && *ptr != '=') {
    //                 ptr++;
    //                 len_key++;
    //             }
    //             if (*ptr != '\0') ptr++;

    //             auto beg_value = ptr;
    //             size_t len_value = 0;
    //             while (*ptr != '\0' && *ptr != '&') {
    //                 ptr++;
    //                 len_value++;
    //             }
    //             std::string key(beg_key, len_key);
    //             std::string val(beg_value, len_value);

    //             if (*ptr != '\0') ptr++;

    //             // process key/value
    //             if (key == "query") {
    //                 sparql_query = UrlHelper::decode(val);
    //             }
    //             // TODO: params can also include 'default-graph-uri' and 'named-graph-uri'. For now we ignore it
    //         }
    //     }

    //     std::stringbuf response_body;
    //     std::ostream os(&response_body);

    //     try {
    //         SparqlServer::execute_query(sparql_query, timeout, os);
    //     } catch (const std::exception& e) {
    //         return request_error(e.what());
    //     }
    //     // TODO: catch timeout and send partial results?

    //     boost::beast::http::response<boost::beast::http::string_body> res { boost::beast::http::status::ok, req.version() };
    //     res.body() = response_body.str();

    //     res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    //     res.set(boost::beast::http::field::content_type, "application/json");
    //     res.keep_alive(req.keep_alive());
    //     res.prepare_payload();
    //     return res;
    // }

    static
    std::string parse_request(boost::beast::http::request<boost::beast::http::string_body>& req)
    {
        // Returns a bad request response
        // auto const request_error = [&req](const std::string why) {
        //     boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::bad_request, req.version()};
        //     res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        //     res.set(boost::beast::http::field::content_type, "application/json");
        //     res.keep_alive(req.keep_alive());
        //     res.body() = "{\"error\": \"" + why + "\"}";
        //     res.prepare_payload();
        //     return res;
        // };

        std::string content_type;
        std::string sparql_query;
        std::string url_encoded;
        std::string output_format;

        for (auto& header : req) {
            if (to_string(header.name()) == "Content-Type") {
                content_type = header.value();
            }
        }

        bool query_url_encoded = true;
        if (req.method() == boost::beast::http::verb::post) {
            // std::cout << "post body: '" << req.body() << "'\n";
            if (content_type == "application/sparql-query") {
                sparql_query = req.body();
                query_url_encoded = false;
            } else if (content_type == "application/x-www-form-urlencoded") {
                url_encoded = req.body();
                query_url_encoded = true;
            } else {
                // return request_error("Unknown Content-Type"); // TODO: throw exception?
            }
        } else if (req.method() == boost::beast::http::verb::get) {
            url_encoded = req.target();
            query_url_encoded = true;
        }

        // std::cout << "target: '" << req.target() << "'\n";

        if (query_url_encoded) {
            const char* ptr = url_encoded.c_str();

            if (*ptr == '/') {
                // params_string_encoded is like "/?query=SELECT+%2A+WHERE+%7B+%3Fs+%3Fp+%3Fo+.+%7D+LIMIT+1000&format=json&output=json&results=json"
                // skip until params starts
                while (*ptr != '?' && *ptr != '\0') {
                    ptr++;
                }
                if (*ptr != '\0') ptr++;
            }

            // new *ptr is something like "query=SELECT+%2A+WHERE+%7B+%3Fs+%3Fp+%3Fo+.+%7D+LIMIT+1000&format=json&output=json&results=json"
            // key1=value1&key2=value2&...&key_n=value_n
            // split by '&' and then by '='
            while (*ptr != '\0') {
                auto beg_key = ptr;
                size_t len_key = 0;
                while (*ptr != '\0' && *ptr != '=') {
                    ptr++;
                    len_key++;
                }
                if (*ptr != '\0') ptr++;

                auto beg_value = ptr;
                size_t len_value = 0;
                while (*ptr != '\0' && *ptr != '&') {
                    ptr++;
                    len_value++;
                }
                std::string key(beg_key, len_key);
                std::string val(beg_value, len_value);

                if (*ptr != '\0') ptr++;

                // process key/value
                if (key == "query") {
                    sparql_query = UrlHelper::decode(val);
                }
                // TODO: params can also include 'default-graph-uri' and 'named-graph-uri'. For now we ignore it
            }
        }

        return sparql_query;
    }
};
} // namespace SPARQL_SERVER
