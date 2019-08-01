#ifndef SESSION_H
#define SESSION_H
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#if BOOST_VERSION >= 107000
#   include <boost/beast/ssl/ssl_stream.hpp>
#else
#   include <boost/beast/experimental/core/ssl_stream.hpp>
#endif
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/make_unique.hpp>
#include <boost/config.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/any.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include "detect_ssl.hpp"
#include <httpserver/router.h>
#include "logger.h"
#include "http_session_i.h"

namespace httpserver {

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;               // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;            // from <boost/beast/http.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

// Echoes back all received WebSocket messages.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template<class Derived>
class websocket_session
{
    // Access the derived class, this is part of
    // the Curiously Recurring Template Pattern idiom.
    Derived&
    derived()
    {
        return static_cast<Derived&>(*this);
    }

    boost::beast::multi_buffer buffer_;
    char ping_state_ = 0;

protected:
    boost::asio::strand<
        boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer timer_;

public:
    // Construct the session
    explicit
    websocket_session(boost::asio::io_context& ioc)
        : strand_(ioc.get_executor())
        , timer_(ioc,
            (std::chrono::steady_clock::time_point::max)())
    {
    }

    // Start the asynchronous operation
    template<class Body, class Allocator>
    void
    do_accept(http::request<Body, http::basic_fields<Allocator>> req)
    {
        // Set the control callback. This will be called
        // on every incoming ping, pong, and close frame.
        derived().ws().control_callback(
            std::bind(
                &websocket_session::on_control_callback,
                this,
                std::placeholders::_1,
                std::placeholders::_2));

        // Set the timer
        timer_.expires_after(std::chrono::seconds(15));

        // Accept the websocket handshake
        derived().ws().async_accept(
            req,
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &websocket_session::on_accept,
                    derived().shared_from_this(),
                    std::placeholders::_1)));
    }

    void
    on_accept(boost::system::error_code ec)
    {
        // Happens when the timer closes the socket
        if(ec == boost::asio::error::operation_aborted)
            return;

        if(ec)
            return fail(ec, "accept");

        // Read a message
        do_read();
    }

    // Called when the timer expires.
    void
    on_timer(boost::system::error_code ec)
    {
        if(ec && ec != boost::asio::error::operation_aborted)
            return fail(ec, "timer");

        // See if the timer really expired since the deadline may have moved.
        if(timer_.expiry() <= std::chrono::steady_clock::now())
        {
            // If this is the first time the timer expired,
            // send a ping to see if the other end is there.
            if(derived().ws().is_open() && ping_state_ == 0)
            {
                // Note that we are sending a ping
                ping_state_ = 1;

                // Set the timer
                timer_.expires_after(std::chrono::seconds(15));

                // Now send the ping
                derived().ws().async_ping({},
                    boost::asio::bind_executor(
                        strand_,
                        std::bind(
                            &websocket_session::on_ping,
                            derived().shared_from_this(),
                            std::placeholders::_1)));
            }
            else
            {
                // The timer expired while trying to handshake,
                // or we sent a ping and it never completed or
                // we never got back a control frame, so close.

                derived().do_timeout();
                return;
            }
        }

        // Wait on the timer
        timer_.async_wait(
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &websocket_session::on_timer,
                    derived().shared_from_this(),
                    std::placeholders::_1)));
    }

    // Called to indicate activity from the remote peer
    void
    activity()
    {
        // Note that the connection is alive
        ping_state_ = 0;

        // Set the timer
        timer_.expires_after(std::chrono::seconds(15));
    }

    // Called after a ping is sent.
    void
    on_ping(boost::system::error_code ec)
    {
        // Happens when the timer closes the socket
        if(ec == boost::asio::error::operation_aborted)
            return;

        if(ec)
            return fail(ec, "ping");

        // Note that the ping was sent.
        if(ping_state_ == 1)
        {
            ping_state_ = 2;
        }
        else
        {
            // ping_state_ could have been set to 0
            // if an incoming control frame was received
            // at exactly the same time we sent a ping.
            BOOST_ASSERT(ping_state_ == 0);
        }
    }

    void
    on_control_callback(
        websocket::frame_type kind,
        boost::beast::string_view payload)
    {
        boost::ignore_unused(kind, payload);

        // Note that there is activity
        activity();
    }

    void
    do_read()
    {
        // Read a message into our buffer
        derived().ws().async_read(
            buffer_,
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &websocket_session::on_read,
                    derived().shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2)));
    }

    void
    on_read(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // Happens when the timer closes the socket
        if(ec == boost::asio::error::operation_aborted)
            return;

        // This indicates that the websocket_session was closed
        if(ec == websocket::error::closed)
            return;

        if(ec)
            fail(ec, "read");

        // Note that there is activity
        activity();

        // Echo the message
        derived().ws().text(derived().ws().got_text());
        derived().ws().async_write(
            buffer_.data(),
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &websocket_session::on_write,
                    derived().shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2)));
    }

    void
    on_write(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // Happens when the timer closes the socket
        if(ec == boost::asio::error::operation_aborted)
            return;

        if(ec)
            return fail(ec, "write");

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Do another read
        do_read();
    }
};

