#ifndef CONVERSORQUISICAO_H
#define CONVERSORQUISICAO_H

#include "request.h"
#include "response.h"
#include <functional>
#include <memory>

namespace httpserver
{

using namespace std::placeholders;
using namespace std;

struct parser_request_i
{
    virtual void operator()(boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser &req) = 0;
};


template<class T>
struct CriadorConversorRequisicao{};

}


#endif // PARSERREQUISICAO_H
