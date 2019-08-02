#ifndef REQUEST_H
#define REQUEST_H

#include <boost/beast.hpp>
#include <unordered_map>
#include "parameter_parser.h"

namespace httpserver
{
using namespace boost::beast;
typedef http::request_parser<http::empty_body> request_parser_empty;
typedef http::request<http::empty_body> request_empty;

typedef http::field field;

class dynamic_request : public request_empty
{
    request_parser_empty* request_parser_;
    boost::asio::ip::tcp::socket* socket_;
    flat_buffer* buffer_;
    std::unordered_map<std::string, parameter_parser> query_;

    void parse_query();
public:
    dynamic_request();
    dynamic_request(request_parser_empty* request_parser);

    template<class TypeBody=http::string_body>
    typename TypeBody::value_type body() const{
        http::request_parser<TypeBody> parser_body{std::move(*request_parser_)};
        http::read(*socket_, *buffer_, parser_body); // Finish reading the message
        return parser_body.release().body();
    }

    void socket(boost::asio::ip::tcp::socket &skt);
    void buffer(flat_buffer &bff);
    parameter_parser query(const std::string& key);
    std::unordered_map<std::string, parameter_parser> query();
};

dynamic_request& request();

}
#endif // REQUEST_H
