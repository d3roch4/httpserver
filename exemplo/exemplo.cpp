#include "httpserver/httpserver.h"
#include "httpserver/conversorrequisicaobody.h"

using namespace httpserver;

struct Carro : Entity<Carro>
{
    string marca;
    string modelo;
    int ano;
};

class CarroControlador
{
public:
    response buscar(string marca, string modelo){
        response resp;
        resp.body() = "Cidade: "+marca+" bairro: "+modelo;
        return resp;
    }

    response salvar(Carro& carro){}
    response alterar(string id){}
    response remover(string id){}
    CarroControlador(HttpServer& bs){
        bs.route(verb::get, "/imoveis", &CarroControlador::buscar, this, {"marca", "modelo"});
        bs.route<Carro&>(verb::post, "/imoveis", &CarroControlador::salvar, this);
        bs.route(verb::put, "/imoveis", &CarroControlador::alterar, this, {"id"});
        bs.route(verb::delete_, "/imoveis", &CarroControlador::remover, this, {"id"});
    }
};

int main()
{
    httpserver::HttpServer server;
    CarroControlador carro{server};
//    auto func = make_generic_function_ptr(&CarroControlador::buscar, carro);
    server.run();
}
