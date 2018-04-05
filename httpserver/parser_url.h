 #ifndef PARSER_URL_H
#define PARSER_URL_H

#define DIVISOR_URL "/?&=#"

namespace httpserver
{

template<int NumberParameters>
bool parser_path_to_params(char *url, const vector<std::string> &params, std::array<char*, NumberParameters>& resultr)
{
    if(params.empty())
        return true;

    int i=0;
    char* pch = strtok (url, DIVISOR_URL);
    while (pch != NULL)
    {
        for(const string& param: params){
            if(param == pch){
                char* name = pch; // not use
                pch = strtok(NULL, DIVISOR_URL);
                resultr[i] = pch;
                i++;
                goto NEXTPARAM;
            }
        }
NEXTPARAM:
        pch = strtok (NULL, DIVISOR_URL);
    }

    return i==params.size();
}

}

#endif // PARSER_URL_H
