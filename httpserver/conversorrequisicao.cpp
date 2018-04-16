#include "conversorrequisicao.h"

using namespace httpserver;

ConversorRequisicao::ConversorRequisicao(std::function<response(request_empty&)> function)
{
    this->function = function;
}

response ConversorRequisicao::operator()(request_empty &requisicao)
{
    return function(requisicao);
}

