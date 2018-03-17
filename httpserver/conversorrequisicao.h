#ifndef CONVERSORREQUISICAO_H
#define CONVERSORREQUISICAO_H

#include "iConversorRequisicao.h"

namespace httpserver
{

class ConversorRequisicao : public iConversorRequisicao
{
    std::function<response(request&)> function;
public:
    ConversorRequisicao(std::function<response(request&)> function);
    response converter(request& requisicao);
};


template<>
struct CriadorConversorRequisicao<request&>{
    template<class F, class I>
    shared_ptr<iConversorRequisicao> criar(const string &path, const F func, I* instancia, const vector<string>& parametros={}){
        auto cvr = make_shared<ConversorRequisicao>(std::bind(func, instancia, _1));
        return cvr;
    }
};

}
#endif // CONVERSORREQUISICAO_H
