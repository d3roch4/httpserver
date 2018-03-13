#ifndef ROTEADOR_H
#define ROTEADOR_H

#include <fstream>
#include <map>
#include <string>
#include "iConversorRequisicao.h"

namespace httpserver {
using namespace std;

typedef boost::beast::http::verb verb;

class Roteador {
    string public_dir;
public:
    Roteador();

    void rota(verb method, const string &path, shared_ptr<iConversorRequisicao> tratador);

    response despachar(request &req);

    void set_public_dir(const string& dir);

private:
    unordered_map<int, unordered_map<string, shared_ptr<iConversorRequisicao>>> mRotas;
};

} // namespace httpserver

#endif // ROTEADOR_H
