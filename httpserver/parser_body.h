#ifndef PARSER_BODY_H
#define PARSER_BODY_H

#include "request.h"
#include "file.h"
#include "json.h"
#include <mor/entity.h>
#include <boost/beast/http/field.hpp>

namespace httpserver {

template <class B>
struct parser_body
{
    static_assert(std::is_base_of<mor::Entity<B>, B>::value, "B is not extended of Entity<B>");

    template <typename... TupleElem>
    std::tuple<B, TupleElem...> operator ()(B& obj, boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser_empty& req, const std::tuple<TupleElem...> &tuple)
    {
        const auto& contentType = req.get()[field::content_type];

        if (contentType.find("application/json") != boost::string_view::npos)
        {
            boost::beast::http::request_parser<boost::beast::http::string_body> req_b{std::move(req)};
            boost::beast::http::read(socket, buffer, req_b); // Finish reading the message
            JSONObject json;
            Json::Reader read;
            if(! read.parse(req_b.release().body(), json) )
                throw std::runtime_error("parser_body: body content is not a valid json.");

            obj << json;
        }else
            throw std::runtime_error("parser_body: Content-Type: "+contentType.to_string()+" is not suported.");

        auto&& t = std::tie(obj);
        return std::tuple_cat(std::move(t), std::move(tuple));
    }
};

template <>
struct parser_body<JSONObject>
{
    template <typename... TupleElem>
    auto operator ()(JSONObject& json, boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser_empty& req, const std::tuple<TupleElem...> &tuple)
    {
        boost::beast::http::request_parser<boost::beast::http::string_body> req_b{std::move(req)};
        // Finish reading the message
        boost::beast::http::read(socket, buffer, req_b);

        to_json(req_b.release().body().c_str(), json);
        return std::tuple_cat(std::tie(json), tuple);
    }
};

template <>
struct parser_body<file>
{
    template <typename... TupleElem>
    auto operator ()(file& f, boost::asio::ip::tcp::socket &socket, boost::beast::flat_buffer &buffer, request_parser_empty& req, const std::tuple<TupleElem...> &tuple)
    {
        f = {socket, buffer, req};

        auto&& tupleBody = std::tie(f);
        return std::tuple_cat(std::move(tupleBody), tuple);
    }
};

template <>
struct parser_body<void*>
{
    template <typename T>
    T operator ()(void*, boost::asio::ip::tcp::socket& socket, boost::beast::flat_buffer& buffer, request_parser_empty& req, const T &tup)
    {
        return tup;
    }
};

}

#endif // PARSER_BODY_H
