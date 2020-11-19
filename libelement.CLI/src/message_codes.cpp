#include <message_codes.hpp>

using namespace libelement::cli;

//it's possible that static initialisation going to sting me here due to method call in constructor accessing this vector...
//will worry about it if it becomes a concern
std::vector<std::pair<message_level, std::string>> message_code::message_levels = {
    { message_level::Warning, "Warning" },
    { message_level::Error, "Error" },
    { message_level::Information, "Information" },
    { message_level::Verbose, "Verbose" }
};