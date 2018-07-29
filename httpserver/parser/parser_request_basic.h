#ifndef PARSER_REQUEST_BASIC_H
#define PARSER_REQUEST_BASIC_H
#include <httpserver/parser/parser_request_i.h>
#include <httpserver/invoker.h>
#include <regex>


namespace httpserver {
namespace parser {

using namespace std;

template<class F, int NumberParameters>
class parser_request_basic : public parser_request_i
{
    std::regex regex_;
    std::cmatch what_;
    F function_;
public:
    parser_request_basic(const string &path, F function, const vector<string>& parametros) :
        function_{ function }
    {
        regex_.assign(path);
    }

    bool macth(const boost::string_view& path){
        return regex_match(path.to_string().c_str(), what_, regex_);
    }

    void operator()(boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser_empty &request)
    {
//        string url{request.get().target().data(), request.get().target().size()};
        std::array<char*, NumberParameters> array;

        std::vector<string> vec{what_.begin(), what_.end()};
        for(int i=0; i<array.size(); i++){
            array[i] = (char*) vec[i+1].c_str();
        }

        auto tuple = a2t(array);
        invoker(function_, tuple);
    }
};

}
}
#endif // PARSER_REQUEST_BASIC_H
