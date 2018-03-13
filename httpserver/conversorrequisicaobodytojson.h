#ifndef CONVERSORREQUISICAOBODYTOJSON_H
#define CONVERSORREQUISICAOBODYTOJSON_H

#include "conversorrequisicaoparametros.h"
#include "httpserver/httpserver.h"
#include "rapidjson/document.h"

namespace httpserver
{

template<typename F, typename I>
class ConversorRequisicaoBodyToJsonUrlToParams : public iConversorRequisicao
{
    F function;
    I* instance;
    vector<string> parameters;
    string prefix;
public:
    ConversorRequisicaoBodyToJsonUrlToParams(const string& prefix, F ptr, I* instance, const vector<string>& params) {
        this->parameters = params;
        this->prefix = prefix;
        this->function = ptr;
        this->instance = instance;
    }

    response converter(request& request)
    {
        const string& url = request.target().to_string().substr(prefix.length());
        vector<pair<char*,char*>>&& vec = parser_path_to_params((char*)url.c_str(), parameters);

        if(vec.size()!=parameters.size()){
            throw runtime_error("ConversorRequisicaoBodyToJsonUrlToParams::converter:> parameters in url are invalids.");
        }
        rapidjson::Document json;
        json.Parse(request.body().c_str());

        void(*func)() = reinterpret_cast<void(*)()>(function);

        switch (parameters.size()) {
        case 0:
            return std::bind((response(*)(I*,rapidjson::Document*))func, instance, &json)();
        case 1:
            return std::bind((response(*)(I*,rapidjson::Document*,string))func, instance, &json, vec[0].second)();
        case 2:
            return std::bind((response(*)(I*,rapidjson::Document*,string,string))func, instance, &json, vec[0].second, vec[1].second)();
        case 3:
            return std::bind((response(*)(I*,rapidjson::Document*,string,string,string))func, instance, &json, vec[0].second, vec[1].second, vec[2].second)();
        case 4:
            return std::bind((response(*)(I*,rapidjson::Document*,string,string,string,string))func, instance, &json, vec[0].second, vec[1].second, vec[2].second, vec[3].second)();
        case 5:
            return std::bind((response(*)(I*,rapidjson::Document*,string,string,string,string,string))func, instance, &json, vec[0].second, vec[1].second, vec[2].second, vec[3].second, vec[4].second)();
        case 6:
            return std::bind((response(*)(I*,rapidjson::Document*,string,string,string,string,string,string))func, instance, &json, vec[0].second, vec[1].second, vec[2].second, vec[3].second, vec[4].second, vec[5].second)();
        case 7:
            return std::bind((response(*)(I*,rapidjson::Document*,string,string,string,string,string,string,string))func, instance, &json, vec[0].second, vec[1].second, vec[2].second, vec[3].second, vec[4].second, vec[5].second, vec[6].second)();
        case 8:
            return std::bind((response(*)(I*,rapidjson::Document*,string,string,string,string,string,string,string,string))func, instance, &json, vec[0].second, vec[1].second, vec[2].second, vec[3].second, vec[4].second, vec[5].second, vec[6].second, vec[7].second)();
        case 9:
            return std::bind((response(*)(I*,rapidjson::Document*,string,string,string,string,string,string,string,string,string))func, instance, &json, vec[0].second, vec[1].second, vec[2].second, vec[3].second, vec[4].second, vec[5].second, vec[6].second, vec[7].second, vec[8].second)();
        case 10:
            return std::bind((response(*)(I*,rapidjson::Document*,string,string,string,string,string,string,string,string,string,string))func, instance, &json, vec[0].second, vec[1].second, vec[2].second, vec[3].second, vec[4].second, vec[5].second, vec[6].second, vec[7].second, vec[8].second, vec[9].second)();
        default:
            throw runtime_error("ConversorRequisicaoBodyToJsonUrlToParams::converter:> parameters in url are invalids.");
            break;
        }
    }
};

template<>
struct FabricaConversorRequisicao<rapidjson::Document*>{
    template<class F, class I>
    shared_ptr<iConversorRequisicao> criar(const string &path, const F func, I* instancia, const vector<string>& parametros={})
    {
        auto cvr = make_shared<ConversorRequisicaoBodyToJsonUrlToParams<F,I>>(path, func, instancia, parametros);
        return cvr;
    }
};

} //namespace

#endif // CONVERSORREQUISICAOBODYTOJSON_H
