#ifndef SESSEION_H
#define SESSEION_H
#include "request.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>
#include <boost/any.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <thread>
#include <string>
#include <unordered_map>

namespace httpserver {

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;         // from <boost/beast/websocket.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;

class session
{    
protected:
    // This queue is used for HTTP pipelining.
    template<class Self>
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

        Self& self_;
        std::vector<std::unique_ptr<work>> items_;

    public:
        explicit
        queue(Self& self)
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
                Self& self_;
                http::message<isRequest, Body, Fields> msg_;

                work_impl(
                    Self& self,
                    http::message<isRequest, Body, Fields>&& msg)
                    : self_(self)
                    , msg_(std::move(msg))
                {
                }

                void
                operator()()
                {
                    http::async_write(
                        self_.stream_,
                        msg_,
                        beast::bind_front_handler(
                            &Self::on_write,
                            self_.shared_from_this(),
                            msg_.need_eof()));
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

    // The parser is stored in an optional container so we can
    // construct it from scratch it at the beginning of each new message.
    boost::optional<request_parser_empty> parser_;
    std::unordered_map<std::string, boost::any> data_;
    dynamic_request request_;

public:
    session();
    virtual ~session(){}

    dynamic_request& request();

    virtual void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred) = 0;

    // Return a object in map
    template< class type>
    type& get_data(const char* key){
        auto&& itr = data_.find(key);
        if(itr != data_.end())
        {
            boost::any& a = itr->second;
            return boost::any_cast<type&>(a);
        }
        throw std::runtime_error(std::string("http_session::data: ")+key+" not found");
    }

    // Return a object in map, return default id not fond
    template< class type>
    type& get_data(const char* key, const type& defaultValue){
        auto&& itr = data_.find(key);
        if(itr != data_.end())
        {
            boost::any& a = itr->second;
            return boost::any_cast<type&>(a);
        }
        return const_cast<type&>(defaultValue);
    }

    // Put a object in map
    template< class type>
    void set_data(const char* key, type&& obj){
        data_[key] = obj;
    }
};

extern std::unordered_map<std::thread::id, session*> map_http_session;

session& get_http_session();



}
#endif // SESSEION_H
