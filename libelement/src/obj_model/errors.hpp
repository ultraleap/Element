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
        oh_no
    };

    template <typename... Args>
    struct error_map
    {
        using func = std::function<std::shared_ptr<error>(Args...)>;
        using map = std::map<error_message_code, func>;

        static inline map func_map;

        static void register_func(error_message_code code, func&& b)
        {
            //func_map.insert(code, std::move(b));

            auto& [it, success] = func_map.insert(std::pair<error_message_code, func>(code, std::move(b)));
            if(!success)
            {
                //todo: already inserted, lol
            }
        }

        static std::shared_ptr<error> build_error(error_message_code code, Args... args)
        {
            auto it = func_map.find(code);
            if (it == func_map.end())
            {
                //todo: not valid so do something better
                throw;
            }

            return it->second(args...);
        }
    };

    //template methods to register and log errors, build_error is intended to fall back on template argument deduction to avoid the need for silly template parameters everywhere
    template <typename... Args>
    void register_error(error_message_code code, std::string format)
    {
        error_map<Args...>::register_func(code, [format = std::move(format)](Args... args)
        {
            //std::cout << fmt::format(format, args...);
            return std::make_shared<error>(fmt::format(format, args...), 0, nullptr); //todo
        });
    }

    template <typename... Args>
    std::shared_ptr<error> build_error(error_message_code code, Args... args)
    {
        return error_map<Args...>::build_error(code, args...);
    }
}