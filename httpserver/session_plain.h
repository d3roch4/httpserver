#ifndef SESSION_PLAIN_H
#define SESSION_PLAIN_H
#include "session.h"
#include "router.h"

namespace httpserver {            // from <boost/asio/ip/tcp.hpp>

// Echoes back all received WebSocket messages
class websocket_session : public std::enable_shared_from_this<websocket_session>
{
    websocket::stream<boost::beast::tcp_stream> ws_;
    boost::beast::flat_buffer buffer_;

public:
    // Take ownership of the socket
    explicit
    websocket_session(tcp::socket&& socket);

    // Start the asynchronous accept operation
    template<class Body, class Allocator>
    void
    do_accept(http::request<Body, http::basic_fields<Allocator>> req)
    {
        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                boost::beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " advanced-server");
            }));

        // Accept the websocket handshake
        ws_.async_accept(
            req,
            boost::beast::bind_front_handler(
                &websocket_session::on_accept,
                shared_from_this()));
    }

private:
    void
    on_accept(boost::beast::error_code ec);

    void
    do_read();

    void
    on_read(
        boost::beast::error_code ec,
        std::size_t bytes_transferred);

    void
    on_write(
        boost::beast::error_code ec,
        std::size_t bytes_transferred);
};

//------------------------------------------------------------------------------

// Handles an HTTP server connection
class session_plain : public std::enable_shared_from_this<session_plain>, public session
{
    friend queue<session_plain>;
    queue<session_plain> queue_;
    boost::beast::tcp_stream stream_;

public:
    virtual ~session_plain(){}

    // Take ownership of the socket
    session_plain(
        tcp::socket&& socket,
        router& router);

    boost::beast::tcp_stream& stream();

    // Start the session
    void
    run();

    // put a response in queue
    template <class R>
    void send(R&& response)
    {
        queue_(std::move(response));
    }
private:
    void
    do_read();

    bool is_queue_write();
    
    void
    do_close();
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener_palin : public std::enable_shared_from_this<listener_palin>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    router &router_;

public:
    listener_palin(
        net::io_context& ioc,
        tcp::endpoint endpoint,
        router &router);

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

int setup_plain(const std::string &addr, unsigned short port, router& router, int thread_qtd);

}
#endif // SESSION_PLAIN_H
