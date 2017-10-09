#ifndef ROUTER_H
#define ROUTER_H

#include <string>
#include <fstream>
#include <map>
#include "request.h"

namespace httpserver
{
using namespace std;


typedef http::response<http::string_body> response;

typedef std::function<httpserver::response(httpserver::request& request)> func_route;
typedef std::function<bool(httpserver::request& request, httpserver::response &response)> func_filter;

class Router
{
    const string& public_dir;
public:
    Router(const string& public_dir);

    void add(const string& path, func_route route, func_filter filter=nullptr);

    void route(request &&request, httpserver::response &response);

    bool route_to_file(const string& filename, httpserver::response& response);

private:
    map<string, pair<func_route, func_filter> > mapRoutes;
};

}
#endif // ROUTER_H
