#ifndef RESPONSE_H
#define RESPONSE_H

#include <boost/beast.hpp>
#include <functional>
namespace httpserver
{
typedef boost::beast::http::response<boost::beast::http::string_body> response;
typedef boost::beast::http::status status;

response& operator << (response&& resp, const char* str);
response& operator << (response&& resp, const std::string& str);
response& operator << (response& resp, const std::string& str);

typedef std::function<void(const std::string&)> type_function_resp_default;

void send(const response& resp);

void ok(const std::string& content="");

void ok(const std::string& content, const std::string& type, bool compress);

void created(const std::string& local, const std::string& body="");

extern type_function_resp_default accepted;

extern type_function_resp_default forbidden;

// Returns a bad request response
extern type_function_resp_default bad_request;

// Returns a not found response
extern type_function_resp_default not_found;

// Returns a server error response
extern type_function_resp_default server_error;

// Returns a redirect response
extern type_function_resp_default redirect_to;

// Return invalid parameters reponse
extern std::function<void(const std::vector<std::string>&)> invalid_parameters;

void send_file(const std::string& filename);

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view mime_type(boost::beast::string_view path);

}
#endif // RESPONSE_H
