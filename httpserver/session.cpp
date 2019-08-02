#include "session.h"
#include <boost/beast/ssl.hpp>

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
    request_ = {&parser_.get()};
//    r.buffer(this->buffer_);
//    r.socket(this->stream_.socket());
    return request_;
}

}
