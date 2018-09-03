#ifndef PARAMETER_PARSER_H
#define PARAMETER_PARSER_H
#include <string>
#include <sstream>

struct parameter_parser
{
    std::string str;

    parameter_parser() {}
    void swap(std::string& sh){
        str.swap(sh);
    }

    operator std::string& (){
        return str;
    }

    template <typename T> operator T () {
        std::stringstream ss{str};
        T ret;
        ss >> ret;
        return ret;
    }
};

#endif // PARAMETER_PARSER_H
