#ifndef ROUTER_H
#define ROUTER_H

#include <string>
#include <fstream>
#include <map>
#include "request.h"
#include "entity.h"

namespace httpserver
{
using namespace std;


typedef http::response<http::string_body> response;

typedef std::function<httpserver::response(httpserver::request& request)> func_route;
typedef std::function<bool(httpserver::request& request, httpserver::response &response)> func_filter;

struct i_router_request
{
    virtual response call(request& req) = 0;
};

template<typename F, typename T>
struct router_request : i_router_request
{
    static_assert(std::is_base_of<Entity<T>, T>::value, "template<T> is not derived of Entity<T>");

    router_request(const F& f)
        : func{f} {
    }

    response call(request& req){
        unordered_map<string, string> params = req.parameters;
        T obj;
        Entity<T>* entity = & obj;
        for(auto& col: entity->_fields){
            auto it = params.find(col->name);
            if(it != params.end())
                col->setValue(it->second.c_str());
        }

        return func(obj);
    }
private:
    const F func;
};

template<typename F>
struct router_request<F, httpserver::request> : i_router_request
{
    router_request(const F& f)
        : func{f} {
    }

    response call(request& req){
        return func(req);
    }
private:
    const F func;
};

class Router
{
    const string& public_dir;
public:
    Router(const string& public_dir);

    void add(const string& path, shared_ptr<i_router_request> route, func_filter filter=nullptr);

    void route(request &&request, httpserver::response &response);

    bool route_to_file(const string& filename, httpserver::response& response);

private:

    struct call_func_route{
        shared_ptr<i_router_request> route;
        func_filter filter;
    };

    map<string, call_func_route > mapRoutes;
};

}
#endif // ROUTER_H
