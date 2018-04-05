#ifndef JSON_H
#define JSON_H

#include <mor/entity.h>
#include <sstream>
#include <iostream>
#include "response.h"
#include <json/json.h>
#include <json/reader.h>
#include "logger.h"
#include <string>
#include <boost/log/trivial.hpp>

namespace httpserver
{

typedef Json::Value JSONObject;

template <class B>
struct from_json{
    static_assert(std::is_base_of<mor::Entity<B>, B>::value, "B is not extended of Entity<B>");

    B& operator()(B& obj, JSONObject& json){
        mor::Entity<B>* ptr = (mor::Entity<B>*) &obj;
        for(std::unique_ptr<mor::iField>& field: ptr->_fields){
            const JSONObject& value = json.get(field->name, JSONObject::null);
            switch (value.type()) {
            case Json::ValueType::intValue:      ///< signed integer value
                field->setValue(std::to_string(value.asInt64()).c_str());
                break;
            case Json::ValueType::uintValue:     ///< unsigned integer value
                field->setValue(std::to_string(value.asUInt64()).c_str());
                break;
            case Json::ValueType::realValue:     ///< double value
                field->setValue(std::to_string(value.asDouble()).c_str());
                break;
            case Json::ValueType::stringValue:   ///< UTF-8 string value
                field->setValue(value.asCString());
                break;
            case Json::ValueType::booleanValue:  ///< bool value
                field->setValue(std::to_string(value.asBool()).c_str());
                break;
            case Json::ValueType::arrayValue:    ///< array value (ordered list)
            case Json::ValueType::objectValue:
            case Json::ValueType::nullValue: ///< 'null' value
            default:
                //logger("from_json: value '"+field->name+"' with type not reconized!");
                break;
            }
        }
        return obj;
    }
};

template <>
struct from_json<response>
{
    auto& operator ()(response& resp, JSONObject& json)
    {
        resp.body() += json.toStyledString();
        return resp;
    }
};

template <>
struct from_json<boost::log::basic_formatting_ostream<char>>
{
    auto& operator ()(boost::log::basic_formatting_ostream<char>& ss, JSONObject& json)
    {
        ss << json.toStyledString();
        return ss;
    }
};

template<class T>
T& operator << (T& obj, JSONObject& json)
{
    from_json<T> f; f(obj, json);
    return obj;
}


template<class T>
T& operator << (T&& obj, JSONObject& json)
{
    from_json<T> f; f(obj, json);
    return obj;
}


    inline void to_json(const char* str, JSONObject& json)
    {
        Json::Reader read;
        read.parse(str, json);
    }

    inline JSONObject to_json(const char* str)
    {
        JSONObject json;
        to_json(str, json);
        return json;
    }

    template<class type_>
    void to_json(type_& obj, JSONObject& json)
    {
        typedef typename std::remove_const<type_>::type type;
        static_assert(std::is_base_of<mor::Entity<type>, type>::value, "type is not a mor::Entity<type> extended");

        const mor::Entity<type>* entity = (mor::Entity<type>*) &obj;
        for(auto& fi: entity->_fields){
            if (*fi->typeinfo.get() == typeid(unsigned short) ||
                *fi->typeinfo.get() == typeid(unsigned int) ||
                *fi->typeinfo.get() == typeid(unsigned long) )
                json[fi->name] = *(Json::UInt64*) fi->value;
            else if( *fi->typeinfo.get() == typeid(short) ||
                     *fi->typeinfo.get() == typeid(int) ||
                     *fi->typeinfo.get() == typeid(long) )
                json[fi->name] = *(Json::Int64*) fi->value;
            else if( *fi->typeinfo.get() == typeid(float) ||
                     *fi->typeinfo.get() == typeid(double) )
                json[fi->name] = *(double*) fi->value;
            else if( *fi->typeinfo.get() == typeid(bool) )
                json[fi->name] = *(bool*) fi->value;
            else
                json[fi->name] = fi->getValue();
        }
    }
    template<class type>
    JSONObject to_json(type& obj)
    {
        JSONObject json;
        to_json(obj, json);
        return json;
    }

    template<class T>
    JSONObject& operator << (JSONObject& json, T& obj)
    {
        to_json(obj, json);
        return json;
    }
    template<class T>
    JSONObject& operator << (JSONObject&& json, T& obj)
    {

        to_json(obj, json);
        return json;
    }    

    inline JSONObject& operator << (JSONObject& json, response& resp)
    {
        Json::Reader parser;
        parser.parse(resp.body(), json);
        return json;
    }

    inline std::string to_string(JSONObject& json)
    {
        return json.toStyledString();
    }
}

#endif // JSON_H

