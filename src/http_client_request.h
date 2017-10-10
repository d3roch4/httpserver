#ifndef HTTP_CLIENT_REQUEST_H
#define HTTP_CLIENT_REQUEST_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/regex.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>


enum method
{
    GET,
    POST
};


void load_root_certificates(ssl::context& ctx);
http::response<http::string_body> http_client_request(const string& url,
                       const string& params = "",
                       http::verb method = http::verb::get,
                       const string& content_type = "application/x-www-form-urlencoded",
                       bool redirects = true,
                       int timeout = 10);



http::response<http::string_body> http_client_request_no_ssl(const string& url,
                       const string& params = "",
                       http::verb method = http::verb::get,
                       const string& content_type = "application/x-www-form-urlencoded",
                       bool redirects = true,
                       int timeout = 10)
{
    // The io_service is required for all I/O
    boost::asio::io_service ios;

    // These objects perform our I/O
    tcp::resolver resolver{ios};
    tcp::socket socket{ios};


    boost::regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
    boost::cmatch what;
    if( regex_match(url.c_str(), what, ex))
    {
        cout << "protocol: " << string(what[1].first, what[1].second) << endl;
        cout << "domain:   " << string(what[2].first, what[2].second) << endl;
        cout << "port:     " << string(what[3].first, what[3].second) << endl;
        cout << "path:     " << string(what[4].first, what[4].second) << endl;
        cout << "query:    " << string(what[5].first, what[5].second) << endl;
        cout << "fragment: " << string(what[6].first, what[6].second) << endl;
    }

    // Look up the domain name
    auto const lookup = resolver.resolve( tcp::resolver::query(string(what[2].first, what[2].second), string(what[3].first, what[3].second)) );

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

    // Receive the HTTP response
    boost::system::error_code ec;
    http::read(socket, buffer, res, ec);
    if(ec)
        throw boost::system::system_error{ec};

    if(redirects && res.base().result_int() == 301)
        return http_client_request(url, params, method, content_type, redirects, timeout);

    return res;
}

http::response<http::string_body> http_client_request_ssl(const string& url,
                       const string& params = "",
                       http::verb method = http::verb::get,
                       const string& content_type = "application/x-www-form-urlencoded",
                       bool redirects = true,
                       int timeout = 10)
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

    boost::regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
    boost::cmatch what;
    if( regex_match(url.c_str(), what, ex))
    {
        cout << "protocol: " << string(what[1].first, what[1].second) << endl;
        cout << "domain:   " << string(what[2].first, what[2].second) << endl;
        cout << "port:     " << string(what[3].first, what[3].second) << endl;
        cout << "path:     " << string(what[4].first, what[4].second) << endl;
        cout << "query:    " << string(what[5].first, what[5].second) << endl;
        cout << "fragment: " << string(what[6].first, what[6].second) << endl;
    }

    // Look up the domain name
    auto const lookup = resolver.resolve( tcp::resolver::query(string(what[2].first, what[2].second), string(what[3].first, what[3].second)) );

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




http::response<http::string_body> http_client_request(const string& url,
                       const string& params,
                       http::verb method,
                       const string& content_type,
                       bool redirects,
                       int timeout)
{
    if(url.find("https") != string::npos)
        return http_client_request_ssl(url, params, method, content_type, redirects, timeout);
    else
        return http_client_request_no_ssl(url, params, method, content_type, redirects, timeout);
}


namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>


// The template argument is gratuituous, to
// allow the implementation to be header-only.
//
template<class = void>
void __load_root_certificates(ssl::context& ctx, boost::system::error_code& ec)
{
    std::string const cert =
        /*  This is the DigiCert root certificate.

            CN = DigiCert High Assurance EV Root CA
            OU = www.digicert.com
            O = DigiCert Inc
            C = US

            Valid to: Sunday, ?November ?9, ?2031 5:00:00 PM

            Thumbprint(sha1):
            5f b7 ee 06 33 e2 59 db ad 0c 4c 9a e6 d3 8f 1a 61 c7 dc 25
        */
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n"
        "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
        "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n"
        "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n"
        "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n"
        "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n"
        "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n"
        "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n"
        "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n"
        "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n"
        "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n"
        "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n"
        "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n"
        "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n"
        "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n"
        "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n"
        "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n"
        "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n"
        "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"
        "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n"
        "+OkuE6N36B9K\n"
        "-----END CERTIFICATE-----\n"
        /*  This is the GeoTrust root certificate.

            CN = GeoTrust Global CA
            O = GeoTrust Inc.
            C = US
            Valid to: Friday, ‎May ‎20, ‎2022 9:00:00 PM

            Thumbprint(sha1):
            ‎de 28 f4 a4 ff e5 b9 2f a3 c5 03 d1 a3 49 a7 f9 96 2a 82 12
        */
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n"
        "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
        "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n"
        "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n"
        "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n"
        "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n"
        "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n"
        "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n"
        "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n"
        "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n"
        "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n"
        "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n"
        "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n"
        "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n"
        "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n"
        "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n"
        "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n"
        "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n"
        "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"
        "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n"
        "+OkuE6N36B9K\n"
        "-----END CERTIFICATE-----\n"
        ;

    ctx.add_certificate_authority(
        boost::asio::buffer(cert.data(), cert.size()), ec);
    if(ec)
        return;
}


// Load the root certificates into an ssl::context
//
// This function is inline so that its easy to take
// the address and there's nothing weird like a
// gratuituous template argument; thus it appears
// like a "normal" function.
//

inline
void
load_root_certificates(ssl::context& ctx, boost::system::error_code& ec)
{
    __load_root_certificates(ctx, ec);
}

inline
void
load_root_certificates(ssl::context& ctx)
{
    boost::system::error_code ec;
    load_root_certificates(ctx, ec);
    if(ec)
        throw boost::system::system_error{ec};
}





#endif
#if 0

#include <iostream>
#include <boost/network/protocol/http/client.hpp>
using namespace std;

enum method
{
    GET,
    POST
};

typedef boost::network::http::client http_client;

http_client::response http_client_request(const string& url,
                       const string& params = "",
                       method method = GET,
                       const string& content_type = "application/x-www-form-urlencoded",
                       bool redirects = true,
                       int timeout = 10);

#endif // CLIENT_REQUEST_H
