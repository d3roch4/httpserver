#ifndef REQUEST_H
#define REQUEST_H

#include <boost/beast.hpp>

namespace httpserver
{
typedef boost::beast::http::request_parser<boost::beast::http::empty_body> request_parser_empty;
typedef boost::beast::http::request<boost::beast::http::empty_body> request_empty;

typedef boost::beast::http::field field;
typedef boost::string_view string_view;
}
#endif // REQUEST_H
