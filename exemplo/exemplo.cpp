#include "httpserver/httpserver.h"

using namespace httpserver;
using namespace mor;

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
    response alterar(file& arquivo, char* id){
        arquivo.write("/tmp/teste.txt");

        return response{};
    }
    response remover(string id){}
    CarroControlador(HttpServer& bs){
        bs.route(verb::get, "/imoveis", &CarroControlador::buscar, this, "marca", "modelo");
        bs.route<Carro&>(verb::post, "/imoveis", &CarroControlador::salvar, this);
        bs.route<file&>(verb::put, "/imoveis", &CarroControlador::alterar, this, "id");
        bs.route(verb::delete_, "/imoveis", &CarroControlador::remover, this, "id");
    }
};

int main()
{
    httpserver::HttpServer server;
    CarroControlador carro{server};
    server.run();
}
