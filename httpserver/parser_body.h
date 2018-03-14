#ifndef PARSER_BODY_H
#define PARSER_BODY_H

#include "request.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "entity.h"
#include <boost/beast/http/field.hpp>

namespace httpserver {

typedef rapidjson::Document::Object Json;

template <class B>
struct parser_body{    
    static_assert(std::is_base_of<Entity<B>, B>::value, "B is not extended of Entity<B>");
    B operator ()(request& req)
    {
        B obj;
        const string& contentType = req[boost::beast::http::field::content_type];
        if (contentType == "application/json"){

            const rapidjson::Value& json = parser_body<Json>(req);
            Entity<B>* ptr = (Entity<B>*) &obj;
            for(Json::MemberIterator itr=json.MemberBegin(); itr!=json.MemberEnd(); itr++)
            {
                auto field = ptr->_fields.begin();
                while (field != ptr->_fields.end()) {
                    std::unique_ptr<iField>& f = *field;
                    const auto& nameItr = itr->name.GetString();
                    if(f->name == nameItr){
                        rapidjson::StringBuffer buffer;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                        itr->value.Accept(writer);
                        f->setValue(buffer.GetString());
                        field = ptr->_fields.end();
                    }else{
                        field++;
                    }
                }
            }
        }else
            throw runtime_error("parser_body: "+contentType+" not suported.");

        return obj;
    }
};

template <>
struct parser_body<Json>{
    Json operator ()(request& req)
    {
        rapidjson::Document d;
        d.Parse(req.body().c_str());
        return d.GetObject();
    }
};


}

#endif // PARSER_BODY_H
