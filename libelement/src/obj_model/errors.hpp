#pragma once

//STD
#include <memory>
#include <string>
#include <functional>
#include <map>

//LIBS
#include <fmt/format.h>

//SELF
#include "object.hpp"

namespace element
{
    enum class error_message_code
    {
        not_indexable,
        not_callable,
        not_compilable,
        failed_to_find_when_resolving_indexing_expr,
        failed_to_find_when_resolving_identifier_expr,
        recursion_detected,
        instance_function_cannot_be_nullary,
        is_not_an_instance_function,
        argument_count_mismatch,
        failed_to_find
    };

    template <typename... Args>
    struct error_map
    {
        using func = std::function<std::shared_ptr<error>(const source_information& source_info, Args...)>;
        using map = std::map<error_message_code, func>;

        static inline map func_map;

        static void register_func(error_message_code code, func&& b)
        {
            auto [it, success] = func_map.insert({ code, std::move(b) });
            if(!success)
                throw;
        }

        static std::shared_ptr<error> build_error(error_message_code code, const source_information& source_info, Args... args)
        {
            auto it = func_map.find(code);
            if (it == func_map.end())
            {
                //todo: not valid so do something better
                throw;
            }

            return it->second(source_info, args...);
        }
    };

    //specialisation for zero parameters
    template <>
    struct error_map<> {
        using func = std::function<std::shared_ptr<error>(const source_information&)>;
        using map = std::map<error_message_code, func>;

        static inline map func_map;

        static void register_func(error_message_code code, func&& b)
        {
            auto [it, success] = func_map.insert({ code, std::move(b) });
            if (!success)
                throw;
        }

        static std::shared_ptr<error> build_error(error_message_code code, const source_information& source_info)
        {
            auto it = func_map.find(code);
            if (it == func_map.end())
            {
                //todo: not valid so do something better
                throw;
            }

            return it->second(source_info);
        }
    };

    //template helper methods
    template <typename... Args>
    void register_error(error_message_code code, std::string format, element_result error_result)
    {
        auto builder = [f = std::move(format), error_result](const source_information& source_info, Args... args)
        {
            //std::cout << fmt::format(format, args...);
            return std::make_shared<error>(fmt::format(f, args...), error_result, source_info);
        };

        error_map<Args...>::register_func(code, std::move(builder));
    }

    //build_error is intended to fall back on template argument deduction to avoid the need for silly template parameters everywhere

    template <typename... Args>
        std::shared_ptr<error> build_error(const source_information& source_info, error_message_code code, Args... args)
    {
        return error_map<Args...>::build_error(code, source_info, args...);
    }

    namespace detail
    {
        //used for initializing the errors in maps statically
        bool register_errors();
    }
}