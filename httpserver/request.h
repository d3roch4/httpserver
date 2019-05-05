#ifndef REQUEST_H
#define REQUEST_H

#include <boost/beast.hpp>
#include <unordered_map>
#include "parameter_parser.h"
#include "session.h"

namespace httpserver
{
using request_parser_empty = boost::beast::http::request_parser<boost::beast::http::empty_body>;
using request_empty = boost::beast::http::request<boost::beast::http::empty_body>;
using field = boost::beast::http::field;

class dynamic_request : public request_empty
{
    httpserver::request_parser_empty& request_parser_;
    http_session_i* session_;
    std::unordered_map<std::string, parameter_parser> query_;

    void parse_query();
public:
    dynamic_request(request_parser_empty& request_parser, http_session_i* session);

    template<class TypeBody=boost::beast::http::string_body>
    typename TypeBody::value_type body() const
    {
        boost::beast::http::request_parser<TypeBody> parser_body{std::move(request_parser_)};
        ssl_http_session* ssl = dynamic_cast<ssl_http_session*>(session_);
        if(ssl)
            boost::beast::http::read(ssl->stream(), ssl->buffer(), parser_body); // Finish reading the message
        else{
            plain_http_session* plain = dynamic_cast<plain_http_session*>(session_);
            if(plain)
                boost::beast::http::read(plain->stream(), plain->buffer(), parser_body); // Finish reading the message
        }
        return parser_body.release().body();
    }

    template<class T>
    T query(const std::string &key, T defaultValue )
    {
        if(query_.empty())
            parse_query();
        auto itr = query_.find(key);
        if(itr != query_.end())
            defaultValue = itr->second;

        return defaultValue;
    }

    void socket(boost::asio::ip::tcp::socket &skt);
    void buffer(boost::beast::flat_buffer &bff);
    parameter_parser query(const std::string& key);
    std::unordered_map<std::string, parameter_parser> query();
};

dynamic_request& request();

}
#endif // REQUEST_H
