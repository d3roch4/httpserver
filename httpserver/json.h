#ifndef JSON_H
#define JSON_H

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/reader.h>
#include <mor/entity.h>
#include <sstream>
#include "response.h"

namespace httpserver
{
    typedef rapidjson::Document::Object Json;

    template<class T>
    rapidjson::Value& operator << (rapidjson::Value& json, T& obj)
    {
        mor::Entity<T>* ptr = &obj;
        for(std::unique_ptr<mor::iField>& field: ptr->_fields){
            json[field->name.c_str()] = field->getValue();
        }
        return json;
    }
}

#endif // JSON_H

