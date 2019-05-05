#ifndef WRAP_HANDLE_REQUEST_BASIC_H
#define WRAP_HANDLE_REQUEST_BASIC_H
#include <httpserver/wrap_handle_request_i.h>
#include <httpserver/invoker.h>
#include <httpserver/parameter_parser.h>
#include <regex>

namespace httpserver {

using namespace std;

template<class F, int NumberParameters>
class wrap_handle_request_basic : public wrap_handle_request_i
{
    std::regex regex_;
    std::smatch what_;
    F function_;
public:
    wrap_handle_request_basic(const string &exp, F function) :
        function_{ function },
        regex_{ exp }
    {
    }

    parser_request_basic(const parser_request_basic<F, NumberParameters>& other) = default;

    bool macth(const std::string& path){
        return std::regex_search(path, what_, regex_);
    }

    void operator()()
    {
        std::array<parameter_parser, NumberParameters> array;

        for(size_t i=0; i<array.size(); i++){
            string&& str = what_[i+1].str();
            array[i].swap(str);
        }

        auto tuple = a2t(array);
        invoker(function_, tuple);
    }

    shared_ptr<parser::parser_request_i> copy()
    {
        auto p = new parser_request_basic<F, NumberParameters>(*this);
        return shared_ptr<parser::parser_request_i>(p);
    }
};

}
#endif // PARSER_REQUEST_BASIC_H
