#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include <iostream>
#include "router.h"
#include "wrap_handle_request_basic.h"
#include "request.h"
#include "response.h"
#include <initializer_list>

namespace httpserver
{

using namespace std::placeholders;
using namespace std;

class HttpServer
{
    HttpServer(HttpServer&& srv) = delete;
    router router_;
public:
    HttpServer() = default;

    template<class F, typename... Args >
    void route(vector<function_filter>&& filters, verb method, const string &path, F function, const Args&... args)
    {
        auto prb = std::make_shared<wrap_handle_request_basic<F,sizeof...(args)>>(path, function);
        prb->filters = filters;
        router_.add(method, prb);
    }

    template<class F, typename... Args >
    void route(verb method, const string &path, F function, const Args&... args)
    {
        route({}, method, path, function, args...);
    }


    void run(const string& address = "0.0.0.0", unsigned short port = 3000, const string& public_dir = "public_dir", int thread_qtd=4 );
};



}// namesapce

#endif // HTTPSERVER_H
