#include "session.h"
#include <boost/beast/ssl.hpp>
#include "request.h"
#include "memory"
#include "logger.h"

namespace httpserver {

std::unordered_map<std::thread::id, session*> map_http_session = {};

session& get_http_session()
{
    session* hs = map_http_session[std::this_thread::get_id()];
    return *hs;
}


session::session(router& router) : 
    router_(router)
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

void session::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if(ec == http::error::end_of_stream)
        return do_close();

    if(ec)
        return fail(ec, "read");

    // Send the response
    //handle_request(*doc_root_, std::move(req_), lambda_);

    map_http_session[std::this_thread::get_id()] = this;
    router_.dispatcher( parser_.get() );
}

void session::on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail(ec, "write");

    if(close)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
    }

    // Inform the queue that a write completed
    if(is_queue_write())
    {
        // Read another request
        do_read();
    }
}

}
