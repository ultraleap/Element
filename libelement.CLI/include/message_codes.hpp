#pragma once

#include <map>
#include <string>

#include <element/common.h>
#include <toml.hpp>

namespace libelement::cli
{
enum class message_level : unsigned int {
    Verbose = 0,
    Information,
    Warning,
    Error
};

struct message_code
{
private:
    typedef const std::vector<std::pair<message_level, std::string>>::const_iterator
        message_level_const_iterator;

public:
    element_result type;
    std::string name;
    message_level level;

    message_code(element_result type, std::string name, const std::string& level)
        : type{ type }
        , name{ std::move(name) }
        , level{ get_message_level(level) }
    {}

private:
    static std::vector<std::pair<message_level, std::string>> message_levels;

    static message_level get_message_level(const std::string& level)
    {
        auto predicate = [level](const std::pair<message_level, std::string>& pair) -> bool {
            return level == pair.second;
        };

        message_level_const_iterator it = std::find_if(std::begin(message_levels), std::end(message_levels), predicate);
        if (it != message_levels.end())
            return it->first;

        return message_level::Error;
    }

    static std::string get_message_level(const message_level level)
    {
        auto predicate = [level](const std::pair<message_level, std::string>& pair) -> bool {
            return level == pair.first;
        };

        message_level_const_iterator it = std::find_if(std::begin(message_levels), std::end(message_levels), predicate);
        if (it != message_levels.end())
            return it->second;

        return "Error";
    }
};

class message_codes
{
    typedef const std::map<element_result, message_code>::const_iterator message_code_const_iterator;

public:
    explicit message_codes(const std::string& path)
    {
        auto data = toml::parse(path);
        auto table = toml::get<toml::table>(data);

        message_code_const_iterator it = code_map.begin();
        for (const auto& item : table) {
            auto name = toml::find<std::string>(item.second, "name");
            auto level = toml::find<std::string>(item.second, "level");
            auto index = static_cast<element_result>(std::stoi(item.first.substr(3)));
            code_map.insert(it, std::pair<element_result, message_code>(
                                    index, message_code(index, name, level)));
        }
    }

    [[nodiscard]] const message_code* get_code(element_result type) const
    {
        const auto it = code_map.find(type);
        if (it != code_map.end())
            return &it->second;

        return nullptr;
    }

    [[nodiscard]] message_level get_level(element_result type) const
    {
        const auto* const message_code = get_code(type);
        if (message_code == nullptr)
            return message_level::Error;

        return message_code->level;
    }

private:
    std::map<element_result, message_code> code_map;
};

inline element_result error_conversion(element_result result)
{
    if (result > 0)
        return result;

    if (result == ELEMENT_ERROR_RESERVED_IDENTIFIER)
        return ELEMENT_ERROR_INVALID_IDENTIFIER;

    return ELEMENT_ERROR_UNKNOWN;
};
} // namespace libelement::cli