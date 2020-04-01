#pragma once

#include <vector>
#include <string>

#include "message_codes.hpp"

namespace libelement::cli 
{
	struct trace_site 
	{
	private:
		std::string message;

	//public:
	//	std::string what;
	//	std::string source;
	//	int line = 0;
	//	int column = 0;

	public:
		trace_site(const std::string& what, const std::string& source, const int line, const int column)
			//: what{ std::move(what) }, source{ std::move(source) }, line{ line }, column{ column }
		{
			std::stringstream ss;
			ss << what << " in " << source << ":" << line << "," << column << std::endl;
			message = ss.str();
		}

		const std::string& get_message() const 
		{
			return message; //$"{What} in {Source}:{Line},{Column}"
		}
	};

	class compiler_message
	{
		 static constexpr const char* const key_message_code { "MessageCode" };
		 static constexpr const char* const key_message_level { "MessageLevel" };
		 static constexpr const char* const key_context { "Context" };
		 static constexpr const char* const key_trace_stack { "TraceStack" };

		int message_code;
		message_level level;
		std::string context;
		std::vector<trace_site> trace_stack;

		//this is nasty, static initialisation that performs file reading, reconsider if it proves problematic
		//inline static message_codes codes { message_codes("config/Messages.toml") };
		
	public:
		compiler_message(std::string message, message_level level)
			: message_code{ -1 }, level{ level }, context{ std::move(message) }
		{
		}

		compiler_message(const int message_code, message_level level, std::string context, std::vector<trace_site> trace_stack)
			: message_code{ message_code }, level{ level }, context{ std::move(context) }, trace_stack{ std::move(trace_stack) }
		{
		}
		
		std::string serialize() const;
	};
}