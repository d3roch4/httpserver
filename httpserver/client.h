#ifndef CLIENT_H
#define CLIENT_H

#include <httpserver/parser/json.h>
#include <boost/asio/io_service.hpp>

namespace httpserver
{
namespace http = boost::beast::http;

struct client
{
    using tcp = boost::asio::ip::tcp;

    std::string base_url;
    // The io_service is required for all I/O
    boost::asio::io_service ios;
    // These objects perform our I/O
    tcp::resolver resolver{ios};
    tcp::socket socket{ios};
    tcp::resolver::results_type lookup;    

    // This buffer is used for reading and must be persisted
    boost::beast::flat_buffer buffer;
    // The parser message.
    http::parser<false, http::string_body> parser;

    client(std::string base_url);

    void connect();

    response request(verb method, const std::string& path, const JSONObject& json);

    response request(verb method, const std::string& url,
                            const std::string& params = "",
                            const std::string& content_type = "application/x-www-form-urlencoded",
                            bool redirects = true,
                            int timeout = 10);

    response request_ssl(const std::string &url, const std::string &params, boost::beast::http::verb method, const std::string &content_type, bool redirects, int timeout);

    response request_no_ssl(const std::string &url, const std::string &params, boost::beast::http::verb method, const std::string &content_type, bool redirects, int timeout);

};

}

#endif // CLIENT_H
