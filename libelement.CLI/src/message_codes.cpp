#include <message_codes.hpp>

using namespace libelement::cli;

std::map<std::string, message_level> message_code::map_message_level = {
		{"Fatal", message_level::FATAL},
		{"Warning", message_level::WARNING},
		{"Error", message_level::ERROR},
		{"Information", message_level::INFORMATION},
		{"Verbose", message_level::VERBOSE}
};