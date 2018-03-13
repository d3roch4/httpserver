#include "httpserver/httpserver.h"

using namespace httpserver;

class ImovelControlador
{
    response buscar(string cidade, string bairro){
        response resp;
        resp.body() = "Cidade: "+cidade+" bairro: "+bairro;
        return resp;
    }

    response salvar(rapidjson::Document* json){}
    response alterar(string id){}
    response remover(string id){}
public:
    ImovelControlador(HttpServer& bs){
        bs.route(verb::get, "/imoveis", &ImovelControlador::buscar, this, {"cidade", "bairro"});
        bs.route<rapidjson::Document*>(verb::post, "/imoveis", &ImovelControlador::salvar, this);
        bs.route(verb::put, "/imoveis", &ImovelControlador::alterar, this, {"id"});
        bs.route(verb::delete_, "/imoveis", &ImovelControlador::remover, this, {"id"});
    }
};

int main()
{
    httpserver::HttpServer server;
    ImovelControlador control{server};
    server.run();
}
