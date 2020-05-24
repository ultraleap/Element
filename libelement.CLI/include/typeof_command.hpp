#pragma once

#include <string>

#include "command.hpp"

namespace libelement::cli
{
	struct typeof_command_arguments 
	{
		std::string expression;

		std::string as_string() const
		{
			std::stringstream ss;
			ss << "typeof --expression " << expression << " ";
			return ss.str();
		}
	};

	class typeof_command final : public command
	{
		typeof_command_arguments custom_arguments;

	public:
		typeof_command(common_command_arguments common_arguments, typeof_command_arguments custom_arguments)
			: command(std::move(common_arguments)), custom_arguments{ std::move(custom_arguments) }
		{
		}

		compiler_message execute(const compilation_input& input) const override
		{
			element_result result = ELEMENT_OK;
			result = setup(input);
			if (result != ELEMENT_OK)
				return compiler_message(ELEMENT_ERROR_PARSE,"Failed to setup context");

			//call into libelement
			const element_function* fn;
			element_compiled_function* cfn;
			const std::vector<trace_site> trace_site{};

			//Not handling error responses properly yet
			const auto typeof = "typeof = " + custom_arguments.expression + ";";
			result = element_interpreter_load_string(ictx, typeof.c_str(), "<input>");
			if (result != ELEMENT_OK) {
				return compiler_message(ELEMENT_ERROR_PARSE, "Failed to parse: " + typeof);
			}

			result = element_interpreter_get_function(ictx, "typeof", &fn);
			if (result != ELEMENT_OK)
				return compiler_message(ELEMENT_ERROR_UNKNOWN, "Failed to find: " + typeof);

			result = element_interpreter_compile_function(ictx, fn, &cfn, nullptr);
			if (result != ELEMENT_OK)
				return compiler_message(ELEMENT_ERROR_UNKNOWN, "Failed to compile: " + typeof);

			//todo: typeof isn't really typeof, it's more like "get conceptual name for this thing". e.g. A namespace doesn't have a type, but if we ask for the typeof a namespace the string "Namespace" is expected.
			//this only handles typeof() for expressions that result in a valid compilation, getting the type of the thing they compiled to. Basically Num.
			//the way to solve it would be to do a scope lookup for a name, and then based on the scope/ast node, determine what "typeof" should be?
			//while this could be an interesting command to keep, top-level functions in element (i.e. user-compiled) must have an interface (parameters and returns) that are serializable, so their type should already be known.
			//We'd have to bypass some checks to do things this way
			std::string typeof_string(256, '\0');
			element_compiled_function_get_typeof_compilation(cfn, typeof_string.data(), 256);

			return generate_response(result, typeof_string, trace_site);
		}

		std::string as_string() const override
		{
			std::stringstream ss;
			ss << custom_arguments.as_string() << " " << common_arguments.as_string();
			return ss.str();
		}

		static void configure(CLI::App& app, const std::shared_ptr<common_command_arguments>& common_arguments, command::callback callback)
		{
			const auto arguments = std::make_shared<typeof_command_arguments>();

			const auto command = app.add_subcommand("typeof")->fallthrough();
			command->add_option("-e,--expression", arguments->expression, "Expression to get the type of.")->required();

			command->callback([callback, common_arguments, arguments]() { callback(typeof_command(*common_arguments, *arguments)); });
		}
	};
}