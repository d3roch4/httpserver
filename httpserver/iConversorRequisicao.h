#ifndef CONVERSORQUISICAO_H
#define CONVERSORQUISICAO_H

#include "request.h"
#include "response.h"
#include <functional>
#include <memory>

namespace httpserver
{

using namespace std::placeholders;
using namespace std;

struct iConversorRequisicao
{
    virtual response converter(request& requisicao) = 0;
};


template<class T>
struct FabricaConversorRequisicao{};

}
#endif // PARSERREQUISICAO_H
