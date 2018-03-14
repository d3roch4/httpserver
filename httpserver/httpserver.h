#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <iostream>
#include "Roteador.h"
#include "conversorrequisicao.h"
#include "conversorrequisicaoparametros.h"

namespace httpserver
{

using namespace std;

class HttpServer
{
    Roteador router_;
public:
    template<class P=void, typename F, typename T>
    void route(verb method, const string &path, const F func, T* instancia, const vector<string>& parametros={})
    {
        FabricaConversorRequisicao<P> fabrica;
        auto cvr = fabrica.criar(path, func, instancia, parametros);
        router_.rota(method, path, cvr);
    }

    void run(const string& address = "0.0.0.0", unsigned short port = 3000, const string& public_dir = "public_dir", int thread_qtd=4 );
};



}// namesapce

#endif // HTTPSERVER_H
