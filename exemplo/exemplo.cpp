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
    void buscar(string marca, string modelo){
        response resp;
        resp.body() = "Marca: "+marca+" Modelo: "+modelo;
        send( resp );
    }

    void um2tres(string um, string dois, int tres){
        response resp;
        resp.body() = "um: "+um+" dois: "+dois+" tres: "+to_string(tres);
        send( resp );
    }
    response remover(string id){}
    CarroControlador(HttpServer& bs){
        bs.route(verb::get, "/carros/(.*)/(.*)", std::bind(&CarroControlador::buscar, this, _1, _2), "marca", "modelo");
        bs.route(verb::get, "/um/([0-9]+)/dois/(.*)/tres/(.*)/fim", std::bind(&CarroControlador::um2tres, this, _1, _2, _3), "marca", "modelo", "");
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

    server.run();
}
