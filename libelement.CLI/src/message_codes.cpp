#include <message_codes.hpp>

std::map<std::string, cli::message_level> cli::message_code::map_message_level = {
		{"Fatal", cli::message_level::FATAL},
		{"Warning", cli::message_level::WARNING},
		{"Error", cli::message_level::ERROR},
		{"Information", cli::message_level::INFORMATION},
		{"Verbose", cli::message_level::VERBOSE}
};