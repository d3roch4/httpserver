#ifndef ROTEADOR_H
#define ROTEADOR_H

#include <fstream>
#include <map>
#include <string>
#include <unordered_map>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include "wrap_handle_request_i.h"

namespace httpserver {
using namespace std;
using request_parser_empty = boost::beast::http::request_parser<boost::beast::http::empty_body>;
using verb = boost::beast::http::verb;

class router {
    string public_dir;
    unordered_map<int, vector<shared_ptr<wrap_handle_request_i>>> mRotas;
public:
    router();
    router(const router& other);

    void add(verb method, shared_ptr<wrap_handle_request_i> handle);

    // Append an HTTP rel-path to a local filesystem path.
    // The returned path is normalized for the platform.
    string path_cat(boost::beast::string_view base, boost::beast::string_view path);

    void send_file(boost::beast::http::request<boost::beast::http::empty_body>& req);

    void dispatcher(request_parser_empty &req);

    void set_public_dir(const string& dir);
};

} // namespace httpserver

#endif // ROTEADOR_H
