#include "request.h"
#include "http_session.h"

namespace httpserver {

dynamic_request::dynamic_request(request_parser_empty &&req_empty) :
    request_empty(req_empty.get()) {
    this->request_parser_ = & req_empty;
}



void dynamic_request::socket(boost::asio::ip::tcp::socket &skt){
    this->socket_ = &skt;
}

void dynamic_request::buffer(flat_buffer &bff){
    this->buffer_ = &bff;
}

dynamic_request request(){
    http_session& ss = get_http_session();
    dynamic_request r{std::move(ss.request_parser())};
    r.socket(ss.socket());
    r.buffer(ss.buffer());
    return r;
}


}
