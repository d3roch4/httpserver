#ifndef PARSER_BODY_H
#define PARSER_BODY_H

#include "request.h"
#include "json.h"
#include <mor/entity.h>
#include <boost/beast/http/field.hpp>

namespace httpserver {

typedef rapidjson::Document::Object Json;

template <class B>
struct parser_body
{
    static_assert(std::is_base_of<mor::Entity<B>, B>::value, "B is not extended of Entity<B>");

    template <typename... TupleElem>
    std::tuple<B, TupleElem...> operator ()(request& req, const std::tuple<TupleElem...> &tuple)
    {
        B obj;
        const auto& contentType = req[boost::beast::http::field::content_type];
        if (contentType == "application/json"){

            rapidjson::Document d; d.Parse(req.body().c_str());
            const Json& json = d.GetObject();
            mor::Entity<B>* ptr = (mor::Entity<B>*) &obj;
            for(Json::MemberIterator itr=json.MemberBegin(); itr!=json.MemberEnd(); itr++)
            {
                auto field = ptr->_fields.begin();
                while (field != ptr->_fields.end()) {
                    std::unique_ptr<mor::iField>& f = *field;
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
            throw runtime_error("parser_body: "+contentType.to_string()+" not suported.");

        return std::tuple_cat(std::make_tuple(obj), tuple);
    }
};

template <>
struct parser_body<Json>
{
    template <typename... TupleElem>
    std::tuple<Json, TupleElem...> operator ()(request& req, const std::tuple<TupleElem...> &tuple)
    {
        rapidjson::Document d;
        d.Parse(req.body().c_str());

        return std::tuple_cat(std::make_tuple(d.GetObject()), tuple);
    }
};

template <>
struct parser_body<void>
{
    template <typename T>
    T operator ()(request& req, const T &tup)
    {
        return tup;
    }
};

}

#endif // PARSER_BODY_H
