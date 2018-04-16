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

typedef function<bool()> function_filter;

struct parser_request_i
{
    vector<function_filter> filters;
    virtual void operator()(boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser_empty &req) = 0;
};


template<class T>
struct CriadorConversorRequisicao{};

}


#endif // PARSERREQUISICAO_H
