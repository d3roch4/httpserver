#ifndef PARSER_URL_H
#define PARSER_URL_H

namespace httpserver
{

template<int NumberParameters>
string parser_path_to_params(char *url, const vector<std::string> &params, std::array<char*, NumberParameters> resultr)
{
    if(params.empty())
        return {};

    vector<char*> paramValue;

    char* pch = strtok (url,"/?&=#");
    while (pch != NULL)
    {
        bool found=false;
        for(const string& param: params){
            if(param == pch){
                char* name = pch; // not use
                pch = strtok(NULL, "/?&=#");
                paramValue.emplace_back(pch);
                found=true;
                goto NEXTPARAM;
            }
        }
NEXTPARAM:
        if( ! found )
            return "parser_path_to_params: '"+string(pch)+"' not found";
        pch = strtok (NULL, "/?&=#");
    }

    return "";
}

}

#endif // PARSER_URL_H
