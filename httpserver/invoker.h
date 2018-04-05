#ifndef INVOKER_H
#define INVOKER_H

#include <tuple>
#include <iostream>
#include <array>
#include <utility>
#include "response.h"
#include <functional>

template<typename F, typename I, typename Tuple, size_t ...S >
decltype(auto) apply_tuple_impl(F&& fn, I* instance, Tuple&& t, std::index_sequence<S...>)
{    

//    auto greet = std::mem_fn(fn);
//    greet(*instance, std::get<S>(t)...);
//    return (instance->*fn)(std::get<S>(t) ...);
//    return (instance->*fn)(std::move(std::get<S>(t))...);
    return std::bind(fn, instance, std::get<S>(t)...)();
}

template<typename F, typename I, typename Tuple>
decltype(auto) invoker(F&& fn, I* instance, Tuple&& t)
{
  std::size_t constexpr tSize = std::tuple_size<typename std::remove_reference<Tuple>::type>::value;
  return apply_tuple_impl(std::forward<F>(fn), instance, t, std::make_index_sequence<tSize>());
}


// Convert array into a tuple
template<typename Array, std::size_t... I>
decltype(auto) a2t_impl(const Array& a, std::index_sequence<I...>)
{
    return std::make_tuple(a[I]...);
}

template<typename T, std::size_t N, typename Indices = std::make_index_sequence<N>>
decltype(auto) a2t(const std::array<T, N>& a)
{
    return a2t_impl(a, Indices{});
}


/** Descobre quantos parametros uma função tem **/
template <typename T>
struct get_arity : get_arity<decltype(&T::operator())> {};
template <typename R, typename... Args>
struct get_arity<R(*)(Args...)> : std::integral_constant<unsigned, sizeof...(Args)> {};
template <typename R, typename C, typename... Args>
struct get_arity<R(C::*)(Args...)> : std::integral_constant<unsigned, sizeof...(Args)> {};
template <typename R, typename C, typename... Args>
struct get_arity<R(C::*)(Args...) const> : std::integral_constant<unsigned, sizeof...(Args)> {};
// All combinations of variadic/non-variadic, cv-qualifiers and ref-qualifiers


#endif // INVOKER_H
