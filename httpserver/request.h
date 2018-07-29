#ifndef REQUEST_H
#define REQUEST_H

#include <boost/beast.hpp>

namespace httpserver
{
using namespace boost::beast;
typedef http::request_parser<http::empty_body> request_parser_empty;
typedef http::request<http::empty_body> request_empty;

typedef http::field field;
typedef string_view string_view;

class dynamic_request : public request_empty
{
    request_parser_empty* request_parser_;
    boost::asio::ip::tcp::socket* socket_;
    flat_buffer* buffer_;
public:
    dynamic_request(request_parser_empty&& request_parser);

    template<class TypeBody=http::string_body>
    typename TypeBody::value_type body(){
        http::request_parser<TypeBody> parser_body{std::move(*request_parser_)};
        http::read(*socket_, *buffer_, parser_body); // Finish reading the message
        return parser_body.release().body();
    }

    void socket(boost::asio::ip::tcp::socket &skt);
    void buffer(flat_buffer &bff);
};

dynamic_request request();

}
#endif // REQUEST_H