// Handles a plain WebSocket connection
class plain_websocket_session
    : public websocket_session<plain_websocket_session>
    , public std::enable_shared_from_this<plain_websocket_session>
{
    websocket::stream<tcp::socket> ws_;
    bool close_ = false;

public:
    // Create the session
    explicit
    plain_websocket_session(tcp::socket socket)
        : websocket_session<plain_websocket_session>(
            socket.get_executor().context())
        , ws_(std::move(socket))
    {
    }

    // Called by the base class
    websocket::stream<tcp::socket>&
    ws()
    {
        return ws_;
    }

    // Start the asynchronous operation
    template<class Body, class Allocator>
    void
    run(http::request<Body, http::basic_fields<Allocator>> req)
    {
        // Run the timer. The timer is operated
        // continuously, this simplifies the code.
        on_timer({});

        // Accept the WebSocket upgrade request
        do_accept(std::move(req));
    }

    void
    do_timeout()
    {
        // This is so the close can have a timeout
        if(close_)
            return;
        close_ = true;

        // Set the timer
        timer_.expires_after(std::chrono::seconds(15));

        // Close the WebSocket Connection
        ws_.async_close(
            websocket::close_code::normal,
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &plain_websocket_session::on_close,
                    shared_from_this(),
                    std::placeholders::_1)));
    }

    void
    on_close(boost::system::error_code ec)
    {
        // Happens when close times out
        if(ec == boost::asio::error::operation_aborted)
            return;

        if(ec)
            return fail(ec, "close");

        // At this point the connection is gracefully closed
    }
};

// Handles an SSL WebSocket connection
class ssl_websocket_session
    : public websocket_session<ssl_websocket_session>
    , public std::enable_shared_from_this<ssl_websocket_session>
{
    websocket::stream<boost::beast::ssl_stream<tcp::socket>> ws_;
    bool eof_ = false;

public:
    // Create the http_session
    explicit
    ssl_websocket_session(boost::beast::ssl_stream<tcp::socket> stream)
        : websocket_session<ssl_websocket_session>(
            stream.get_executor().context())
        , ws_(std::move(stream))
    {
    }

    // Called by the base class
    websocket::stream<boost::beast::ssl_stream<tcp::socket>>&
    ws()
    {
        return ws_;
    }

    // Start the asynchronous operation
    template<class Body, class Allocator>
    void
    run(http::request<Body, http::basic_fields<Allocator>> req)
    {
        // Run the timer. The timer is operated
        // continuously, this simplifies the code.
        on_timer({});

        // Accept the WebSocket upgrade request
        do_accept(std::move(req));
    }

    void
    do_eof()
    {
        eof_ = true;

        // Set the timer
        timer_.expires_after(std::chrono::seconds(15));

        // Perform the SSL shutdown
        ws_.next_layer().async_shutdown(
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &ssl_websocket_session::on_shutdown,
                    shared_from_this(),
                    std::placeholders::_1)));
    }

    void
    on_shutdown(boost::system::error_code ec)
    {
        // Happens when the shutdown times out
        if(ec == boost::asio::error::operation_aborted)
            return;

        if(ec)
            return fail(ec, "shutdown");

        // At this point the connection is closed gracefully
    }

    void
    do_timeout()
    {
        // If this is true it means we timed out performing the shutdown
        if(eof_)
            return;

        // Start the timer again
        timer_.expires_at(
            (std::chrono::steady_clock::time_point::max)());
        on_timer({});
        do_eof();
    }
};

