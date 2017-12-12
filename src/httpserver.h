#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <string>
#include "router.h"
#include "controller.h"
#include "mor.h"

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
    void run();

    template<class F, class T>
    struct route_entity{
        typedef F f_type;
        typedef T t_type;
        F func;
    };

    template<class T=httpserver::request, class F=std::function<httpserver::response(httpserver::request)>>
    void addRoute(const string &path, const F& route, func_filter filter=nullptr)
    {
        shared_ptr<i_router_request> route_request{ new router_request<F,T>{route} };
        this->router.add(path, route_request, filter);
    }

private:
    // "Loop" forever accepting new connections.
    void loop(tcp::acceptor& acceptor, tcp::socket& socket);

    size_t thread_qtd=4;
    boost::asio::io_service ios{thread_qtd};
    tcp::socket socket{ios};
    tcp::acceptor acceptor;
    Router router;
};

}

#endif // HTTPSERVER_H
