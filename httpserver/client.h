#ifndef CLIENT_H
#define CLIENT_H

#include <d3util/json.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ssl/stream.hpp>
#include "response.h"

namespace httpserver
{
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;
using verb = boost::beast::http::verb;

struct client
{
    using tcp = boost::asio::ip::tcp;

    bool useSSL;
    std::string base_url;
    std::string host;
    // The io_service is required for all I/O
    boost::asio::io_service ios;
    // These objects perform our I/O
    tcp::resolver resolver{ios};
    tcp::socket socket{ios};
    tcp::resolver::results_type lookup;
    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23_client};
    // These objects perform our I/O
    ssl::stream<tcp::socket> stream{ios, ctx};
    // This buffer is used for reading and must be persisted
    boost::beast::flat_buffer buffer;

    // The parser message.
//    http::parser<false, http::string_body> parser;

    client(std::string base_url);
    client();

    void connect();

    response request(verb method, const std::string& path, const JSONObject& json);

    response request(verb method, const std::string& url,
                            const std::string& params = "",
                            const std::string& content_type = "application/x-www-form-urlencoded",
                            bool redirects = true,
                            int timeout = 10);

    response request(http::request<http::string_body> &req);

};

}

#endif // CLIENT_H
