#ifndef WRAP_HANDLE_REQUEST_I_H
#define WRAP_HANDLE_REQUEST_I_H
#include <functional>
#include <memory>
#include <vector>

namespace httpserver {

typedef std::function<bool()> function_filter;

/**
 * @brief The parser_request_i struct
 */
struct wrap_handle_request_i
{
    std::vector<function_filter> filters;
    virtual bool macth(const std::string& path) = 0;
<<<<<<< HEAD:httpserver/wrap_handle_request_i.h
    virtual void operator()() = 0;
=======
    virtual void operator()(boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser_empty &req) = 0;
    virtual std::shared_ptr<parser::parser_request_i> copy() = 0;
>>>>>>> master:httpserver/parser/parser_request_i.h
};

/**
 *
 * @brief The parser_request_creater struct
 */
struct wrap_handle_request_creator
{
    template<class F, typename... Args>
    std::shared_ptr<wrap_handle_request_i> create(const std::string &path, const F& function, const Args&... args)
    {
        std::initializer_list<std::string> inputs({args...});
        std::vector<std::string> parametros(inputs);
//        parser_request_i p = (parser_request_i) new parser_request_basic<sizeof...(args)>(path, function, parametros);

        return std::shared_ptr<wrap_handle_request_i>();
    }
};

}

#endif // PARSERREQUISICAO_H