template<class Body, class Allocator>
void
make_websocket_session(
    tcp::socket socket,
    http::request<Body, http::basic_fields<Allocator>> req)
{
    std::make_shared<plain_websocket_session>(
        std::move(socket))->run(std::move(req));
}

template<class Body, class Allocator>
void
make_websocket_session(
    boost::beast::ssl_stream<tcp::socket> stream,
    http::request<Body, http::basic_fields<Allocator>> req)
{
    std::make_shared<ssl_websocket_session>(
        std::move(stream))->run(std::move(req));
}

//------------------------------------------------------------------------------

// Handles an HTTP server connection.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template<class Derived>
class http_session : public http_session_i
{
    // Access the derived class, this is part of
    // the Curiously Recurring Template Pattern idiom.
    Derived&
    derived()
    {
        return static_cast<Derived&>(*this);
    }

    // This queue is used for HTTP pipelining.
    class queue
    {
        enum
        {
            // Maximum number of responses we will queue
            limit = 8
        };

        // The type-erased, saved work item
        struct work
        {
            virtual ~work() = default;
            virtual void operator()() = 0;
        };

        http_session& self_;
        std::vector<std::unique_ptr<work>> items_;

    public:
        explicit
        queue(http_session& self)
            : self_(self)
        {
            static_assert(limit > 0, "queue limit must be positive");
            items_.reserve(limit);
        }

        // Returns `true` if we have reached the queue limit
        bool
        is_full() const
        {
            return items_.size() >= limit;
        }

        // Called when a message finishes sending
        // Returns `true` if the caller should initiate a read
        bool
        on_write()
        {
            BOOST_ASSERT(! items_.empty());
            auto const was_full = is_full();
            items_.erase(items_.begin());
            if(! items_.empty())
                (*items_.front())();
            return was_full;
        }

        // Called by the HTTP handler to send a response.
        template<bool isRequest, class Body, class Fields>
        void
        operator()(http::message<isRequest, Body, Fields>&& msg)
        {
            // This holds a work item
            struct work_impl : work
            {
                http_session& self_;
                http::message<isRequest, Body, Fields> msg_;

                work_impl(
                    http_session& self,
                    http::message<isRequest, Body, Fields>&& msg)
                    : self_(self)
                    , msg_(std::move(msg))
                {
                }

                void
                operator()()
                {
                    http::async_write(
                        self_.derived().stream(),
                        msg_,
                        boost::asio::bind_executor(
                            self_.strand_,
                            std::bind(
                                &http_session::on_write,
                                self_.derived().shared_from_this(),
                                std::placeholders::_1,
                                msg_.need_eof())));
                }
            };

            // Allocate and store the work
            items_.push_back(
                boost::make_unique<work_impl>(self_, std::move(msg)));

            // If there was no previous work, start this one
            if(items_.size() == 1)
                (*items_.front())();
        }
    };

    httpserver::router& router_;
    httpserver::request_parser_empty request_parser_;
    shared_ptr<dynamic_request> dynamic_request_;
    queue queue_;

protected:
    boost::asio::steady_timer timer_;
    boost::asio::strand<
        boost::asio::io_context::executor_type> strand_;
    boost::beast::flat_buffer buffer_;

public:
    // Construct the session
    http_session(
        boost::asio::io_context& ioc,
        boost::beast::flat_buffer buffer,
        httpserver::router& router)
        : router_(router)
        , queue_(*this)
        , timer_(ioc, (std::chrono::steady_clock::time_point::max)())
        , strand_(ioc.get_executor())
        , buffer_(std::move(buffer))
    {        
        request_parser_.body_limit(1024 * 1024 * 50); // 50MB
    }

    boost::beast::flat_buffer& buffer()
    {
        return buffer_;
    }

    dynamic_request& request()
    {
        if(dynamic_request_ == nullptr)
            dynamic_request_ = make_shared<dynamic_request>(request_parser_, (Derived*)this);
        return *dynamic_request_;
    }

    // put a response in queue
    template <class R>
    void send(R&& response)
    {
        queue_(std::move(response));
    }

