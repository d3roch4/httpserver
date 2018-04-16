#ifndef FILE_H
#define FILE_H

#include "request.h"

namespace httpserver
{

class  file
{
    boost::asio::ip::tcp::socket *socket_;
    boost::beast::flat_buffer *buffer_;
    request_parser_empty *req_;

    std::string data_;
public:
    file();
    file(boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser_empty& req);
    void write(const std::string& filename);
    std::string data();
};

}

#endif // FILE_H
