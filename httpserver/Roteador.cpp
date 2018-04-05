#include "Roteador.h"
#include <iostream>
#include <boost/beast.hpp>
#include "response.h"

namespace httpserver
{

Roteador::Roteador() {}

void Roteador::rota(verb method, const string &path, shared_ptr<parser_request_i> tratador)
{
    mRotas[(int)method][path] = tratador;
}

void Roteador::dispatcher(boost::asio::ip::tcp::socket& socket, boost::beast::flat_buffer& buffer, request_parser &req)
{
    for(auto& method: mRotas){
        if(method.first == (int)req.get().method()){
            size_t end = req.get().target().find('?');
            if (end == string::npos)
                end = req.get().target().find('#');
            const boost::string_view& path = req.get().target().substr(0, end);
            for(auto& mapPath: method.second){
                if(path.find(mapPath.first) == 0){
                    try{
                        return (*mapPath.second)(socket, buffer, req);
                    }catch(const exception& ex){
                        return server_error(req.get().target().to_string()+": "+ex.what());
                    }catch(...){
                        return server_error(req.get().target().to_string()+": Unknow error");
                    }
                }
            }
        }
    }
    send_file(req.get());
}

string Roteador::path_cat(boost::beast::string_view base, boost::beast::string_view path)
{
    if(base.empty())
        return path.to_string();
    std::string result = base.to_string();
#if BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    size_t sizePath = path.rfind("?");
    if(sizePath == boost::beast::string_view::npos)
        sizePath = path.rfind("#");
    if(sizePath == boost::beast::string_view::npos)
        sizePath = path.size();
    result.append(path.data(), sizePath);
#endif
    return result;
}

void Roteador::set_public_dir(const string &dir)
{
    public_dir = std::move(dir);
}


void Roteador::send_file(boost::beast::http::request<http::empty_body> &req)
{
    // Request path must be absolute and not contain "..".
    if( req.target().empty() ||
            req.target()[0] != '/' ||
            req.target().find("..") != boost::beast::string_view::npos)
        return bad_request("Illegal request-target");

    // Build the path to the requested file
    std::string path = path_cat(public_dir, req.target());
    if(req.target().back() == '/')
        path.append("index.html");

    httpserver::send_file(path);
}

}
