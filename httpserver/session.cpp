#include "session.h"
#include <boost/beast/ssl.hpp>
#include "request.h"
#include "memory"

namespace httpserver {

std::unordered_map<std::thread::id, session*> map_http_session = {};

session& get_http_session()
{
    session* hs = map_http_session[std::this_thread::get_id()];
    return *hs;
}


session::session() 
{
}

dynamic_request& session::request()
{
    if(!request_)
        request_ = std::make_shared<dynamic_request>(parser_.get_ptr(), this);
    return *request_;
}

beast::flat_buffer& session::buffer()
{
    return buffer_;
}

}
