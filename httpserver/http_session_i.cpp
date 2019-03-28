#include "http_session_i.h"
namespace httpserver {

std::unordered_map<std::thread::id, http_session_i*> map_http_session;

http_session_i::~http_session_i()
{
    map_http_session.erase(std::this_thread::get_id());
}

http_session_i* get_http_session()
{
    http_session_i* hs = map_http_session[std::this_thread::get_id()];
    return hs;
}


}
