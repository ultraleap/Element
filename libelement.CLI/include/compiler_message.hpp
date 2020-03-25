#pragma once

#include <vector>
#include <string>
#include <optional>

enum class message_level {
	FATAL = 0,
	WARNING,
	ERROR,
	INFORMATION,
	VERBOSE,
};

struct trace_site {

	std::string what;
	std::string source;
	int line;
	int column;
};

class compiler_message 
{
private:
	std::optional<int> message_code;
	std::optional<message_level> message_level;
	std::optional<std::string> context;
	std::vector<trace_site> trace_stack;

public:
};