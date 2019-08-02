#include "session_plain.h"
#include "request.h"
#include "response.h"
#include "logger.h"

namespace httpserver {

//------------------------------------------------------------------------------
websocket_session::websocket_session(tcp::socket &&socket)
    : ws_(std::move(socket))
{
}

void websocket_session::on_accept(beast::error_code ec)
{
    if(ec)
        return fail(ec, "accept");

    // Read a message
    do_read();
}

void websocket_session::do_read()
{
    // Read a message into our buffer
    ws_.async_read(
                buffer_,
                beast::bind_front_handler(
                    &websocket_session::on_read,
                    shared_from_this()));
}

void websocket_session::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This indicates that the websocket_session was closed
    if(ec == websocket::error::closed)
        return;

    if(ec)
        fail(ec, "read");

    // Echo the message
    ws_.text(ws_.got_text());
    ws_.async_write(
                buffer_.data(),
                beast::bind_front_handler(
                    &websocket_session::on_write,
                    shared_from_this()));
}

void websocket_session::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail(ec, "write");

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Do another read
    do_read();
}

session_plain::session_plain(tcp::socket &&socket, router& router)
    : stream_(std::move(socket))
    , queue_(*this)
    , router_(router)
{
}

void session_plain::run()
{
    do_read();
}

void session_plain::do_read()
{
    // Construct a new parser for each message
    parser_.emplace();

    // Apply a reasonable limit to the allowed size
    // of the body in bytes to prevent abuse.
    parser_->body_limit(10000);

    // Set the timeout.
    stream_.expires_after(std::chrono::seconds(30));

    // Read a request using the parser-oriented interface
    http::async_read(stream_, buffer_, *parser_,
                beast::bind_front_handler(
                    &session_plain::on_read,
                    shared_from_this()));
}

void session_plain::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if(ec == http::error::end_of_stream)
        return do_close();

    if(ec)
        return fail(ec, "read");

    // See if it is a WebSocket Upgrade
    if(websocket::is_upgrade(parser_->get()))
    {
        // Create a websocket session, transferring ownership
        // of both the socket and the HTTP request.
        std::make_shared<websocket_session>(
                    stream_.release_socket())->do_accept(parser_->release());
        return;
    }

    // Send the response
    //handle_request(*doc_root_, std::move(parser_->release()), queue_);

    map_http_session[std::this_thread::get_id()] = this;
    router_.dispatcher( parser_.get() );

    // If we aren't at the queue limit, try to pipeline another request
    if(! queue_.is_full())
        do_read();
}

void session_plain::on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
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
    if(queue_.on_write())
    {
        // Read another request
        do_read();
    }
}

void session_plain::do_close()
{
    // Send a TCP shutdown
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

listener_palin::listener_palin(boost::asio::io_context &ioc, tcp::endpoint endpoint, router &router)
    : ioc_(ioc)
    , acceptor_(net::make_strand(ioc))
    , router_(router)
{
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec)
    {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if(ec)
    {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec)
    {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
                net::socket_base::max_listen_connections, ec);
    if(ec)
    {
        fail(ec, "listen");
        return;
    }
}

void listener_palin::run()
{
    do_accept();
}

void listener_palin::do_accept()
{
    // The new connection gets its own strand
    acceptor_.async_accept(
                net::make_strand(ioc_),
                beast::bind_front_handler(
                    &listener_palin::on_accept,
                    shared_from_this()));
}

void listener_palin::on_accept(beast::error_code ec, tcp::socket socket)
{
    if(ec)
    {
        fail(ec, "accept");
    }
    else
    {
        // Create the http session and run it
        std::make_shared<session_plain>(
                    std::move(socket),
                    router_)->run();
    }

    // Accept another connection
    do_accept();
}

int setup_plain(const string &addr, unsigned short port, router &router, int thread_qtd)
{
    auto const address = net::ip::make_address(addr);
    auto const threads = std::max<int>(1, thread_qtd);

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<listener_palin>(
                ioc,
                tcp::endpoint{address, port},
                router)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                    [&ioc]
        {
            ioc.run();
        });
    ioc.run();
}

}