    void
    do_read()
    {
        // Set the timer
        timer_.expires_after(std::chrono::seconds(30));

        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        request_parser_.release();

        // Read a request
        http::async_read_header(
                    derived().stream(),
                    buffer_,
                    request_parser_.base(),
                    boost::asio::bind_executor(
                        strand_,
                        std::bind(
                            &http_session::on_read,
                            derived().shared_from_this(),
                            std::placeholders::_1)));
    }

    // Called when the timer expires.
    void
    on_timer(boost::system::error_code ec)
    {
        if(ec && ec != boost::asio::error::operation_aborted)
            return fail(ec, "timer");

        // Check if this has been upgraded to Websocket
        if(timer_.expires_at() == (std::chrono::steady_clock::time_point::min)())
            return;

        // Verify that the timer really expired since the deadline may have moved.
        if(timer_.expiry() <= std::chrono::steady_clock::now())
            return derived().do_timeout();

        // Wait on the timer
        timer_.async_wait(
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &http_session::on_timer,
                    derived().shared_from_this(),
                    std::placeholders::_1)));
    }

    void
    on_read(boost::system::error_code ec)
    {
        // Happens when the timer closes the socket
        if(ec == boost::asio::error::operation_aborted)
            return;

        // This means they closed the connection
        if(ec == http::error::end_of_stream)
            return derived().do_eof();

        if(ec)
            return fail(ec, "read");

        // See if it is a WebSocket Upgrade
        if(websocket::is_upgrade(request_parser_.get()))
        {
            // Make timer expire immediately, by setting expiry to time_point::min we can detect
            // the upgrade to websocket in the timer handler
            timer_.expires_at((std::chrono::steady_clock::time_point::min)());

            // Transfer the stream to a new WebSocket session
            return make_websocket_session(
                derived().release_stream(),
                std::move(request_parser_.get()));
        }
        if(request_parser_.get().target().empty())
            return ;

        map_http_session[std::this_thread::get_id()] = (http_session_i*)this;
        // Send the response
        router_.dispatcher(request_parser_);
//        handle_request(*doc_root_, std::move(req_), queue_);

        // If we aren't at the queue limit, try to pipeline another request
        if(! queue_.is_full())
            do_read();
    }

    void
    on_write(boost::system::error_code ec, bool close)
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
            return derived().do_eof();
        }

        // Inform the queue that a write completed
        if(queue_.on_write())
        {
            // Read another request
            do_read();
        }
    }
};

// Handles a plain HTTP connection
class plain_http_session
    : public http_session<plain_http_session>
    , public std::enable_shared_from_this<plain_http_session>
{
    tcp::socket socket_;
    boost::asio::strand<
        boost::asio::io_context::executor_type> strand_;

public:
    // Create the http_session
    plain_http_session(
        tcp::socket socket,
        boost::beast::flat_buffer buffer,
        httpserver::router& router)
        : http_session<plain_http_session>(
            socket.get_executor().context(),
            std::move(buffer),
            router)
        , socket_(std::move(socket))
        , strand_(socket_.get_executor())
    {
    }

    // Called by the base class
    tcp::socket&
    stream()
    {
        return socket_;
    }

    // Called by the base class
    tcp::socket
    release_stream()
    {
        return std::move(socket_);
    }

    // Start the asynchronous operation
    void
    run()
    {
        // Make sure we run on the strand
        if(! strand_.running_in_this_thread())
            return boost::asio::post(
                boost::asio::bind_executor(
                    strand_,
                    std::bind(
                        &plain_http_session::run,
                        shared_from_this())));

        // Run the timer. The timer is operated
        // continuously, this simplifies the code.
        on_timer({});

        do_read();
    }

    void
    do_eof()
    {
        // Send a TCP shutdown
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }

    void
    do_timeout()
    {
        // Closing the socket cancels all outstanding operations. They
        // will complete with boost::asio::error::operation_aborted
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }
};

