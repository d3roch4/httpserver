#ifndef REQUEST_H
#define REQUEST_H

#include <boost/beast.hpp>

namespace httpserver
{
typedef boost::beast::http::request<boost::beast::http::string_body> request;
}
#endif // REQUEST_H
