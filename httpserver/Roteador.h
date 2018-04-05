#ifndef ROTEADOR_H
#define ROTEADOR_H

#include <fstream>
#include <map>
#include <string>
#include <boost/asio/ip/tcp.hpp>
#include "parser_request_i.h"

namespace httpserver {
using namespace std;

class Roteador {
    string public_dir;
public:
    Roteador();

    void rota(verb method, const string &path, shared_ptr<parser_request_i> tratador);    

    // Append an HTTP rel-path to a local filesystem path.
    // The returned path is normalized for the platform.
    string path_cat(boost::beast::string_view base, boost::beast::string_view path);

    void send_file(boost::beast::http::request<boost::beast::http::empty_body>& req);

    void dispatcher(boost::asio::ip::tcp::socket& socket, boost::beast::flat_buffer& buffer, request_parser &req);

    void set_public_dir(const string& dir);

private:
    unordered_map<int, unordered_map<string, shared_ptr<parser_request_i>>> mRotas;
};

} // namespace httpserver

#endif // ROTEADOR_H
