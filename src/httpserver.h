#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <string>
#include "router.h"

namespace httpserver
{

using namespace std;
using tcp = boost::asio::ip::tcp;       // from <boost/asio.hpp>

typedef http::status status;
typedef http::field field;

class HttpServer
{
public:
    HttpServer(const string& address = "0.0.0.0", unsigned short port = 3000, const string& public_dir = "public_dir", size_t thread_qtd=4 );
    void addRoute(const string& path, func_route route, func_filter filter=nullptr);
    void run();
private:
    // "Loop" forever accepting new connections.
    void http_server(tcp::acceptor& acceptor, tcp::socket& socket);

    size_t thread_qtd=4;
    boost::asio::io_service ios{thread_qtd};
    tcp::socket socket{ios};
    tcp::acceptor acceptor;
    Router router;
};

}

#endif // HTTPSERVER_H
