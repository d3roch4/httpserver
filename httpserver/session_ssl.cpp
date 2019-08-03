#include "session_ssl.h"
#include "server_certificate.hpp"
#include "logger.h"

namespace httpserver
{

httpserver::session_ssl::session_ssl(boost::asio::ip::tcp::socket &&socket, boost::asio::ssl::context &ctx, router& router)
    : stream_(std::move(socket), ctx)
    , queue_(*this)
    , router_(router)
{
}

beast::ssl_stream<beast::tcp_stream>& session_ssl::stream()
{
    return stream_;
}

void httpserver::session_ssl::run()
{
    // Set the timeout.
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    stream_.async_handshake(
                ssl::stream_base::server,
                beast::bind_front_handler(
                    &session_ssl::on_handshake,
                    shared_from_this()));
}

void httpserver::session_ssl::on_handshake(boost::beast::error_code ec)
{
    if(ec)
        return fail(ec, "handshake");

    do_read();
}

void httpserver::session_ssl::do_read()
{
    // Construct a new parser for each message
    parser_.emplace();

    // Set the timeout.
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Read a request
    http::async_read(stream_, buffer_, *parser_,
                     beast::bind_front_handler(
                         &session_ssl::on_read,
                         shared_from_this()));
}

void httpserver::session_ssl::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
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

void httpserver::session_ssl::on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
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

    // Read another request
    do_read();
}

void httpserver::session_ssl::do_close()
{
    // Set the timeout.
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Perform the SSL shutdown
    stream_.async_shutdown(
                beast::bind_front_handler(
                    &session_ssl::on_shutdown,
                    shared_from_this()));
}

void httpserver::session_ssl::on_shutdown(boost::beast::error_code ec)
{
    if(ec)
        return fail(ec, "shutdown");

    // At this point the connection is closed gracefully
}

httpserver::listener_ssl::listener_ssl(boost::asio::io_context &ioc, boost::asio::ssl::context &ctx, boost::asio::ip::tcp::endpoint endpoint, router& router)
    : ioc_(ioc)
    , ctx_(ctx)
    , acceptor_(ioc)
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

void httpserver::listener_ssl::run()
{
    do_accept();
}

void httpserver::listener_ssl::do_accept()
{
    // The new connection gets its own strand
    acceptor_.async_accept(
                net::make_strand(ioc_),
                beast::bind_front_handler(
                    &listener_ssl::on_accept,
                    shared_from_this()));
}

void httpserver::listener_ssl::on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
{
    if(ec)
    {
        fail(ec, "listener_ssl::on_accept");
    }
    else
    {
        // Create the session and run it
        std::make_shared<session_ssl>(
                    std::move(socket),
                    ctx_,
                    router_)->run();
    }

    // Accept another connection
    do_accept();
}


int setup_ssl(const std::string &addr, unsigned short port, router& router, int thread_qtd)
{
    auto const address = net::ip::make_address(addr);
    auto const threads = std::max<int>(1, thread_qtd);

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12};

    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx);

    // Create and launch a listening port
    std::make_shared<listener_ssl>(
                ioc,
                ctx,
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
