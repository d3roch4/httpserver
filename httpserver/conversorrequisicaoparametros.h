#ifndef CONVERSORREQUISICAOPARAMETROS_H
#define CONVERSORREQUISICAOPARAMETROS_H

#include "parser_request_i.h"
#include <vector>
#include <forward_list>
#include "invoker.h"
#include "parser_body.h"
#include "parser_url.h"

namespace httpserver
{
using namespace std;

template<typename F, typename I, typename B, int NumberParameters>
class ConversorRequisicaoParametros : public parser_request_i
{
    F function;
    I* instance;
    vector<string> parameters;
    string path;
    parser_body<B> parserBody;
public:
    ConversorRequisicaoParametros(const string& path, F ptr, I* instance, const vector<string>& params) {
        this->parameters = params;
        this->path = path;
        this->function = ptr;
        this->instance = instance;
    }

    void operator()(boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser_empty &request)
    {
        const char* url = request.get().target().data();
        std::array<char*, NumberParameters> array;

        if( ! parser_path_to_params<NumberParameters>((char*)url, parameters, array)){
            boost::beast::http::request_parser<boost::beast::http::dynamic_body> req{std::move(request)};
            // Finish reading the message
            read(socket, buffer, req);
            req.release();
            return invalid_parameters(parameters);
        }

        auto tuple = a2t(array);
        B body;
        const auto& tuple2 = parserBody(body, socket, buffer, request, tuple);
        invoker(function, instance, tuple2);
    }
};


template<class B>
struct CriadorConversorRequisicao<B&>
{
    template<class F, class I, typename... Args>
    shared_ptr<parser_request_i> criar(const string &path, const F func, I* instancia, const Args&... args)
    {
        std::initializer_list<string> inputs({args...});
        vector<string> parametros(inputs);
        auto cvr = make_shared<ConversorRequisicaoParametros<F,I,B, sizeof...(args)>>(path, func, instancia, parametros);
        return cvr;
    }
};

template<>
struct CriadorConversorRequisicao<void>
{
    template<class F, class I, typename... Args>
    shared_ptr<parser_request_i> criar(const string &path, const F func, I* instancia, const Args&... args)
    {
        std::initializer_list<string> inputs({args...});
        vector<string> parametros(inputs);
        auto cvr = make_shared<ConversorRequisicaoParametros<F,I,void*, sizeof...(args)>>(path, func, instancia, parametros);
        return cvr;
    }
};

}

#endif // CONVERSORREQUISICAOPARAMETROS_H
