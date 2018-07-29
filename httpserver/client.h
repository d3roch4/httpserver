#ifndef CLIENT_H
#define CLIENT_H

#include <httpserver/response.h>
#include <httpserver/parser/json.h>

namespace httpserver
{

struct client
{
    std::string base_url;

    client(std::string base_url);

    response request(verb method, const std::string& path, const JSONObject& json);

    static response request(verb method, const std::string& url,
                            const std::string& params = "",
                            const std::string& content_type = "application/x-www-form-urlencoded",
                            bool redirects = true,
                            int timeout = 10);

    static response request_ssl(const std::string &url, const std::string &params, boost::beast::http::verb method, const std::string &content_type, bool redirects, int timeout);

    static response request_no_ssl(const std::string &url, const std::string &params, boost::beast::http::verb method, const std::string &content_type, bool redirects, int timeout);

};

}

#endif // CLIENT_H
