#include "httpserver.h"

#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace httpserver
{

using namespace boost::asio;         // from <boost/asio.hpp>
using namespace boost::beast;    // from <boost/beast/http.hpp>

HttpServer::HttpServer(const string& address, unsigned short port, const string& public_dir, size_t threads) :
    thread_qtd{threads},
    ios{threads},
    acceptor{ios, {ip::address::from_string(address), port}},
    router{public_dir}
{
    loop(acceptor, socket);
}

void HttpServer::run()
{
    for(;;){
        ios.poll();
        std::this_thread::sleep_for(chrono::microseconds(500));
    }
}

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    http_connection(tcp::socket socket, Router& router)
        : socket_(std::move(socket)),
          router_{&router}
    {
        parser_.body_limit(1024 * 1024 * 32); // 32MB of limit upload file
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

    // The parser message.
    http::parser<true, http::string_body> parser_; // limit body of 32MB

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

        http::async_read( socket_, buffer_, parser_.base(),
            [self](boost::beast::error_code ec, std::size_t bytes_transferred)
            {
                boost::ignore_unused(bytes_transferred);
                if(!ec){
                    self->request_ = self->parser_.release();
                    self->process_request();
                }
                else{
                    std::cerr << BOOST_CURRENT_FUNCTION <<" - ERROR: " << ec.message()
                              << ", on request: "<<self->request_.target().to_string() << std::endl;
                    self->response_erro(ec.message());
                }
                self->write_response();
            });
    }

    // Determine what needs to be done with the request message.
    void process_request()
    {
        try{
            router_->route(request_, response_);
        }catch(const std::exception& ex){
            std::cerr << BOOST_CURRENT_FUNCTION <<" - ERROR: " << ex.what()
                      << ", on request: "<<request_.target().to_string()
                      << ", with body: "<< request_.body() << '\n';
            response_erro(ex.what());
        }
    }

    void response_erro(const string& msg){
        response_.result(http::status::internal_server_error);
        response_.set(http::field::content_type, "text/plain");
        response_.body() = msg;
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


void HttpServer::loop(tcp::acceptor &acceptor, tcp::socket &socket)
{
    acceptor.async_accept(socket, [&](boost::beast::error_code ec)
      {
          if(!ec)
              std::make_shared<http_connection>(std::move(socket), router)->start();
          loop(acceptor, socket);
      });
}

}
