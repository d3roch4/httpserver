#include <httpserver/httpserver.h>
#include <mor/entity.h>

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
    void buscar(const char* marca, const char* modelo){
        response resp;
        resp.body() = string("Marca: ")+marca+" Modelo: "+modelo;
        send( resp );
    }

    void um2tres(string um, string dois, string tres){
        response resp;
        resp.body() = "um: "+um+" dois: "+dois+" tres: "+tres;
        send( resp );
    }
    response alterar(httpserver::file& arquivo, char* id){
//        arquivo.write("/tmp/teste.txt");

        return response{};
    }
    response remover(string id){}
    CarroControlador(HttpServer& bs){
        bs.route(verb::get, "/carros/(.*)/(.*)", std::bind(&CarroControlador::buscar, this, _1, _2), "marca", "modelo");
        bs.route(verb::get, "/um\\/(.*)/dois/(.*)/tres/(.*)/fim", std::bind(&CarroControlador::um2tres, this, _1, _2, _3), "marca", "modelo", "");
//        bs.route(verb::post, "/imoveis", &CarroControlador::salvar, this);
//        bs.route(verb::put, "/imoveis", &CarroControlador::alterar, this, "id");
//        bs.route(verb::delete_, "/imoveis", &CarroControlador::remover, this, "id");
    }
};

template< class F >
void call(const F& fun){
    fun("adasd", "4534");
}

int main()
{
    httpserver::HttpServer server;
    CarroControlador carro{server};

//    call(std::bind(&CarroControlador::buscar, &carro, _1, _2));

    server.run();
}
