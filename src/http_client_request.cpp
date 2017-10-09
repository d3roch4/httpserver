
#if 0

#define BOOST_NETWORK_ENABLE_HTTPS
#include <iostream>
#include "http_client_request.h"

using namespace std;

http_client::response http_client_request(const string& url,
                       const string& params,
                       method method,
                       const string& content_type,
                       bool redirects,
                       int timeout)
{
    http_client::options options;
    options.follow_redirects(redirects)
           .timeout(timeout);
    boost::network::http::client client(options);
    http_client::request request;
    http_client::response response;
    switch(method){
    case GET:
        cout << url+(params.size()?'?'+params:"");
        request = http_client::request(url+(params.size()?'?'+params:""));
        request.add_header({"Connection", "close"});
        response = client.get(request);
        break;
    case POST:
        request = http_client::request(url);
        response = client.post(request, params, content_type);
        break;
    default:
        throw runtime_error("client_request: method not reconized");
    }

    return response;
}

#endif
