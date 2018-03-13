#include "Roteador.h"
#include <iostream>
#include <boost/beast.hpp>

using namespace httpserver;

Roteador::Roteador() {}

void Roteador::rota(verb method, const string &path, shared_ptr<iConversorRequisicao> tratador)
{
  mRotas[(int)method][path] = tratador;
}

// Returns a not found response
auto const not_found = [](boost::beast::string_view target)
{
    boost::beast::http::response<boost::beast::http::string_body> res;
    res.result(boost::beast::http::status::not_found);
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
//    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + target.to_string() + "' was not found.";
    res.prepare_payload();
    return res;
};

response Roteador::despachar(request &req)
{
    for(auto& method: mRotas){
        if(method.first == (int)req.method()){
            size_t end = req.target().find('?');
            if (end == string::npos)
                end = req.target().find('#');
            auto path = req.target().substr(0, end);
            for(auto& mapPath: method.second){
                if(path.find(mapPath.first) != string::npos){
                    return mapPath.second->converter(req);
                }
            }
        }
    }
    return not_found(req.target());

/*
    auto method = mRotas.find((int)req.method());
    if(method != mRotas.end()){
        auto mapPath = method->second;

        size_t end = req.target().find('?');
        if (end == string::npos)
            end = req.target().find('#');

        GlobalRequisicao::instancia()->requisicao(req);
        auto path = req.target().substr(0, end);
        std::cout << "Requisicao: " << path << std::endl;

        auto itr = mapPath.find(path.to_string());
        if (itr != mapPath.end()) {
            shared_ptr<iConversorRequisicao> &conversor = itr->second;
            return conversor->converter(req);
        } else{
            size_t bar = path.substr(1).find('/');
            auto itr = mapPath.find(path.substr(0, (bar==string::npos)?bar:bar+1).to_string()+'*');
            if (itr != mapPath.end()) {
                shared_ptr<iConversorRequisicao> &conversor = itr->second;
                return conversor->converter(req);
            } else
                return RespostaFabrica::notFound("Arquivo nao encontrado", "text/plain");
        }
    }else
        return RespostaFabrica::notFound("Arquivo nao encontrado", "text/plain");
        */
}

void Roteador::set_public_dir(const string &dir)
{
    public_dir = dir;
}

