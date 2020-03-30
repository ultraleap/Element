#pragma once

#include <vector>
#include <string>
#include <optional>

#include "message_codes.hpp"

namespace cli {

	struct trace_site {

		std::string what;
		std::string source;
		int line;
		int column;

		std::string message() const {

			return "";
		}
	};

	class compiler_message
	{
	private:
		const char* key_message_code = "MessageCode";
		const char* key_message_level = "MessageLevel";
		const char* key_context = "Context";
		const char* key_trace_stack = "TraceStack";

	public:
		compiler_message(std::string message, message_level message_level)
			: compiler_message(-1, message_level, message, std::vector<trace_site> {})
		{
		}

		compiler_message(int message_code, message_level message_level, const std::string& context, const std::vector<trace_site>& trace_stack) 
			: message_code{ message_code }, message_level{ message_level }, context{ context }, trace_stack{ trace_stack }
		{
		}

	private:
		int message_code;
		message_level message_level;
		std::string context;
		std::vector<trace_site> trace_stack;

	public:
		const std::string serialize();
	};
}