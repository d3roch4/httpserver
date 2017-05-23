#ifndef REQUEST_HPP_INCLUDED
#define REQUEST_HPP_INCLUDED

#include <boost/network/protocol/http/server.hpp>
#include <boost/network/uri/decode.hpp>
#include <boost/algorithm/string.hpp>
#include <unordered_map>
#include <iostream>
#include <json/json.h>

using namespace std;
namespace http = boost::network::http;

struct handler;

namespace _http_server {
struct request : http::server<handler>::request{
    std::unordered_map<std::string, std::string> parameters;
    Json::Value data;
    
    request(const http::server<handler>::request& req)
    {
        http::server<handler>::request::operator=(req);
        std::size_t found = req.destination.find('?');
        if(found != std::string::npos){
            const string& query = req.destination.substr(found+1);
            insert_parameters(query);
        }
        insert_parameters(req.body);
        Json::Reader reader;
        reader.parse(req.body, data);
    }
    void insert_parameters(const string& query)
    {
        if( ! query.empty()){
            vector<string> vecStr;
            if(query.find('&') != string::npos)
                boost::split(vecStr, query, boost::is_any_of("&"));
            else
                vecStr.push_back(query);

            for(const string& param: vecStr){
                size_t sep = param.find('=');
                const string& key = param.substr(0, sep);
                const string& val = boost::network::uri::decoded(param.substr(sep+1));
                parameters.insert({key, val});
            }
        }
    }
};

}

#endif // REQUEST_HPP_INCLUDED
