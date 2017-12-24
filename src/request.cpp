#include "request.h"
#include <boost/beast/core.hpp>

using namespace std;

bool insert_files(boost::beast::http::request<boost::beast::http::string_body> &req, std::unordered_map<std::string, std::string>& files)
{
    const char* boundary = "+\r\n+";
    const char* filenametag = "filename=";

    size_t sepName = req.body().find(filenametag);
    size_t sepFile = req.body().find(boundary);
    size_t sepFileEnd = req.body().find_last_of(boundary);
    if(sepName!=string::npos && sepFile!=string::npos && sepFileEnd!=string::npos){
        sepName+=strlen(filenametag);;
        const string& name = req.body().substr(sepName, sepFile-sepName);
        const string& file = req.body().substr(sepFile+strlen(boundary), sepFileEnd-sepFile);
        files[name] = file;
        return true;
    }
    return false;
}

void insert_parameters(const auto& query, httpserver::request* req)
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
            const string& val = param.substr(sep+1);
            req->parameters.insert({key, val});
        }
    }
}


httpserver::request::request(http::request<boost::beast::http::string_body> &req)
{
    http::request<boost::beast::http::string_body>::operator=(req);
    std::size_t found = req.target().find('?');
    if(found != std::string::npos){
        const auto& query = req.target().substr(found+1);
        insert_parameters(query.to_string(), this);
    }
    Json::Reader reader;
    if(!insert_files(req, files))
        if(!reader.parse(req.body(), data))
            insert_parameters(req.body(), this);
}
