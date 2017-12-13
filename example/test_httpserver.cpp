#include "httpserver.h"
using namespace httpserver;

struct User : Entity<User>
{
    string name;
    string login;
    string password;

    User(){
        column(name, "name");
        column(login, "login");
        column(password, "password");
    }
};

class LoginControler : Controller
{
    response login(User& u){
        return ok("User: "+u.login);
    }
    response name(User& u){
        return ok("User: "+u.name);
    }
    response erro(request req){
        return bad_request("erro: "+req.target().to_string());
    }

public:
    LoginControler(HttpServer& server){

        server.addRoute<User>("/name", BIND(LoginControler::name));
        server.addRoute<User>("/login", BIND(LoginControler::login), POST);
        server.addRoute("/error", BIND(LoginControler::erro));
    }
};


int main()
{
    HttpServer server;
    LoginControler login{server};

    server.run();
    return 0;
}
