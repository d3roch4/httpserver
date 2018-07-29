#ifndef FROM_JSON_H
#define FROM_JSON_H

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

    response& operator << (response&& resp, const Json::Value& json);

    void from_json(std::vector<mor::DescField>& vecDescs, std::vector<std::shared_ptr<mor::iField>>& vecFields, const Json::Value& json);

    template<class B>
    B& from_json(B& obj, const Json::Value& json)
    {
        static_assert(std::is_base_of<mor::Entity<B>, B>::value, "B is not extended of Entity<B>");

        mor::Entity<B>* ptr = (mor::Entity<B>*) & obj;
        from_json(ptr->_desc_fields, ptr->_fields, json);
        return obj;
    }
    
    template<class T>
    T from_json(const Json::Value& json)
    {
        T obj;
        return from_json(obj, json);
    }
}

#endif // FROM_JSON_H

