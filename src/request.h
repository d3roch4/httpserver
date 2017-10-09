#ifndef REQUEST_H
#define REQUEST_H

#include <boost/beast/http.hpp>
#include <boost/algorithm/string.hpp>
#include <unordered_map>
#include <iostream>
#include <json/json.h>

namespace httpserver {

using namespace std;
using namespace boost::beast;    // from <boost/beast/http.hpp>

class request : public http::request<http::string_body>
{    
public:
    std::unordered_map<std::string, std::string> parameters;
    std::unordered_map<std::string, std::string> files;
    Json::Value data;
    request(http::request<http::string_body>& req);
};

}
#endif // REQUEST_H
