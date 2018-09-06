#ifndef TO_JSON_H
#define TO_JSON_H

#include <mor/entity.h>
#include <sstream>
#include <iostream>
#include <httpserver/response.h>
#include <json/json.h>
#include <json/reader.h>
#include <httpserver/logger.h>
#include <string>
#include <boost/log/trivial.hpp>

namespace httpserver {

using namespace mor;

inline void to_json(const std::string& strJson, Json::Value& json)
{
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader{builder.newCharReader()};

    std::string errors;

    bool parsingSuccessful = reader->parse(
        strJson.c_str(),
        strJson.c_str() + strJson.size(),
        &json,
        &errors
    );

    if (!parsingSuccessful) {
        throw_with_trace(runtime_error("Failed to parse the JSON, errors:"));
    }
}

inline Json::Value to_json(const std::string str)
{
    Json::Value json;
    to_json(str, json);
    return json;
}

void setJson(Json::Value& json, const vector<mor::DescField>& descs, const vector<shared_ptr<iField>>& fields);

template<class type_>
void to_json(type_& obj, Json::Value& json)
{
    typedef typename std::remove_const<type_>::type type;
    static_assert(std::is_base_of<mor::Entity<type>, type>::value, "type is not a mor::Entity<type> extended");

    const mor::Entity<type>* entity = (mor::Entity<type>*) &obj;
    setJson(json, entity->_desc_fields, entity->_fields);
}
template<class type>
Json::Value to_json(const type& obj)
{
    Json::Value json;
    to_json(obj, json);
    return json;
}

}

#endif // TO_JSON_H

