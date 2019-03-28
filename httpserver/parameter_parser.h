#ifndef PARAMETER_PARSER_H
#define PARAMETER_PARSER_H
#include <string>
#include <sstream>

namespace httpserver {

struct parameter_parser : public std::string
{
    parameter_parser() {}

    parameter_parser(const std::string& s) :
        std::string(s){
    }

    void swap(std::string& sh){
        std::string::swap(sh);
    }

    operator std::string& (){
        return *this;
    }

    operator bool (){
        if(size()){
            char c = at(0);
            if(c=='s' || c=='1' || c=='y' ||c=='t')
                return true;
        }
        return false;
    }

    template <typename T>
    operator T () {
        std::stringstream ss{*this};
        T ret{};
        ss >> ret;
        return ret;
    }
};


}
#endif // PARAMETER_PARSER_H
