#include "conversorrequisicaoparametros.h"

using namespace std;

namespace httpserver{

vector<pair<char*,char*>> parser_path_to_params(char *url, const vector<std::string> &params)
{
    if(params.empty())
        return {};

    vector<pair<char*,char*>> paramValue;

    char* pch = strtok (url,"/?&=#");
    while (pch != NULL)
    {
        bool found=false;
        for(const string& param: params){
            if(param == pch){
                char* name = pch;
                pch = strtok(NULL, "/?&=#");
                paramValue.emplace_back(name, pch);
                found=true;
                goto NEXTPARAM;
            }
        }
NEXTPARAM:
        if( ! found )
            throw runtime_error("parser_request_url: '"+string(pch)+"' not found");
        pch = strtok (NULL, "/?&=#");
    }

    return paramValue;
}

}
