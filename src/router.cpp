#include "router.h"

namespace httpserver
{

Router::Router(const string& public_dir_) :
    public_dir{public_dir_}
{

}

void Router::add(const string &path, func_route route, func_filter filter)
{
    mapRoutes.insert({path, {route, filter}});
}

void Router::route(httpserver::request &&request, httpserver::response &response)
{
    size_t end = request.target().find('?');
    if(end == string::npos)
        end = request.target().find('#');

    auto path = request.target().substr(0, end);
    auto itr = mapRoutes.find(path.to_string());
    if(itr != mapRoutes.end()){
        //contatos::request req = request;
        auto& pair = itr->second;
        auto& func = pair.first;
        auto& filter = pair.second;
        if(filter != nullptr){
            if( filter(request, response) )
                response = func(request);
        }else
            response = func(request);
    }else{
        if(route_to_file(public_dir+path.to_string(), response))
            return;

        response.result(http::status::not_found);
        response.set(http::field::content_type, "text/plain");
        response.body() = "File not found\r\n";
    }
}

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view
mime_type(boost::beast::string_view path)
{
    using boost::beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == boost::beast::string_view::npos)
            return boost::beast::string_view{};
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}


bool Router::route_to_file(const string &filename, http::response<http::string_body> &response)
{
    std::ifstream file(filename);
    if (file.good()) {
        stringstream ss; ss << file.rdbuf();//read the file
        response.set(http::field::content_type, mime_type(filename));
        response.body() = ss.str();

        /*struct stat stbuf;
        if (stat(path.c_str(), &stbuf) == 0) {
            response << boost::network::header("last-modified", to_string(stbuf.st_mtim) );
        }*/

        return true;
    }
    return false;
}

}
