#include "request.h"
#include "session_plain.h"
#include <regex>

namespace httpserver {

std::string urlDecode(std::string &eString) {
    std::string ret;
    char ch;
    int i, j;
    for (i=0; i<eString.length(); i++) {
        if (int(eString[i])==37) {
            sscanf(eString.substr(i+1,2).c_str(), "%x", &j);
            ch=static_cast<char>(j);
            ret+=ch;
            i=i+2;
        } else {
            ret+=eString[i];
        }
    }
    return (ret);
}

void dynamic_request::parse_query()
{
    std::string url = this->target().to_string();

    std::regex pattern("([\\w+%]+)=([^&#]*)");
    auto words_begin = std::sregex_iterator(url.begin(), url.end(), pattern);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; i++)
    {
        std::string key = (*i)[1].str();
        std::string value = (*i)[2].str();
        query_[key] = urlDecode( value );
    }
}

dynamic_request::dynamic_request()  {
    this->request_parser_ = nullptr;
}

dynamic_request::dynamic_request(request_parser_empty* req_empty, session* session) :
    request_empty(req_empty->get()) {
    this->request_parser_ = req_empty;
    session_ = session;
}

parameter_parser dynamic_request::query(const std::string &key)
{
    if(query_.empty())
        parse_query();
    auto itr = query_.find(key);
    if(itr != query_.end())
        return itr->second;
    else
        return {};
}

std::unordered_map<std::string, parameter_parser> dynamic_request::query()
{
    return query_;
}

dynamic_request& request(){
    session& ss = get_http_session();
    return ss.request();
}


}
