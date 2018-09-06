#ifndef JSON_H
#define JSON_H

#include <mor/entity.h>
#include <sstream>
#include <iostream>
#include <httpserver/response.h>
#include <json/json.h>
#include <json/reader.h>
#include <httpserver/logger.h>
#include <string>
#include <boost/log/trivial.hpp>
#include <httpserver/parser/to_json.h>
#include <httpserver/parser/from_json.h>

namespace httpserver {
    typedef Json::Value JSONObject;
    
    /*
    template <class B>
    B& from_json(B& obj, JSONObject& json) {
        static_assert(std::is_base_of<mor::Entity<B>, B>::value, "B is not extended of Entity<B>");


        mor::Entity<B>* ptr = (mor::Entity<B>*) & obj;
        int i = 0;
        const JSONObject::Members& members = json.getMemberNames();
        for (const JSONObject& value : json) {
            for (int j = 0; j < ptr->_desc_fields.size(); j++) {
                if (ptr->_desc_fields[j].name == members[i]) {
                    switch (value.type()) {
                        case Json::ValueType::intValue: ///< signed integer value
                            ptr->_fields[j]->setValue(std::to_string(value.asInt64()).c_str());
                            break;
                        case Json::ValueType::uintValue: ///< unsigned integer value
                            ptr->_fields[j]->setValue(std::to_string(value.asUInt64()).c_str());
                            break;
                        case Json::ValueType::realValue: ///< double value
                            ptr->_fields[j]->setValue(std::to_string(value.asDouble()).c_str());
                            break;
                        case Json::ValueType::stringValue: ///< UTF-8 string value
                            ptr->_fields[j]->setValue(value.asCString());
                            break;
                        case Json::ValueType::booleanValue: ///< bool value
                            ptr->_fields[j]->setValue(std::to_string(value.asBool()).c_str());
                            break;
                        case Json::ValueType::arrayValue: ///< array value (ordered list)
                        case Json::ValueType::objectValue:
                        case Json::ValueType::nullValue: ///< 'null' value
                        default:
                            //logger("from_json: value '"+field->name+"' with type not reconized!");
                            break;
                    }
                }
            }
            i++;
        }
        return obj;
    }

    template <class B>
    B& from_json(const JSONObject& json) {
        B obj;
        return from_json(std::forward(obj), json);
    }
    
    inline auto& from_json(response& resp, JSONObject& json) {
        resp.body() += json.toStyledString();
        return resp;
    }

    inline auto& from_json(boost::log::basic_formatting_ostream<char>& ss, JSONObject& json) {
        {
            ss << json.toStyledString();
            return ss;
        }
    }

    template<class T>
    T& operator<<(T& obj, JSONObject& json) {
        from_json<T>(obj, json);
        return obj;
    }

    template<class T>
    T& operator<<(T&& obj, JSONObject& json) {
        from_json<T>(obj, json);
        return obj;
    } */

    template<class T>
    JSONObject& operator<<(JSONObject& json, T& obj) {
        to_json(obj, json);
        return json;
    }

    template<class T>
    JSONObject& operator<<(JSONObject&& json, T& obj) {

        to_json(obj, json);
        return json;
    }

    inline JSONObject& operator<<(JSONObject& json, response& resp) {
        const string& str = resp.body();
        to_json(str, json);
        return json;
    }

    inline std::string to_string(JSONObject& json) {
        return json.toStyledString();
    }
}

#endif // JSON_H

