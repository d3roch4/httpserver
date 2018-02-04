# httpserver
Uma biblioteca para criação de servidores web embutidos escrita em C++ com o uso da boost::beast.

## Pre-requisitos:
- boost
- cmake

Faça a instalação dos intens acima: `sudo apt-get install libboost-all-dev cmake`

## Criando um novo projeto:
Crie um novo projeto e adicione o `httpserver` como um submodulo do seu projeto, depois basta lincar a biblioteca no seu CMakeLists.txt.
- Novo Projeto:
```
  mkdir -p MyProject/build
  cd MyProject
  git init
```
- Adicione o submodulo do `httpserver`:
```
  git submodule add https://github.com/d3roch4/httpserver deps/httpserver
  git submodule update --init --recursive
  git pull --recurse-submodules
  git submodule update --remote --recursive
```
- Crie o arquivo `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.0)
project(MyProject)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_subdirectory(deps/httpserver)

file(GLOB CPP_FILES src/*.cpp)
add_executable(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} httpserver)
```

- Crie o arquivo `src/main.cpp`:
```c++
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
    response user(User& u){
        return ok("User: "+u.name+" login: "+u.login);
    }
    response erro(request req){
        return bad_request("erro: "+req.target().to_string());
    }

public:
    LoginControler(HttpServer& server){

        server.addRoute<User>("/user", BIND(LoginControler::user));
        server.addRoute<User>("/login", BIND(LoginControler::login), POST);
        server.addRoute("/error", BIND(LoginControler::erro));
    }
};


int main()
{
    HttpServer server;
    LoginControler login{server};
    
    clog << "Access: http://localhost:3000/user?name=Fulano&login=email@e.com" << endl;
    server.run();
    return 0;
}

```

- Agora vamos compilar e testar:
```
cd build
cmake ..
make
./MyProject
```
