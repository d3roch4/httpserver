#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/make_unique.hpp>
#include <boost/config.hpp>
#include <boost/any.hpp>
#include <thread>
#include <unordered_map>
#include "request.h"


namespace httpserver {

using namespace httpserver;
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;            // from <boost/beast/http.hpp>
class router;

// Handles an HTTP server connection
class http_session : public std::enable_shared_from_this<http_session>
{
    // This queue is used for HTTP pipelining.
    class queue_responses
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
        queue_responses(http_session& self)
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
                        self_.socket_,
                        msg_,
                        boost::asio::bind_executor(
                            self_.strand_,
                            std::bind(
                                &http_session::on_write,
                                self_.shared_from_this(),
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

    tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer timer_;
    boost::beast::flat_buffer buffer_;
    request_parser_empty request_parser_;
    queue_responses queue_;
    router& router_;
    std::unordered_map<std::string, boost::any> data_;

public:
    // Take ownership of the socket
    explicit
    http_session(tcp::socket socket, router& router_);

    tcp::socket& socket()
    {
        return socket_;
    }
    boost::beast::flat_buffer& buffer()
    {
        return buffer_;
    }

    // put a response in queue
    template <class R>
    void send(R&& response)
    {
        queue_(std::move(response));
    }

    // Return the request by client
    request_parser_empty &request_parser();

    // Return a object in map
    template< class type>
    type& data(const char* key){
        auto&& itr = data_.find(key);
        if(itr != data_.end())
        {
            boost::any& a = itr->second;
            return boost::any_cast<type&>(a);
        }
        throw std::runtime_error(std::string("http_session::data: ")+key+" not found");
    }    

    // Put a object in map
    template< class type>
    void data(const char* key, type&& obj){
        data_[key] = obj;
    }

    // Start the asynchronous operation
    void
    run();

    void
    do_read();

    // Called when the timer expires.
    void
    on_timer(boost::system::error_code ec);

    void
    on_read(boost::system::error_code ec);

    void
    on_write(boost::system::error_code ec, bool close);

    void
    do_close();
};

//------------------------------------------------------------------------------

extern std::unordered_map<std::thread::id, http_session*> map_http_session;

http_session& get_http_session();

}

#endif // HTTP_SESSION_H
