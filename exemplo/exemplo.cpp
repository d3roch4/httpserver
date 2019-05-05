#include <httpserver/httpserver.h>

using namespace httpserver;

struct Carro
{
    string marca;
    string modelo;
    int ano;
};

class CarroControlador
{
public:
    void getCarros(string marca, string modelo){
        int param = request().query("p");
        cout << request().body();
        ok( "Marca: "+marca+" Modelo: "+modelo+", Param: "+to_string(param) );
    }

    void um2tres(string um, string dois, int tres){
        response resp;
        ok( "um: "+um+" dois: "+dois+" tres: "+to_string(tres) );
    }

    void pega(int numero){
        response resp;
        resp.body() = "Peguei: "+to_string(numero);
        send( resp );
    }

    response remover(string id){}
    CarroControlador(HttpServer& bs){
        bs.route(verb::get, "/carros/(.+)/(.+)", std::bind(&CarroControlador::getCarros, this, _1, _2), "marca", "modelo");
        bs.route(verb::get, "/usuario/(.+)/extrato", std::bind(&CarroControlador::pega, this, _1), "");
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

    cout << "Acesse: https://localhost:3000" << endl;
    server.run();
}
