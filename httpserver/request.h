#ifndef REQUEST_H
#define REQUEST_H

#include <boost/beast.hpp>
#include <unordered_map>
#include "parameter_parser.h"
#include "session_plain.h"
#include "session_ssl.h"

namespace httpserver
{
using namespace boost::beast;
typedef http::request_parser<http::empty_body> request_parser_empty;
typedef http::request<http::empty_body> request_empty;

typedef http::field field;

class dynamic_request : public request_empty
{
    request_parser_empty* request_parser_;
    session* session_;
    std::unordered_map<std::string, parameter_parser> query_;

    void parse_query();
public:
    dynamic_request();
    dynamic_request(request_parser_empty* request_parser, session* session);

    template<class TypeBody=http::string_body>
    typename TypeBody::value_type body() const{
        http::request_parser<TypeBody> parser_body{std::move(*request_parser_)};
        // Apply a reasonable limit to the allowed size
        // of the body in bytes to prevent abuse.
        parser_body.body_limit(1024 * 1024 * 10);

        session_ssl* ssl = dynamic_cast<session_ssl*>(session_);
        if(ssl){
            boost::beast::http::read(ssl->stream(), ssl->buffer(), parser_body); // Finish reading the message
        }else{
            session_plain* plain = dynamic_cast<session_plain*>(session_);
            if(plain){
                boost::beast::http::read(plain->stream(), plain->buffer(), parser_body); // Finish reading the message
            }
        }

        return parser_body.release().body();
    }

    parameter_parser query(const std::string& key);

    template<class T>
    T query(const std::string& key, const T& defaultValue){
        if(query_.empty())
            parse_query();
        auto itr = query_.find(key);
        if(itr != query_.end()){
            std::stringstream ss(itr->second);
            T ret{};
            ss >> std::boolalpha >> ret;
            return ret;
        }else
            return defaultValue;
    }
    std::unordered_map<std::string, parameter_parser> query();
};

dynamic_request& request();

}
#endif // REQUEST_H
