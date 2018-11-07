#include "http_session.h"
#include "logger.h"
#include "router.h"

namespace httpserver {

std::unordered_map<std::thread::id, http_session*> map_http_session = {};

http_session::http_session(tcp::socket socket, router &router_)
    : socket_(std::move(socket))
    , strand_(socket_.get_executor())
    , timer_(socket_.get_executor().context(),
             (std::chrono::steady_clock::time_point::max)())
    , queue_(*this)
    , router_(router_)
    , dynamic_request_(&request_parser_)
{
    request_parser_.body_limit(1024 * 1024 * 50); // 10MB

    dynamic_request_.socket(socket_);
    dynamic_request_.buffer(buffer_);
}

httpserver::http_session::~http_session()
{

}

request_parser_empty &http_session::request_parser()
{
    return request_parser_;
}

dynamic_request http_session::request()
{
    dynamic_request r{&request_parser_};
    r.buffer(this->buffer_);
    r.socket(this->socket_);
    string auth = r[http::field::authorization].to_string();

    return r;
}

void http_session::run()
{
    // Run the timer. The timer is operated
    // continuously, this simplifies the code.
    on_timer({});

    do_read();
}

void http_session::do_read()
{
    // Set the timer
    timer_.expires_after(std::chrono::seconds(30));

    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    request_parser_.release();

    // Read a request
    http::async_read_header(socket_, buffer_, request_parser_.base(),
                            boost::asio::bind_executor(
                                strand_,
                                std::bind(
                                    &http_session::on_read,
                                    shared_from_this(),
                                    std::placeholders::_1)));
}

void http_session::on_read(boost::system::error_code ec)
{
    // Happens when the timer closes the socket
    if(ec == boost::asio::error::operation_aborted)
        return;

    // This means they closed the connection
    if(ec == http::error::end_of_stream)
        return;// do_close();

    if(ec)
        return fail(ec, "read");

    if(!request_parser_.get().target().empty())
    {
        map_http_session[std::this_thread::get_id()] = this;
        // Send the response
        router_.dispatcher(socket_, buffer_, request_parser_);
        //        handle_request(std::move(req_), queue_);
    }
    else
        send(response{});
    // If we aren't at the queue limit, try to pipeline another request
    if(! queue_.is_full())
        do_read();
}

void http_session::on_timer(boost::system::error_code ec)
{
    if(ec && ec != boost::asio::error::operation_aborted)
        return fail(ec, "timer");

    // Verify that the timer really expired since the deadline may have moved.
    if(timer_.expiry() <= std::chrono::steady_clock::now())
    {
        // Closing the socket cancels all outstanding operations. They
        // will complete with boost::asio::error::operation_aborted
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        socket_.close(ec);
        return;
    }

    // Wait on the timer
    timer_.async_wait(
                boost::asio::bind_executor(
                    strand_,
                    std::bind(
                        &http_session::on_timer,
                        shared_from_this(),
                        std::placeholders::_1)));
}

void http_session::on_write(boost::system::error_code ec, bool close)
{
    // Happens when the timer closes the socket
    if(ec == boost::asio::error::operation_aborted)
        return;

    if(ec)
        return fail(ec, "write");

    if(close)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
    }

    // Inform the queue that a write completed
    if(queue_.on_write())
    {
        // Read another request
        do_read();
    }
}

void http_session::do_close()
{
    // Send a TCP shutdown
    boost::system::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

http_session& get_http_session()
{
    http_session* hs = map_http_session[std::this_thread::get_id()];
    return *hs;
}

}
