#include "httpserver.h"

#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>

namespace httpserver
{

using namespace boost::asio;         // from <boost/asio.hpp>
using namespace boost::beast;    // from <boost/beast/http.hpp>

HttpServer::HttpServer(const string& address, unsigned short port, const string& public_dir ) :
    acceptor{ios, {ip::address::from_string(address), port}},
    router{public_dir}
{
    http_server(acceptor, socket);
}

void HttpServer::addRoute(const string &path, func_route route, func_filter filter)
{
    router.add(path, route, filter);
}

void HttpServer::run()
{
    ios.run();
}

namespace my_program_state
{
    std::size_t
    request_count()
    {
        static std::size_t count = 0;
        return ++count;
    }

    std::time_t
    now()
    {
        return std::time(0);
    }
}

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    http_connection(tcp::socket socket, Router& router)
        : socket_(std::move(socket)),
          router_{&router}
    {
    }

    // Initiate the asynchronous operations associated with the connection.
    void
    start()
    {
        read_request();
        check_deadline();
    }

private:
    // The socket for the currently connected client.
    tcp::socket socket_;
    Router* router_;

    // The buffer for performing reads.
    boost::beast::flat_buffer buffer_{8192};

    // The request message.
    http::request<http::string_body> request_;

    // The response message.
    http::response<http::string_body> response_;

    // The timer for putting a deadline on connection processing.
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> deadline_{
        socket_.get_io_service(), std::chrono::seconds(60)};

    // Asynchronously receive a complete request message.
    void read_request()
    {
        auto self = shared_from_this();

        http::async_read( socket_, buffer_, request_,
            [self](boost::beast::error_code ec, std::size_t bytes_transferred)
            {
                boost::ignore_unused(bytes_transferred);
                if(!ec)
                    self->process_request();
            });
    }

    // Determine what needs to be done with the request message.
    void process_request()
    {
        try{
            router_->route(request_, response_);
        }catch(const std::exception& ex){
            std::cerr << "router::operator() - ERROR: " << ex.what() << ", on request: "<<request_.target().to_string() << ", with body: "/*<< request_.body().to_string()*/ << '\n';
            response_.result(http::status::internal_server_error);
            response_.set(http::field::content_type, "text/plain");
            response_.body() = ex.what();
        }

        write_response();
    }

    // Asynchronously transmit the response message.
    void write_response()
    {
        auto self = shared_from_this();

        response_.set(http::field::content_length, response_.body().size());

        http::async_write(socket_, response_, [self](boost::beast::error_code ec, std::size_t)
        {
            self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            self->deadline_.cancel();
        });
    }

    // Check whether we have spent enough time on this connection.
    void check_deadline()
    {
        auto self = shared_from_this();

        deadline_.async_wait([self](boost::beast::error_code ec)
        {
            if(!ec)
            {
                // Close socket to cancel any outstanding operation.
                self->socket_.close(ec);
            }
        });
    }
};


void HttpServer::http_server(tcp::acceptor &acceptor, tcp::socket &socket)
{
    acceptor.async_accept(socket, [&](boost::beast::error_code ec)
      {
          if(!ec)
              std::make_shared<http_connection>(std::move(socket), router)->start();
          http_server(acceptor, socket);
      });
}

}
