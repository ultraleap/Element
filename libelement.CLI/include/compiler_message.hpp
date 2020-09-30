#pragma once

#include <vector>
#include <string>
#include <optional>

#include "element/interpreter.h"

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
	//	int character = 0;

	public:
		trace_site(const std::string& what, const std::string& source, const int line, const int character)
			//: what{ std::move(what) }, source{ std::move(source) }, line{ line }, character{ character }
		{
			std::stringstream ss;
			ss << what << " in " << source << ":" << line << "," << character << std::endl;
			message = ss.str();
		}

		[[nodiscard]] const std::string& get_message() const
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

		std::optional<message_type> type;
		std::optional<message_level> level;
		std::string context;
		std::vector<trace_site> trace_stack;
		bool serialize_to_json = false;

		//this is nasty, static initialisation that performs file reading, reconsider if it proves problematic
		inline static message_codes codes { message_codes("Messages.toml") };
		
	public:
		compiler_message(std::string message, bool log_json, std::vector<trace_site> trace_stack = std::vector<libelement::cli::trace_site>())
			: type{ std::nullopt }
	        , level{ std::nullopt }
	        , context{ std::move(message) }
			, trace_stack{ std::move(trace_stack) }
	        , serialize_to_json(log_json)
		{
		}

		compiler_message(message_type type, std::string message, bool log_json, std::vector<trace_site> trace_stack = std::vector<libelement::cli::trace_site>())
			: type{ type }, level{ codes.get_level(type) }, context{ std::move(message) }, trace_stack{ std::move(trace_stack) }, serialize_to_json(log_json)
		{
		}

		compiler_message(message_type type, const element_log_message* const error, bool log_json)
			: type{ type }, level{ codes.get_level(type) }, context { error->message }, serialize_to_json(log_json)
		{
			//TODO: set trace_stack based on expression_cache frame list?
		}

        [[nodiscard]] message_level get_level() const;
		[[nodiscard]] std::string serialize() const;
	};
}