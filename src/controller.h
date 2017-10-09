#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "httpserver.h"
#include <functional>
using namespace std::placeholders;

namespace httpserver {

class Controller
{
public:
    Controller();


    httpserver::func_filter POST;
    bool filterPost(httpserver::request& request, response &response);

    response ok(const string& content, const string &content_type = "text/plain");
    response bad_request(const string& content = "bad_request");
    response not_found(const string& content = "not_found");
    response no_content(const string& content = "no_content");

};

}

#define BIND(X) bind(& X, this, std::placeholders::_1 )

#endif // CONTROLLER_H
