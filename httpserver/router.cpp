#include "router.h"
#include <iostream>
#include <boost/beast.hpp>
#include <d3util/stacktrace.h>
#include <d3util/logger.h>
#include "response.h"

namespace httpserver
{
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;            // from <boost/beast/http.hpp>


router::router() {}

void router::add(verb method, shared_ptr<wrap_handle_request_i> tratador)
{
    mRotas[(int)method].push_back(tratador);
}

void router::dispatcher(request_parser_empty& req)
{
    for(auto& rota: mRotas){
        if(rota.first == (int)req.get().method()){
            size_t end = req.get().target().find('?');
            if (end == string::npos)
                end = req.get().target().find('#');
            const boost::string_view& path = req.get().target().substr(0, end);

            //LOG_DEBUG << to_string(req.get().method()) << ' ' << path;

            std::smatch what;
            for(shared_ptr<wrap_handle_request_i> pr: rota.second){
                try{
                    const std::string& str = path.to_string();
                    if(pr->macth(str, what)){
                        const auto& filters = pr->filters;
                        for(const auto& filter: filters)
                            if(filter())
                                return;
                        return pr->exec(what);
                    }
                }catch(const std::exception& ex){
                    string erro = req.get().target().to_string()+": "+ex.what();
                    //LOG_ERROR << boost::diagnostic_information(ex);
                    server_error(erro);
                    print_stacktrace(ex);
                    return;
                }catch(...){
                    return server_error(req.get().target().to_string()+": Unknow error");
                }
            }
        }
    }
    send_file(req.get());
}

string router::path_cat(boost::beast::string_view base, boost::beast::string_view path)
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

void router::set_public_dir(const string &dir)
{
    public_dir = std::move(dir);
}

void router::set_cors_origin(const cors_origin_config &cors_origin)
{
    this->cors_origin = cors_origin;
}

cors_origin_config router::get_cors_origin()
{
    return this->cors_origin;
}

void router::send_file(boost::beast::http::request<http::empty_body> &req)
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
