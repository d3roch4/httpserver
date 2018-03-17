#ifndef CONVERSORREQUISICAOPARAMETROS_H
#define CONVERSORREQUISICAOPARAMETROS_H

#include "iConversorRequisicao.h"
#include <vector>
#include "invoker.h"
#include "parser_body.h"
#include "parser_url.h"

namespace httpserver
{
using namespace std;

template<typename F, typename I, typename B, int NumberParameters>
class ConversorRequisicaoParametros : public iConversorRequisicao
{
    F function;
    I* instance;
    vector<string> parameters;
    string prefix;
    parser_body<B> body;
public:
    ConversorRequisicaoParametros(const string& prefix, F ptr, I* instance, const vector<string>& params) {
        this->parameters = params;
        this->prefix = prefix;
        this->function = ptr;
        this->instance = instance;
    }

    response converter(request &request)
    {
        const string& url = request.target().to_string().substr(prefix.length());
        std::array<char*, NumberParameters> array;
        const string& erro = parser_path_to_params<NumberParameters>((char*)url.c_str(), parameters, array);

        if(erro.size())
            return bad_request(erro);

        auto tuple = a2t(array);
        const auto& tuple2 = body(request, tuple);
        return invoker(function, instance, tuple2);
    }
};


template<class B>
struct CriadorConversorRequisicao<B&>
{
    template<class F, class I, typename... Args>
    shared_ptr<iConversorRequisicao> criar(const string &path, const F func, I* instancia, const Args&... args)
    {
        vector<string> parametros;
        parametros.emplace_back(args...);
        auto cvr = make_shared<ConversorRequisicaoParametros<F,I,B, sizeof...(args)>>(path, func, instancia, parametros);
        return cvr;
    }
};

template<>
struct CriadorConversorRequisicao<void>
{
    template<class F, class I, typename... Args>
    shared_ptr<iConversorRequisicao> criar(const string &path, const F func, I* instancia, const Args&... args)
    {
        vector<string> parametros;
        parametros.emplace_back(args...);
        auto cvr = make_shared<ConversorRequisicaoParametros<F,I,void, sizeof...(args)>>(path, func, instancia, parametros);
        return cvr;
    }
};

}

#endif // CONVERSORREQUISICAOPARAMETROS_H
