#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include <iostream>
#include "router.h"
#include "wrap_handle_request_basic.h"
#include "request.h"
#include "response.h"
#include "session.h"

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
    
    template<class I, class F, typename... Args >
    void routeB(vector<function_filter>&& filters, verb method, const string &path, I instance, F function, const Args&... args)
    {
        auto binded = bind(function, instance, args...);        
        route(move(filters), method, path, binded, args...);
    }

    template<class I, class F, typename... Args >
    void routeB(verb method, const string &path, I instance, F function, const Args&... args)
    {
        auto binded = bind(function, instance, args...);        
        route({}, method, path, binded, args...);
    }

    void run(const string& address = "0.0.0.0",
             std::vector<std::pair<unsigned short, bool>> ports = { {3000, false}, {3001, true} }, // 3000 plain, 3001 ssl
             const string& public_dir = "public_dir",
             int thread_qtd=4,
             const std::string& certificadoFileName="", const std::string& chaveFileName="");
};



}// namesapce

#endif // HTTPSERVER_H
