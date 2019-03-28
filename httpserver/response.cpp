#include "response.h"
#include "request.h"
#include <boost/filesystem.hpp>
#include <iomanip>
#include "compress.h"

namespace httpserver
{
using field = boost::beast::http::field;

httpserver::response getCompressResponse(const string& str)
{
    using namespace httpserver;
    const auto& req = httpserver::request();
    httpserver::response resp;
    boost::string_view encoding = req[field::accept_encoding];

    if(encoding.find("gzip") != boost::string_view::npos && str.size()>1450){
        resp.body() = compress_gzip(str);
        resp.set(field::content_encoding, "gzip");
    }else if(encoding.find("deflate") != boost::string_view::npos && str.size()>1450){
        resp.body() = compress_deflate(str);
        resp.set(field::content_encoding, "deflate");
    }else
        resp.body() = str;

    return resp;
}

void compress_and_send(const string &str)
{
    httpserver::response resp = getCompressResponse(str);
    send(resp);
}

void ok(const std::string& content)
{
    compress_and_send(content);
}

void ok(const string &content, const string &type, bool compress)
{
    response res = getCompressResponse(content);
    res.set(boost::beast::http::field::content_type, type);
    send(res);
}

type_function_resp_default created = [](const std::string& local)
{
    response resp;
    resp.result(status::created);
    resp.set("Local", local );
    send(resp);
};

type_function_resp_default accepted = [](const std::string& content)
{
    response resp;
    resp.result(status::accepted);
    resp.body() = content;
    send(resp);
};

type_function_resp_default forbidden = [](const std::string& why)
{
    response res;
    res.result(boost::beast::http::status::bad_request);
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.body() = why;
    res.keep_alive(false);
    res.prepare_payload();
    send(res);
};

type_function_resp_default bad_request = [](const std::string& why)
{
    response res;
    res.result(boost::beast::http::status::bad_request);
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.body() = why;
    res.keep_alive(false);
    res.prepare_payload();
    send(res);
};

type_function_resp_default not_found = [](const std::string& target)
{
    response res;
    res.result(boost::beast::http::status::not_found);
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.body() = "The resource '" + target + "' was not found.";
    res.keep_alive(false);
    res.prepare_payload();
    send(res);
};

type_function_resp_default server_error = [](const std::string& what)
{
   response res;
   res.result(boost::beast::http::status::internal_server_error);
   res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
   res.set(boost::beast::http::field::content_type, "text/html");
   res.body() = "An error occurred: '" + what + "'";
   res.prepare_payload();
   res.keep_alive(false);
   send(res);
};

type_function_resp_default redirect_to = [](const std::string& url)
{
    response res;
    res.result(boost::beast::http::status::temporary_redirect);
    res.set(field::location, url);
    send(res);
};

std::function<void(const std::vector<std::string>&)> invalid_parameters = [](const std::vector<std::string>& parameters)
{
    std::string erro = "Processing parameters: ";
    erro += parameters[0];
    for(int i=1; i<parameters.size(); i++)
        erro += ", "+parameters[i];
    server_error(erro);
};

response &operator <<(response& resp, const std::string &str)
{
    resp.body() += str;
    return resp;
}

response &operator <<(response&& resp, const std::string &str)
{
    resp.body() += str;
    return resp;
}

response &operator <<(response&& resp, const char* str)
{
    resp.body() += str;
    return resp;
}

template<class R>
void setHeader(R& res, const std::string &filename, const size_t size, std::time_t lastWrite)
{
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&lastWrite), "%a, %d %b %Y %T GMT");

    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, mime_type(filename));
    res.set(boost::beast::http::field::last_modified, ss.str());
//    res.set(boost::beast::http::field::cache_control, "public");
    res.content_length(size);
//    res.keep_alive(req.keep_alive());
}

void send_file(const std::string &filename)
{
    http_session_i* hs = get_http_session();
    const auto& req = hs->request();

    // Attempt to open the file
    boost::beast::error_code ec;
    boost::beast::http::file_body::value_type body;
    body.open(filename.c_str(), boost::beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if(ec == boost::system::errc::no_such_file_or_directory)
        return not_found(to_string(req.method()).to_string()+':'+req.target().to_string());

    // Handle an unknown error
    if(ec)
        return server_error(ec.message());

    // Cache the size since we need it after the move
    auto const size = body.size();
    std::time_t lastWrite = boost::filesystem::last_write_time(filename);

    // Respond to HEAD request
    if(req.method() == verb::head)
    {
        boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::ok, req.version()};
        setHeader(res, filename, size, lastWrite);
        return send(std::move(res));
    }
    // Respond to GET request
    boost::beast::http::response<boost::beast::http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(boost::beast::http::status::ok, req.version())};
    setHeader(res, filename, size, lastWrite);
    return send(std::move(res));
}

boost::beast::string_view mime_type(boost::beast::string_view path)
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

}
