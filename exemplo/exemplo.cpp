#include <httpserver/httpserver.h>
#include <httpserver/client.h>

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
         ok( "um: "+um+" dois: "+dois+" tres: "+to_string(tres)+" calc: "+to_string(tres*stod(dois)/stoi(um)) );
     }

     void pega(int numero){
         response resp;
         resp.body() = "Peguei: "+to_string(numero)+", body: "+request().body();
         //send( resp );
     }

    CarroControlador(HttpServer& bs){
         bs.route(verb::get, "/carros/(.+)/(.+)", std::bind(&CarroControlador::getCarros, this, _1, _2), "marca", "modelo");
         bs.route(verb::get, "/usuario/(.+)/extrato", std::bind(&CarroControlador::pega, this, _1), "");
         bs.route(verb::get, "/um/([0-9]+)/dois/(.*)/tres/(.*)/fim", std::bind(&CarroControlador::um2tres, this, _1, _2, _3), "marca", "modelo", "");
         bs.route(verb::get, "/pega/([0-9]+)", std::bind(&CarroControlador::pega, this, _1), _1);
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

    cout << "Acesse: http://localhost:3000" << endl;
    cout << "Acesse: https://localhost:3001" << endl;
    thread t([&]{
        server.run();
    });
    
    client http{"https://localhost"};
    clog << "/pega" << endl;
    clog << http.request(verb::post, "/pega", "Testando. Um, dois, três", "");

    clog << "join" << endl;
    t.join();
}
