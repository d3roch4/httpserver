#ifndef REQUEST_H
#define REQUEST_H

#include <boost/beast.hpp>

namespace httpserver
{
typedef boost::beast::http::request_parser<boost::beast::http::empty_body> request_parser;
typedef boost::beast::http::request<boost::beast::http::empty_body> request;

class  file
{
    boost::asio::ip::tcp::socket *socket_;
    boost::beast::flat_buffer *buffer_;
    request_parser *req_;

    std::string data_;
public:
    file();

    file(boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser& req);

//    file(const file& other){
//        this->socket_ = other.socket_;
//        this->buffer_ = other.buffer_;
//        this->req_ = other.req_;
//    }

    void write(const std::string& filename);

    std::string data();
};

}
#endif // REQUEST_H
