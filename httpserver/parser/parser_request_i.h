#ifndef CONVERSORQUISICAO_H
#define CONVERSORQUISICAO_H

#include <httpserver/request.h>
#include <httpserver/response.h>
#include <functional>
#include <memory>

namespace httpserver {
namespace parser {

typedef std::function<bool()> function_filter;

/**
 * @brief The parser_request_i struct
 */
struct parser_request_i
{
    std::vector<function_filter> filters;
    virtual bool macth(const std::string& path) = 0;
    virtual void operator()(boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser_empty &req) = 0;
    virtual std::shared_ptr<parser::parser_request_i> copy() = 0;
};

/**
 *
 * @brief The parser_request_creater struct
 */
struct parser_request_creater
{
    template<typename... Args>
    std::shared_ptr<parser_request_i> create(const std::string &path, const auto& function, const Args&... args)
    {
        std::initializer_list<std::string> inputs({args...});
        std::vector<std::string> parametros(inputs);
//        parser_request_i p = (parser_request_i) new parser_request_basic<sizeof...(args)>(path, function, parametros);

        return std::shared_ptr<parser_request_i>();
    }
};

}
}

#endif // PARSERREQUISICAO_H
