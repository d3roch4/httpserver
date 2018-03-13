#ifndef RESPONSE_H
#define RESPONSE_H

#include <boost/beast.hpp>

namespace httpserver
{
typedef boost::beast::http::response<boost::beast::http::string_body> response;
}
#endif // RESPONSE_H
