#ifndef ROUTER_HPP_INCLUDED
#define ROUTER_HPP_INCLUDED

#include <boost/network/protocol/http/server.hpp>
#include <boost/network/protocol/http/request.hpp>
#include <iostream>
#include <fstream>
#include <functional>
#include <exception>
#include "request.hpp"

using namespace std;
namespace http = boost::network::http;

struct router;
typedef http::server<router> http_server;
typedef std::function<void(const http_server::request& request, http_server::response &response)> func_route;
typedef std::function<bool(const http_server::request& request, http_server::response &response)> func_filter;

struct router
{


    vector<string> docroot;

    router(vector<string> docroot){
        this->docroot = docroot;
    }
    router(){}

    void operator() (http_server::request const &req, http_server::response &response) {
        try{
            route(req, response);
        }catch(const std::exception& ex){
            std::cerr << "router::operator() - ERROR: " << ex.what() << ", on request: "<<req.destination << ", with body: "<< req.body << '\n';
            response = http_server::response::stock_reply(http_server::response::internal_server_error, ex.what());
        }
    }

    void log(http_server::string_type const &info) {
        std::clog  << "ERROR: " << info << '\n';
    }

    void add(const string& path, func_route route, func_filter filter=nullptr){
        routes.insert({path, {route, filter}});
    }

    void route(http_server::request const &request, http_server::response &response) {
        size_t end = request.destination.find('?');
        if(end == string::npos)
            end = request.destination.find('#');

        string path = request.destination.substr(0, end);
        auto itr = routes.find(path);
        if(itr != routes.end()){
            //contatos::request req = request;
            auto pair = itr->second;
            auto route = pair.first;
            auto filter = pair.second;
            if(filter != nullptr){
                if( filter(request, response) )
                    route(request, response);
            }else
                route(request, response);
        }else{

            if (path == "/") {
                path = "/index.html";
            }

            for(auto& dir: docroot)
                if(route_to_file(dir+path, response))
                    return;
            
            response = http_server::response::stock_reply( http_server::response::not_found, "not found" );
        }
    }
    
    bool route_to_file(const string& filename, http_server::response& response)
    {
        std::ifstream file(filename);
        if (file.good()) {
            stringstream stream;
            stream << file.rdbuf();//read the file
            response = http_server::response::stock_reply( http_server::response::ok, stream.str() );
            
            /*struct stat stbuf;
            if (stat(path.c_str(), &stbuf) == 0) {
                response << boost::network::header("last-modified", to_string(stbuf.st_mtim) );
            }*/
           
            return true;
        }
        return false;
    }

private:
    map<string, pair<func_route, func_filter> >routes;
};


#endif // ROUTER_HPP_INCLUDED
