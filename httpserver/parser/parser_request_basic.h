#ifndef PARSER_REQUEST_BASIC_H
#define PARSER_REQUEST_BASIC_H
#include <httpserver/parser/parser_request_i.h>
#include <httpserver/invoker.h>
#include <regex>
#include "parameter_parser.h"

namespace httpserver {
namespace parser {

using namespace std;

template<class F, int NumberParameters>
class parser_request_basic : public parser_request_i
{
    const std::regex regex_;
    std::smatch what_;
    F function_;
public:
    parser_request_basic(const string &exp, F function, const vector<string>& parametros) :
        function_{ function },
        regex_{ exp }
    {
    }

    bool macth(const std::string& path){
        return std::regex_match(path, what_, regex_);
    }

    void operator()(boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser_empty &request)
    {
        std::array<parameter_parser, NumberParameters> array;

        for(size_t i=0; i<array.size(); i++){
            string&& str = what_[i+1].str();
            array[i].swap(str);
        }

        auto tuple = a2t(array);
        invoker(function_, tuple);
    }
};

}
}
#endif // PARSER_REQUEST_BASIC_H
