#include "response.h"

namespace httpserver
{

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

void send_file(const std::string &filename)
{
    // Attempt to open the file
    boost::beast::error_code ec;
    boost::beast::http::file_body::value_type body;
    body.open(filename.c_str(), boost::beast::file_mode::scan, ec);

    http_session* hs = map_http_session[std::this_thread::get_id()];
    request_empty& req = hs->request_parser().get();

    // Handle the case where the file doesn't exist
    if(ec == boost::system::errc::no_such_file_or_directory)
        return not_found(req.target().to_string());

    // Handle an unknown error
    if(ec)
        return server_error(ec.message());

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if(req.method() == verb::head)
    {
        boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::ok, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, mime_type(filename));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }
    // Respond to GET request
    boost::beast::http::response<boost::beast::http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(boost::beast::http::status::ok, req.version())};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, mime_type(filename));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
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
