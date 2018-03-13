#include "conversorrequisicao.h"

using namespace httpserver;

ConversorRequisicao::ConversorRequisicao(std::function<response(request&)> function)
{
    this->function = function;
}

response ConversorRequisicao::converter(request &requisicao)
{
    return function(requisicao);
}

