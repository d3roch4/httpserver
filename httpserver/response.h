#ifndef RESPONSE_H
#define RESPONSE_H

#include <boost/beast.hpp>

namespace httpserver
{
typedef boost::beast::http::response<boost::beast::http::string_body> response;


// Returns a bad request response
auto bad_request = [](boost::beast::string_view why)
{
    response res;
    res.result(boost::beast::http::status::bad_request);
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.body() = why.to_string();
    res.prepare_payload();
    return res;
};

// Returns a not found response
auto not_found = [](boost::beast::string_view target)
{
    response res;
    res.result(boost::beast::http::status::not_found);
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.body() = "The resource '" + target.to_string() + "' was not found.";
    res.prepare_payload();
    return res;
};


// Returns a server error response
auto server_error = [](boost::beast::string_view what)
{
    response res;
    res.result(boost::beast::http::status::internal_server_error);
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.body() = "An error occurred: '" + what.to_string() + "'";
    res.prepare_payload();
    return res;
};
}
#endif // RESPONSE_H
