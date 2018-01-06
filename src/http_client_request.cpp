#include "http_client_request.h"

void load_root_certificates(boost::asio::ssl::context &ctx, boost::system::error_code &ec)
{
    __load_root_certificates(ctx, ec);
}

void load_root_certificates(boost::asio::ssl::context &ctx)
{
    boost::system::error_code ec;
    load_root_certificates(ctx, ec);
    if(ec)
        throw boost::system::system_error{ec};
}

http::response<boost::beast::http::string_body> http_client_request(const std::string &url, const std::string &params, boost::beast::http::verb method, const std::string &content_type, bool redirects, int timeout)
{
    if(url.find("https") != string::npos)
        return http_client_request_ssl(url, params, method, content_type, redirects, timeout);
    else
        return http_client_request_no_ssl(url, params, method, content_type, redirects, timeout);
}

http::response<boost::beast::http::string_body> http_client_request_ssl(const std::string &url, const std::string &params, boost::beast::http::verb method, const std::string &content_type, bool redirects, int timeout)
{

    // The io_service is required for all I/O
    boost::asio::io_service ios;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23_client};

    // This holds the root certificate used for verification
    load_root_certificates(ctx);

    // These objects perform our I/O
    tcp::resolver resolver{ios};
    ssl::stream<tcp::socket> stream{ios, ctx};

    string port;
    boost::regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
    boost::cmatch what;
    if( regex_match(url.c_str(), what, ex))
    {
        port.assign(what[3].first, what[3].second);
        if(port.empty()) port = "443";
#ifdef DEBUG
        cout << "protocol: " << string(what[1].first, what[1].second) << endl;
        cout << "domain:   " << string(what[2].first, what[2].second) << endl;
        cout << "port:     " << string(what[3].first, what[3].second) << endl;
        cout << "path:     " << string(what[4].first, what[4].second) << endl;
        cout << "query:    " << string(what[5].first, what[5].second) << endl;
        cout << "fragment: " << string(what[6].first, what[6].second) << endl;
#endif
    }

    // Look up the domain name
    auto const lookup = resolver.resolve( tcp::resolver::query(string(what[2].first, what[2].second), port) );

    // Make the connection on the IP address we get from a lookup
    boost::asio::connect(stream.next_layer(), lookup);

    // Perform the SSL handshake
    stream.handshake(ssl::stream_base::client);

    // Set up an HTTP request message
    string target = string(what[4].first, what[4].second);
    string query = string(what[5].first, what[5].second);
    if( method == http::verb::get ){
        query += (query.size()?"&":"") + (params.size()?params:"");
    }
    target += query.size()?'?'+query:"";

    http::request<http::string_body> req{method, target, 11};
    req.set(http::field::host, string(what[2].first, what[2].second));
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    if(method != http::verb::get){
        req.set(http::field::content_length, params.size());
        req.set(http::field::content_type, content_type);
        req.body() = params;
    }

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    boost::beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::string_body> res;

    // Receive the HTTP response
    boost::system::error_code ec;
    http::read(stream, buffer, res, ec);
    if(ec)
        throw boost::system::system_error{ec};

    if(redirects && res.base().result_int() == 301)
        return http_client_request(url, params, method, content_type, redirects, timeout);

    return res;
}

http::response<boost::beast::http::string_body> http_client_request_no_ssl(const std::string &url, const std::string &params, boost::beast::http::verb method, const std::string &content_type, bool redirects, int timeout)
{
    // The io_service is required for all I/O
    boost::asio::io_service ios;

    // These objects perform our I/O
    tcp::resolver resolver{ios};
    tcp::socket socket{ios};

    string port;
    boost::regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
    boost::cmatch what;
    if( regex_match(url.c_str(), what, ex))
    {
        port.assign(what[3].first, what[3].second);
        if(port.empty()) port = "80";
#ifdef DEBUG
        cout << "protocol: " << string(what[1].first, what[1].second) << endl;
        cout << "domain:   " << string(what[2].first, what[2].second) << endl;
        cout << "port:     " << string(what[3].first, what[3].second) << endl;
        cout << "path:     " << string(what[4].first, what[4].second) << endl;
        cout << "query:    " << string(what[5].first, what[5].second) << endl;
        cout << "fragment: " << string(what[6].first, what[6].second) << endl;
#endif
    }

    // Look up the domain name
    auto const lookup = resolver.resolve( tcp::resolver::query(string(what[2].first, what[2].second), port) );

    // Make the connection on the IP address we get from a lookup
    boost::asio::connect(socket, lookup);

    // Set up an HTTP request message
    string target = string(what[4].first, what[4].second);
    string query = string(what[5].first, what[5].second);
    if( method == http::verb::get ){
        query += (query.size()?"&":"") + (params.size()?params:"");
    }
    target += query.size()?'?'+query:"";

    http::request<http::string_body> req{method, target, 11};
    req.set(http::field::host, string(what[2].first, what[2].second));
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    if(method != http::verb::get){
        req.set(http::field::content_length, params.size());
        req.set(http::field::content_type, content_type);
        req.body() = params;
    }

    // Send the HTTP request to the remote host
    http::write(socket, req);

    // This buffer is used for reading and must be persisted
    boost::beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::string_body> res;

    // The parser message.
    http::parser<false, http::string_body> parser;
    parser.body_limit(1024 * 1024 * 96); // 96MB of limit upload file

    // Receive the HTTP response
    boost::system::error_code ec;
    http::read(socket, buffer, parser.base(), ec);
    if(ec)
        throw boost::system::system_error{ec};
    else
        res = parser.release();

    if(redirects && res.base().result_int() == 301)
        return http_client_request(url, params, method, content_type, redirects, timeout);

    return res;
}
