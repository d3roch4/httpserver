#include "controller.h"

namespace httpserver {

Controller::Controller()
{
    POST = std::bind(&Controller::filterPost, this, _1, _2);
}

bool Controller::filterPost(httpserver::request &request, httpserver::response &response)
{
    if(request.method() == http::verb::post)
        return true;

    response = not_found();
    return false;
}


response Controller::ok(const string &content, const string& content_type)
{
    httpserver::response response;
    response.result(http::status::ok);
    response.set(http::field::content_type, content_type);
    response.body() = content;
    return response;
}

response Controller::bad_request(const string &content)
{
    httpserver::response response;
    response.result(http::status::bad_request);
    response.body() = content;
    return response;
}

response Controller::not_found(const string &content)
{
    httpserver::response response;
    response.result(http::status::not_found);
    response.body() = content;
    return response;
}

response Controller::no_content(const string &content)
{
    httpserver::response response;
    response.result(http::status::no_content);
    response.body() = content;
    return response;
}

}
