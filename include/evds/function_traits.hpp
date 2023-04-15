#pragma once

#include <functional>

namespace evds::details
{
template <typename RetType, typename... Args>
struct function_traits_helper
{
    using ReturnT = RetType;
    using FunctionT = RetType(Args...);
    using FunctionWrapperT = std::function<FunctionT>;
};

template <typename T>
struct function_traits;

template <typename RetType, typename... Args>
struct function_traits<RetType(Args...)> : function_traits_helper<RetType, Args...>
{
};

template <typename RetType, typename... Args>
struct function_traits<RetType (*)(Args...)> : function_traits_helper<RetType, Args...>
{
};

template <typename RetType, typename... Args>
struct function_traits<RetType (&)(Args...)> : function_traits_helper<RetType, Args...>
{
};

template <typename CType, typename RetType, typename... Args>
struct function_traits<RetType (CType::*)(Args...)> : function_traits_helper<RetType, Args...>
{
};

template <typename CType, typename RetType, typename... Args>
struct function_traits<RetType (CType::*)(Args...) const> : function_traits_helper<RetType, Args...>
{
};

template <typename T>
struct function_traits : function_traits<decltype(&T::operator())>
{
};

}