// Handles an SSL HTTP connection
class ssl_http_session
    : public http_session<ssl_http_session>
    , public std::enable_shared_from_this<ssl_http_session>
{
    boost::beast::ssl_stream<tcp::socket> stream_;
    boost::asio::strand<
        boost::asio::io_context::executor_type> strand_;
    bool eof_ = false;

public:
    // Create the http_session
    ssl_http_session(
        tcp::socket socket,
        ssl::context& ctx,
        boost::beast::flat_buffer buffer,
        httpserver::router& router)
        : http_session<ssl_http_session>(
            socket.get_executor().context(),
            std::move(buffer),
            router)
        , stream_(std::move(socket), ctx)
        , strand_(stream_.get_executor())
    {
    }

    // Called by the base class
    boost::beast::ssl_stream<tcp::socket>&
    stream()
    {
        return stream_;
    }

    // Called by the base class
    boost::beast::ssl_stream<tcp::socket>
    release_stream()
    {
        return std::move(stream_);
    }

    // Start the asynchronous operation
    void
    run()
    {
        // Make sure we run on the strand
        if(! strand_.running_in_this_thread())
            return boost::asio::post(
                boost::asio::bind_executor(
                    strand_,
                    std::bind(
                        &ssl_http_session::run,
                        shared_from_this())));

        // Run the timer. The timer is operated
        // continuously, this simplifies the code.
        on_timer({});

        // Set the timer
        timer_.expires_after(std::chrono::seconds(15));

        // Perform the SSL handshake
        // Note, this is the buffered version of the handshake.
        stream_.async_handshake(
            ssl::stream_base::server,
            buffer_.data(),
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &ssl_http_session::on_handshake,
                    shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2)));
    }
    void
    on_handshake(
        boost::system::error_code ec,
        std::size_t bytes_used)
    {
        // Happens when the handshake times out
        if(ec == boost::asio::error::operation_aborted)
            return;

        if(ec)
            return fail(ec, "handshake");

        // Consume the portion of the buffer used by the handshake
        buffer_.consume(bytes_used);

        do_read();
    }

    void
    do_eof()
    {
        eof_ = true;

        // Set the timer
        timer_.expires_after(std::chrono::seconds(15));

        // Perform the SSL shutdown
        stream_.async_shutdown(
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &ssl_http_session::on_shutdown,
                    shared_from_this(),
                    std::placeholders::_1)));
    }

    void
    on_shutdown(boost::system::error_code ec)
    {
        // Happens when the shutdown times out
        if(ec == boost::asio::error::operation_aborted)
            return;

        if(ec)
            return fail(ec, "shutdown");

        // At this point the connection is closed gracefully
    }

    void
    do_timeout()
    {
        // If this is true it means we timed out performing the shutdown
        if(eof_)
            return;

        // Start the timer again
        timer_.expires_at(
            (std::chrono::steady_clock::time_point::max)());
        on_timer({});
        do_eof();
    }
};

//------------------------------------------------------------------------------
// Detects SSL handshakes
class detect_session : public std::enable_shared_from_this<detect_session>
{
    tcp::socket socket_;
    ssl::context& ctx_;
    boost::asio::strand<
    boost::asio::io_context::executor_type> strand_;
    httpserver::router& router_;
    boost::beast::flat_buffer buffer_;

public:
    explicit
    detect_session(
        tcp::socket socket,
        ssl::context& ctx,
        httpserver::router& router)
        : socket_(std::move(socket))
        , ctx_(ctx)
        , strand_(socket_.get_executor())
        , router_(router)
    {
    }

    // Launch the detector
    void
    run()
    {
        async_detect_ssl(
            socket_,
            buffer_,
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &detect_session::on_detect,
                    shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2)));

    }

    void
    on_detect(boost::system::error_code ec, boost::tribool result)
    {
        if(ec)
            return fail(ec, "detect");

        if(result)
        {
            // Launch SSL session
            auto session = std::make_shared<ssl_http_session>(
                std::move(socket_),
                ctx_,
                std::move(buffer_),
                router_);
            session->run();
            return;
        }else
        {
            // Launch plain session
            auto session = std::make_shared<plain_http_session>(
                std::move(socket_),
                std::move(buffer_),
                router_);
            session->run();
        }
    }
};


// Send function
template <class R>
void send(R&& response)
{
    http_session_i* ptr = get_http_session();
    ssl_http_session* ptrSSl = dynamic_cast<ssl_http_session*>(ptr);
    if(ptrSSl)
        ptrSSl->send(response);
    else{
        plain_http_session* ptrPlain = dynamic_cast<plain_http_session*>(ptr);
        if(ptrPlain)
            ptrPlain->send(response);
    }
}


}
#endif // SESSION_H
