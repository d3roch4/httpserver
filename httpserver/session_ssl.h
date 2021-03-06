#ifndef SESSION_SSL_H
#define SESSION_SSL_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "router.h"
#include "session.h"

namespace httpserver {

namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>

// Handles an HTTP server connection
class session_ssl : public std::enable_shared_from_this<session_ssl>, public session
{
    friend queue<session_ssl>;
    queue<session_ssl> queue_;
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;

public:
    // Take ownership of the socket
    explicit
    session_ssl(
        tcp::socket&& socket,
        ssl::context& ctx,
        router& router);

    boost::beast::ssl_stream<boost::beast::tcp_stream>& stream();

    // Start the asynchronous operation
    void
    run();

    void
    on_handshake(boost::beast::error_code ec);

    void
    do_read();

    bool is_queue_write();

    void
    do_close();

    void
    on_shutdown(boost::beast::error_code ec);

    // put a response in queue
    template <class R>
    void send(R&& response)
    {
        queue_(std::move(response));
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener_ssl : public std::enable_shared_from_this<listener_ssl>
{
    net::io_context& ioc_;
    ssl::context& ctx_;
    tcp::acceptor acceptor_;
    router& router_;

public:
    listener_ssl(
        net::io_context& ioc,
        ssl::context& ctx,
        tcp::endpoint endpoint,
        router& router);

    // Start accepting incoming connections
    void
    run();

private:
    void
    do_accept();

    void
    on_accept(boost::beast::error_code ec, tcp::socket socket);
};

//------------------------------------------------------------------------------
int setup_ssl(const std::string &addr, unsigned short port, router& router, int thread_qtd);

}
#endif // SESSION_SSL_H
