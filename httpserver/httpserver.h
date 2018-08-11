#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <iostream>
#include "logger.h"
#include "router.h"
#include <httpserver/parser/parser_request_basic.h>
#include <httpserver/parser/json.h>


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
//    template<typename... Args >
//    void route(vector<parser::function_filter>&& filters, verb method, const string &path, const F func, T* instancia, const Args&... args)
//    {
       /* parser_request_creater<P> prc;
        auto cvr = prc.create(path, func, instancia, args...);
        cvr->filters = filters;
        router_.add(method, path, cvr)*/;
//    }

    template<class F, typename... Args >
    void route(verb method, const string &path, F function, const Args&... args)
    {
        std::initializer_list<std::string> inputs({args...});
        std::vector<std::string> parametros(inputs);

        auto prb = std::make_shared<parser::parser_request_basic<F,sizeof...(args)>>(path, function, parametros);
        router_.add(method, prb);
    }


    void run(const string& address = "0.0.0.0", unsigned short port = 3000, const string& public_dir = "public_dir", int thread_qtd=4 );
};



}// namesapce

#endif // HTTPSERVER_H
