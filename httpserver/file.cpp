#include "file.h"
#include <fstream>

namespace httpserver {

file::file(){}

file::file(boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser_empty &req)
{
    socket_ = &socket;
    buffer_ = &buffer;
    req_ = &req;
}

void file::write(const std::string& filename){
    std::ofstream file{filename, std::ofstream::binary};
    if(! file.good())
        throw std::runtime_error("file::write: "+filename+" not is good");
    boost::beast::http::request_parser<boost::beast::http::string_body> req_file{std::move(*req_)};
    boost::beast::http::read(*socket_, *buffer_, req_file);
    data_.swap(req_file.get().body());

    file.write(data_.c_str(), data_.size());
}

std::string file::data()
{
    return data_;
}

}
