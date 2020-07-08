#pragma once

//STD
#include <memory>
#include <string>
#include <functional>
#include <map>

//LIBS
#include <fmt/format.h>

#include "object.hpp"

namespace element
{
    enum class error_message_code
    {
        failed_to_find_when_resolving_indexing_expr,
        failed_to_find_when_resolving_identifier_expr,
        recursion_detected,
        instance_function_cannot_be_nullary,
        is_not_an_instance_function,

    };

    template <typename... Args>
    struct error_map
    {
        using func = std::function<std::shared_ptr<error>(const declaration*, Args...)>;
        using map = std::map<error_message_code, func>;

        static inline map func_map;

        static void register_func(error_message_code code, func&& b)
        {
            auto& [it, success] = func_map.insert({ code, std::move(b) });
            if(!success)
                throw;
        }

        static std::shared_ptr<error> build_error(error_message_code code, const declaration* location, Args... args)
        {
            auto it = func_map.find(code);
            if (it == func_map.end())
            {
                //todo: not valid so do something better
                throw;
            }

            return it->second(location, args...);
        }
    };

    //template helper methods
    template <typename... Args>
    void register_error(error_message_code code, std::string format, element_result error_result)
    {
        auto builder = [f = std::move(format), error_result](const declaration* location, Args... args)
        {
            //std::cout << fmt::format(format, args...);
            return std::make_shared<error>(fmt::format(f, args...), error_result, location);
        };

        error_map<Args...>::register_func(code, std::move(builder));
    }

    //build_error is intended to fall back on template argument deduction to avoid the need for silly template parameters everywhere
    template <typename... Args>
    std::shared_ptr<error> build_error(error_message_code code, Args... args)
    {
        return error_map<Args...>::build_error(code, nullptr, args...);
    }

    template <typename... Args>
        std::shared_ptr<error> build_error(error_message_code code, const declaration* location, Args... args)
    {
        return error_map<Args...>::build_error(code, location, args...);
    }

    namespace detail
    {
        //used for initializing the errors in maps statically
        bool register_errors();
    }
}